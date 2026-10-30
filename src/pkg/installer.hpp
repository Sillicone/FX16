// Установка, обновление и проверка программ.
#pragma once
#include <string>
#include <utility>
#include <vector>

#include "core/globals.hpp"

// Скачивает архив, заменяет папку программы и записывает её в installed.txt.
// Старая версия удаляется только после успешной загрузки.
bool fetch(const std::string& name, const std::string& tag, const std::string& url, const fs::path& dir, const std::string& source);

void install(const Row& p);  // p = {name, source, description}; спрашивает папку

// Проверяет и ставит новую версию в ту же папку. kind: 0 - актуальна, 1 - обновлена, 2 - ошибка.
std::pair<int, std::string> updateRow(const Row& r);
void updateAll();
void addByLink();

// FX16 --check: пробная установка записей каталога с проверкой содержимого и немедленным удалением.
int check(const std::vector<Row>& targets);
