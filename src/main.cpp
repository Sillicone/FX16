// FX16: консольный менеджер портативных программ (zip) в духе Chocolatey / console portable-app manager.
// Каталог: config\config.toml рядом с FX16.exe. Данные: %FX16_HOME% или %USERPROFILE%\FX16
// (installed.txt, settings.txt, apps\).
#include <windows.h>

#include <cstdlib>
#include <iostream>

#include "core/config.hpp"
#include "core/db.hpp"
#include "core/settings.hpp"
#include "net/net.hpp"
#include "pkg/installer.hpp"
#include "screens/screens.hpp"
#include "ui/menu.hpp"
#include "ui/term.hpp"

int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD m = 0;
    GetConsoleMode(h, &m);
    SetConsoleMode(h, m | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    bool checkMode = argc > 1 && std::string(argv[1]) == "--check";
    if (!checkMode) {
        std::cout << "\x1b[?1049h\x1b[?25l" << std::flush;
        std::atexit(leave);
        SetConsoleCtrlHandler(onCtrlC, TRUE);
    }

    const char* e = std::getenv("FX16_HOME");
    if (!e) e = std::getenv("CPKG_HOME");  // старое имя переменной
    const char* u = std::getenv("USERPROFILE");
    fs::path base = u ? u : ".";
    std::error_code ec;
    if (!e && fs::exists(base / "cpkg") && !fs::exists(base / "FX16")) fs::rename(base / "cpkg", base / "FX16", ec);  // перенос данных после переименования
    home = e ? fs::path(e) : base / (fs::exists(base / "cpkg") && !fs::exists(base / "FX16") ? "cpkg" : "FX16");
    installedFile = home / "installed.txt";
    fs::create_directories(home / "apps");
    configFile = findConfig();
    loadSettings();

    if (checkMode) {
        quiet = true;
        if (argc < 3) return check(loadPackages());
        std::string a = argv[2];
        std::string src = a.rfind("src:", 0) == 0 || a.rfind("web:", 0) == 0 ? a : repoOf(a);
        if (src.empty()) { std::cout << tr("Не похоже на ссылку GitHub: ", "Does not look like a GitHub link: ") << a << "\n"; return 2; }
        return check({{src.substr(src.find_last_of("/:") + 1), src, ""}});
    }

    for (;;) {
        size_t nCat = loadPackages().size(), nInst = readDb(installedFile).size();
        int c = pick(tr("Главная", "Home"),
                     {{tr("Каталог", "Catalog"), tr("найти и установить программы", "find and install programs"), std::to_string(nCat)},
                      {tr("Установленные", "Installed"), tr("подробности, перемещение, удаление", "details, move, delete"), std::to_string(nInst)},
                      {tr("Настройки", "Settings"), tr("язык и цветовая тема", "language and color theme")},
                      {tr("Выход", "Quit"), ""}},
                     false,
                     {kv(tr("Папка данных", "Data folder"), home.string()),
                      kv(tr("Каталог", "Catalog"), configFile.empty() ? tr("config\\config.toml НЕ НАЙДЕН", "config\\config.toml NOT FOUND") : configFile.string()),
                      kv(tr("Источник", "Source"), tr("GitHub Releases (последние версии)", "GitHub Releases (latest versions)"))},
                     tr("Состояние", "Status"));
        if (c == 0) catalog();
        else if (c == 1) installed();
        else if (c == 2) settings();
        else return 0;
    }
}
