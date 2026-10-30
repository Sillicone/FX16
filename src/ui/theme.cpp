#include "ui/theme.hpp"

#include <algorithm>

#include "core/globals.hpp"

const Theme themes[] = {
    {"Классика", "Classic", "голубой → фиолетовый", "blue → violet", {137, 180, 250}, {203, 166, 247}, {49, 50, 68}},
    {"Лес", "Forest", "зелёный → бирюзовый", "green → teal", {166, 227, 161}, {94, 200, 170}, {38, 58, 52}},
    {"Закат", "Sunset", "красный → оранжевый", "red → orange", {243, 139, 168}, {250, 179, 135}, {66, 46, 54}},
    {"Океан", "Ocean", "бирюзовый → синий", "cyan → blue", {137, 220, 235}, {116, 150, 250}, {36, 52, 72}},
    {"Неон", "Neon", "розовый → голубой", "pink → cyan", {245, 110, 200}, {110, 220, 250}, {58, 40, 72}},
    {"Золото", "Gold", "жёлтый → оранжевый", "yellow → orange", {249, 226, 175}, {250, 160, 100}, {62, 54, 40}},
    {"Моно", "Mono", "светло-серый → серый", "light grey → grey", {220, 224, 240}, {130, 136, 160}, {50, 52, 62}},
};
const int nThemes = sizeof themes / sizeof themes[0];

std::string fg(int r, int g, int b) {
    return "\x1b[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
}

const std::string RST = "\x1b[0m", BOLD = "\x1b[1m", NOBOLD = "\x1b[22m", KEYBG = "\x1b[48;2;69;71;90m";
const std::string GRN = fg(166, 227, 161), YEL = fg(249, 226, 175), RED = fg(243, 139, 168), DIM = fg(127, 132, 156),
                  TXT = fg(205, 214, 244), SUB = fg(166, 173, 200);
std::string ACC, VIO, SELBG;

void applyTheme(int i) {
    theme = std::clamp(i, 0, nThemes - 1);
    const Theme& t = themes[theme];
    ACC = fg(t.a[0], t.a[1], t.a[2]);
    VIO = fg(t.b[0], t.b[1], t.b[2]);
    SELBG = "\x1b[48;2;" + std::to_string(t.sel[0]) + ";" + std::to_string(t.sel[1]) + ";" + std::to_string(t.sel[2]) + "m";
}
