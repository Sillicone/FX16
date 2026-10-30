// Цветовые темы и палитра ANSI.
#pragma once
#include <string>

// Темы: два цвета градиента (a -> b) и фон выбранной строки.
struct Theme {
    const char *ru, *en, *descRu, *descEn;
    int a[3], b[3], sel[3];
};
extern const Theme themes[];
extern const int nThemes;

std::string fg(int r, int g, int b);

// Постоянные цвета.
extern const std::string RST, BOLD, NOBOLD, KEYBG, GRN, YEL, RED, DIM, TXT, SUB;
// Зависят от темы, задаются в applyTheme.
extern std::string ACC, VIO, SELBG;

void applyTheme(int i);
