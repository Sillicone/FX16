#include "core/config.hpp"

#include <windows.h>

#include <algorithm>
#include <fstream>

#include "core/embedded.hpp"

fs::path appDir() {
    char buf[MAX_PATH];
    GetModuleFileNameA(nullptr, buf, MAX_PATH);
    return fs::path(buf).parent_path();
}

// Куда класть создаваемые папки: рядом с FX16.exe, а для build\FX16.exe внутри проекта - в корень проекта.
static fs::path baseDir() {
    fs::path d = appDir();
    return d.filename() == "build" && fs::exists(d.parent_path() / "CMakeLists.txt") ? d.parent_path() : d;
}

fs::path findConfig() {
    fs::path dir = appDir();
    for (fs::path d : {dir, dir.parent_path()})
        if (fs::exists(d / "config" / "config.toml")) return d / "config" / "config.toml";
    fs::path base = baseDir();
    std::error_code ec;
    fs::create_directories(base / "config", ec);
    std::ofstream(base / "config" / "config.toml", std::ios::binary) << kDefaultConfig;
    return base / "config" / "config.toml";
}

// shortcut: разбирается только это подмножество TOML (строки в двойных кавычках, экранирование только \\ и \").
std::vector<Row> loadPackages() {
    std::vector<Row> v;
    std::ifstream in(configFile);
    std::string l;
    while (std::getline(in, l)) {
        l.erase(0, l.find_first_not_of(" \t\xEF\xBB\xBF"));  // пробелы и BOM
        while (!l.empty() && (l.back() == '\r' || l.back() == ' ')) l.pop_back();
        if (l == "[[package]]") { v.push_back({"", "", "", ""}); continue; }
        size_t eq = l.find('='), q1 = l.find('"'), q2 = l.rfind('"');
        if (v.empty() || l.empty() || l[0] == '#' || eq == std::string::npos || q1 == std::string::npos || q2 <= q1) continue;
        std::string k = l.substr(0, eq), val = l.substr(q1 + 1, q2 - q1 - 1);
        k.erase(k.find_last_not_of(" \t") + 1);
        std::string un;
        for (size_t i = 0; i < val.size(); ++i)
            un += (val[i] == '\\' && i + 1 < val.size() && (val[i + 1] == '\\' || val[i + 1] == '"')) ? val[++i] : val[i];
        if (k == "name") v.back()[0] = un;
        else if (k == "source") v.back()[1] = un;
        else if (k == "description") v.back()[2] = un;
        else if (k == "usage") v.back()[3] = un;
    }
    v.erase(std::remove_if(v.begin(), v.end(), [](const Row& r) { return r[0].empty() || r[1].empty(); }), v.end());
    return v;
}
