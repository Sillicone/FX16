// Каталог программ из config\config.toml.
#pragma once
#include "core/globals.hpp"

fs::path appDir();  // папка с FX16.exe

// Ищет config\config.toml рядом с FX16.exe (или на уровень выше, если запущен build\FX16.exe).
// Если файла нет, создаёт папку config и файл по умолчанию рядом с FX16.exe.
fs::path findConfig();

// Блоки [[package]] -> строки {name, source, description, usage}. Порядок в файле любой.
std::vector<Row> loadPackages();
