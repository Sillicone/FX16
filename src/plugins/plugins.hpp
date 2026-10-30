// Система плагинов на JavaScript (QuickJS). Плагины - файлы *.js в папке plugins данных FX16.
#pragma once
#include <string>
#include <vector>

// Создаёт папку plugins (с примером при первом запуске), поднимает движок и загружает все *.js.
void initPlugins();

// Пункты главного меню, добавленные плагинами через fx16.menu.add().
struct PluginEntry {
    std::string title, description, tag;
};
const std::vector<PluginEntry>& pluginEntries();

// Выполняет onSelect пункта i; ошибки JavaScript показывает в окне и пишет в plugins.log.
void runPluginEntry(size_t i);

// Ошибки загрузки плагинов (для отладки авторами).
const std::vector<std::string>& pluginErrors();

std::string pluginsDir();
std::string pluginsSummary();  // "2 пункта меню · 1 ошибка"
