// Низкоуровневый вывод в консоль: размеры, обрезка текста, шапка, рамки, футер, клавиши.
#pragma once
#include <windows.h>

#include <string>
#include <utility>
#include <vector>

size_t vlen(const std::string& s);                  // видимая длина без кодов ANSI
std::string cut(const std::string& s, size_t w);    // обрезка простого текста с многоточием
std::string fit(const std::string& s, size_t w);    // обрезка + добивка пробелами до w
std::string rep(const std::string& s, size_t n);
std::string gradient(const std::string& s);         // градиент текущей темы по символам

void term(int& w, int& h);
size_t width();  // ширина интерфейса: от 40 до 110

void ln(std::string& o, const std::string& s = "");
void header(std::string& o, const std::string& crumb, size_t W);
void box(std::string& o, const std::string& title, const std::vector<std::string>& lines, size_t W, const std::string& c);
std::string kv(const std::string& k, const std::string& v);  // строка "ключ  значение" для карточек
std::string footer(const std::vector<std::pair<std::string, std::string>>& keys, const std::string& right = "");

void leave();  // вернуть обычный экран и курсор
BOOL WINAPI onCtrlC(DWORD);
int key();  // читает клавишу; Ctrl-C завершает программу
