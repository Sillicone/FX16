// Каталог программ из config\config.toml.
#pragma once
#include "core/globals.hpp"

fs::path findConfig();

// Блоки [[package]] -> строки {name, source, description, usage}. Порядок в файле любой.
std::vector<Row> loadPackages();
