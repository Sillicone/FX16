// Файлы по умолчанию, вшитые в программу при сборке: на их основе FX16 восстанавливает
// config\config.toml и plugins\example.js, если их нет рядом с FX16.exe.
// Источник - обычные файлы resources/default-config.toml и resources/example-plugin.js (см. CMakeLists.txt).
#pragma once

extern const char* const kDefaultConfig;
extern const char* const kExamplePlugin;
