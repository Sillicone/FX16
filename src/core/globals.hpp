// Общие типы и глобальное состояние программы.
#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using Row = std::vector<std::string>;

extern fs::path home, configFile, installedFile;
extern bool quiet;  // режим --check: без полос прогресса curl
extern int lang;    // 0 - русский, 1 - English
extern int theme;   // индекс в themes[]

// Выбор строки по языку интерфейса.
std::string tr(const char* ru, const char* en);
