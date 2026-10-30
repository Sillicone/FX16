#include "core/settings.hpp"

#include <cstdlib>
#include <fstream>

#include "core/globals.hpp"
#include "ui/theme.hpp"

void loadSettings() {
    std::ifstream in(home / "settings.txt");
    int th = 0;
    for (std::string l; std::getline(in, l);) {
        if (l == "lang=en") lang = 1;
        else if (l == "lang=ru") lang = 0;
        else if (l.rfind("theme=", 0) == 0) th = std::atoi(l.c_str() + 6);
    }
    applyTheme(th);
}

void saveSettings() { std::ofstream(home / "settings.txt") << "lang=" << (lang ? "en" : "ru") << "\ntheme=" << theme << "\n"; }
