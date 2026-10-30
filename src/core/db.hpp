// Простая база installed.txt (строки с полями через '|') и мелкие помощники для строк/строк БД.
#pragma once
#include "core/globals.hpp"

std::vector<Row> readDb(const fs::path& p);
void writeDb(const fs::path& p, const std::vector<Row>& v);
std::string lower(std::string s);
int findRow(const std::vector<Row>& v, const std::string& name);
std::string col(const Row& r, size_t i);
