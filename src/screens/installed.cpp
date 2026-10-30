#include <sstream>

#include "core/config.hpp"
#include "core/db.hpp"
#include "pkg/installer.hpp"
#include "screens/screens.hpp"
#include "ui/menu.hpp"
#include "ui/term.hpp"

// Карточка программы: версия, папка, размер, источник и примеры запуска из config.toml.
static std::vector<std::string> details(const Row& r) {
    std::error_code ec;
    std::string usage;  // примеры команд берём из config.toml по имени программы
    for (auto& p : loadPackages())
        if (p[0] == r[0]) usage = p[3];
    std::vector<std::string> ex;
    std::stringstream us(usage);
    for (std::string e; std::getline(us, e, ';') && ex.size() < 3;) {
        e.erase(0, e.find_first_not_of(' '));
        while (!e.empty() && e.back() == ' ') e.pop_back();
        if (!e.empty()) ex.push_back(e);
    }
    // Ищем .exe по первому слову первой команды: точное имя лучше, чем имя, начинающееся с него.
    std::string word = ex.empty() ? "" : lower(ex[0].substr(0, ex[0].find(' ')));
    fs::path exe;
    bool exact = false;
    uintmax_t bytes = 0, files = 0;
    for (fs::recursive_directory_iterator it(r[2], ec), end; !ec && it != end; it.increment(ec))
        if (it->is_regular_file(ec)) {
            bytes += it->file_size(ec);
            ++files;
            std::string stem = lower(it->path().stem().string());
            if (!word.empty() && lower(it->path().extension().string()) == ".exe") {
                if (stem == word) { exe = it->path(); exact = true; }
                else if (!exact && exe.empty() && stem.rfind(word, 0) == 0) exe = it->path();
            }
        }
    std::string size = bytes >= 1048576 ? std::to_string(bytes / 1048576) + tr(" МБ", " MB") : std::to_string(bytes / 1024) + tr(" КБ", " KB");
    std::vector<std::string> lines = {kv(tr("Версия", "Version"), r[1]), kv(tr("Установлена", "Installed"), col(r, 3)),
                                      kv(tr("Папка", "Folder"), r[2] + (fs::exists(r[2]) ? "" : tr("  (не найдена!)", "  (not found!)"))),
                                      kv(tr("Размер", "Size"), size + tr(" · файлов: ", " · files: ") + std::to_string(files)),
                                      kv(tr("Источник", "Source"), col(r, 4))};
    if (ex.empty()) {
        lines.push_back(kv(tr("Использование", "Usage"), tr("не задано (поле usage в config.toml)", "not set (usage field in config.toml)")));
    } else {
        if (!exe.empty()) {
            lines.push_back(kv(tr("Перейти", "Go to"), "cd /d " + exe.parent_path().string()));
            if (lower(exe.stem().string()) != word) lines.push_back(kv(tr("Файл", "File"), exe.filename().string()));
        }
        lines.push_back(kv(tr("Команды", "Commands"), ex[0]));
        for (size_t i = 1; i < ex.size(); ++i) lines.push_back(kv("", ex[i]));
    }
    return lines;
}

// Действия над выбранной программой: обновить, переместить, удалить.
static void actions(size_t idx) {
    auto inst = readDb(installedFile);
    Row r = inst[idx];
    std::string crumb = tr("Установленные", "Installed") + " › " + r[0];
    int a = pick(crumb,
                 {{tr("Обновить", "Update"), tr("проверить и скачать новую версию с GitHub", "check and download a newer version")},
                  {tr("Переместить", "Move"), tr("в другую папку", "to another folder")},
                  {tr("Удалить", "Delete"), tr("программу вместе с папкой", "the program with its folder"), "", true},
                  {tr("Назад", "Back"), ""}},
                 false, details(r), r[0]);
    std::error_code ec;
    if (a == 0) {
        status(tr("Обновление", "Update") + " › " + r[0], tr("◆ Проверка", "◆ Checking"),
               {TXT + r[0] + " " + r[1], DIM + tr("Спрашиваю GitHub о новой версии…", "Asking GitHub for a newer version…")});
        auto [kind, text] = updateRow(r);
        msg(text, kind);
    } else if (a == 1) {
        fs::path dst = ask(crumb, tr("Новая папка для ", "New folder for ") + r[0], r[2]);
        if (dst == fs::path(r[2])) return;
        if (fs::exists(dst)) return msg(tr("Папка уже существует:\n", "Folder already exists:\n") + dst.string(), 2);
        fs::create_directories(dst.parent_path(), ec);
        fs::rename(r[2], dst, ec);
        if (ec) {  // другой диск: копируем и удаляем
            fs::copy(r[2], dst, fs::copy_options::recursive, ec);
            if (ec) return msg(tr("Не удалось переместить:\n", "Could not move:\n") + ec.message(), 2);
            fs::remove_all(r[2], ec);
        }
        inst[idx][2] = dst.string();
        writeDb(installedFile, inst);
        msg(tr("Перемещено в\n", "Moved to\n") + dst.string(), 1);
    } else if (a == 2) {
        if (pick(crumb + " › " + tr("Удаление", "Delete"),
                 {{tr("Нет, оставить", "No, keep it"), ""}, {tr("Да, удалить", "Yes, delete"), tr("это действие необратимо", "this cannot be undone"), "", true}},
                 false, {kv(tr("Программа", "Program"), r[0] + " " + r[1]), kv(tr("Папка", "Folder"), r[2])}, tr("Удалить?", "Delete?"), RED) != 1)
            return;
        fs::remove_all(r[2], ec);
        inst.erase(inst.begin() + idx);
        writeDb(installedFile, inst);
        msg(r[0] + tr(" удалена.", " deleted."), 1);
    }
}

void installed() {
    for (;;) {
        auto inst = readDb(installedFile);
        std::vector<Item> items = {{tr("+ Добавить по ссылке", "+ Add by link"), tr("установить репозиторий с GitHub", "install a GitHub repository")},
                                   {tr("~ Обновить все", "~ Update all"), tr("проверить новые версии и обновить", "check for new versions and update")}};
        for (auto& r : inst) items.push_back({r[0], col(r, 3) + "  ·  " + r[2], r[1]});
        int i = pick(tr("Установленные · ", "Installed · ") + std::to_string(inst.size()), items, true);
        if (i < 0) return;
        if (i == 0) addByLink();
        else if (i == 1) updateAll();
        else actions(i - 2);
    }
}
