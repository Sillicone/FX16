#include "pkg/installer.hpp"

#include <ctime>
#include <iomanip>
#include <iostream>

#include "core/db.hpp"
#include "net/net.hpp"
#include "ui/menu.hpp"
#include "ui/term.hpp"
#include "ui/theme.hpp"

bool fetch(const std::string& name, const std::string& tag, const std::string& url, const fs::path& dir, const std::string& source) {
    fs::path tmp = home / "download.zip";
    std::error_code ec;
    if (!run(std::string("curl -fL ") + (quiet ? "-sS" : "-#") + " -o \"" + tmp.string() + "\" \"" + url + "\"")) { fs::remove(tmp, ec); return false; }
    auto inst = readDb(installedFile);
    int old = findRow(inst, name);
    if (old >= 0) fs::remove_all(inst[old][2], ec);
    fs::create_directories(dir);
    bool ok = run("tar -xf \"" + tmp.string() + "\" -C \"" + dir.string() + "\"");
    fs::remove(tmp, ec);
    if (!ok) {
        fs::remove_all(dir, ec);
        if (old >= 0) inst.erase(inst.begin() + old);
        writeDb(installedFile, inst);
        return false;
    }
    char date[32];
    std::time_t now = std::time(nullptr);
    std::strftime(date, sizeof date, "%Y-%m-%d %H:%M", std::localtime(&now));
    Row r = {name, tag, dir.string(), date, url, source};
    if (old >= 0) inst[old] = r; else inst.push_back(r);
    writeDb(installedFile, inst);
    return true;
}

void install(const Row& p) {
    std::string crumb = tr("Установка", "Install") + " › " + p[0];
    status(crumb, tr("◆ Поиск релиза", "◆ Looking for release"),
           {TXT + tr("Репозиторий: ", "Repository: ") + p[1], DIM + tr("Запрашиваю последний релиз на GitHub…", "Asking GitHub for the latest release…")});
    std::string tag, url;
    if (!resolve(p[1], tag, url))
        return msg(tr("Не удалось найти Windows .zip в последнем релизе\n", "No Windows .zip found in the latest release of\n") + p[1] +
                       tr("\n(или нет сети / лимит GitHub API).", "\n(or no network / GitHub API rate limit)."),
                   2);

    auto inst = readDb(installedFile);
    int old = findRow(inst, p[0]);
    fs::path def = old >= 0 ? fs::path(inst[old][2]) : home / "apps" / p[0];
    fs::path dir = ask(crumb, p[0] + " " + tag + tr(" - папка установки", " - install folder"), def.string());
    status(crumb, tr("◆ Загрузка ", "◆ Downloading ") + tag, {TXT + cut(url, width() - 4), DIM + "→ " + cut(dir.string(), width() - 8)});
    if (!fetch(p[0], tag, url, dir, p[1])) return msg(tr("Не удалось скачать или распаковать архив.", "Could not download or unpack the archive."), 2);
    msg(p[0] + " " + tag + tr(" установлена\n", " installed\n") + dir.string(), 1);
}

std::pair<int, std::string> updateRow(const Row& r) {
    std::string repo = r.size() > 5 ? r[5] : repoOf(col(r, 4));  // старые записи без источника: берём из ссылки GitHub
    if (repo.empty()) return {2, r[0] + tr(": неизвестный источник, установите заново", ": unknown source, reinstall it")};
    std::string tag, url;
    if (!resolve(repo, tag, url)) return {2, r[0] + tr(": не удалось получить релиз ", ": could not get the release of ") + repo};
    if (tag == r[1]) return {0, r[0] + " " + r[1] + tr(" - актуальная", " - up to date")};
    if (!fetch(r[0], tag, url, r[2], repo)) return {2, r[0] + tr(": ошибка загрузки ", ": download failed ") + tag};
    return {1, r[0] + ": " + r[1] + " → " + tag};
}

void updateAll() {
    auto inst = readDb(installedFile);
    std::vector<std::string> lines;
    int fresh = 0, updated = 0, failed = 0;
    for (size_t i = 0; i < inst.size(); ++i) {
        status(tr("Обновление", "Update"), tr("◆ Проверка ", "◆ Checking ") + std::to_string(i + 1) + "/" + std::to_string(inst.size()),
               {TXT + inst[i][0] + " " + inst[i][1], DIM + tr("Спрашиваю GitHub о новой версии…", "Asking GitHub for a newer version…")});
        auto [kind, text] = updateRow(inst[i]);
        if (kind == 0) ++fresh;
        else if (kind == 1) { ++updated; lines.push_back(GRN + "✓ " + text); }
        else { ++failed; lines.push_back(RED + "✗ " + text); }
    }
    lines.push_back(DIM + std::to_string(updated) + tr(" обновлено · ", " updated · ") + std::to_string(fresh) + tr(" актуальны · ", " up to date · ") +
                    std::to_string(failed) + tr(" ошибок", " errors"));
    std::string all;
    for (auto& l : lines) all += l + "\n";
    msg(all, failed ? 2 : 1);
}

void addByLink() {
    const std::string example = "https://github.com/owner/repo";
    std::string in = ask(tr("Установленные › Добавить", "Installed › Add"),
                         tr("Ссылка на GitHub-репозиторий (или owner/repo)", "GitHub repository link (or owner/repo)"), example);
    if (in == example) return;
    std::string repo = repoOf(in);
    if (repo.empty()) return msg(tr("Не похоже на ссылку GitHub:\n", "Does not look like a GitHub link:\n") + in, 2);
    install({repo.substr(repo.find('/') + 1), repo, tr("добавлено по ссылке", "added by link")});
}

int check(const std::vector<Row>& targets) {
    int ok = 0, bad = 0;
    for (auto& t : targets) {
        std::cout << std::left << std::setw(12) << t[0] << " " << std::flush;
        std::string tag, url;
        if (!resolve(t[1], tag, url)) {
            std::cout << "FAIL  " << tr("нет подходящего релиза/архива в ", "no suitable release/archive in ") << t[1] << "\n";
            ++bad;
            continue;
        }
        fs::path dir = home / "apps" / t[0];
        if (!fetch(t[0], tag, url, dir, t[1])) {
            std::cout << "FAIL  " << tr("не скачался/не распаковался ", "download/unpack failed ") << url << "\n";
            ++bad;
            continue;
        }
        std::error_code ec;
        uintmax_t bytes = 0, files = 0, exes = 0;
        std::string firstExe;
        for (fs::recursive_directory_iterator it(dir, ec), end; !ec && it != end; it.increment(ec))
            if (it->is_regular_file(ec)) {
                bytes += it->file_size(ec);
                ++files;
                if (lower(it->path().extension().string()) == ".exe" && ++exes == 1) firstExe = it->path().filename().string();
            }
        bool src = t[1].rfind("src:", 0) == 0;
        bool good = files > 0 && (src || exes > 0);
        std::cout << (good ? "OK    " : "WARN  ") << tag << "  " << bytes / 1048576 << tr(" МБ, файлов ", " MB, files ") << files << ", exe " << exes
                  << (firstExe.empty() ? "" : " (" + firstExe + ")") << (good ? "" : tr("  нет .exe внутри архива", "  no .exe inside the archive")) << "\n";
        good ? ++ok : ++bad;
        auto inst = readDb(installedFile);  // пробную установку сразу убираем
        int i = findRow(inst, t[0]);
        if (i >= 0) inst.erase(inst.begin() + i);
        writeDb(installedFile, inst);
        fs::remove_all(dir, ec);
    }
    std::cout << tr("\nИтого: ", "\nTotal: ") << ok << " OK, " << bad << tr(" проблем\n", " problems\n");
    return bad ? 1 : 0;
}
