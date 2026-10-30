#include "ui/term.hpp"

#include <conio.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>

#include "core/globals.hpp"
#include "ui/theme.hpp"

// Видимая длина: коды ANSI не считаются, UTF-8 считается по символам.
size_t vlen(const std::string& s) {
    size_t n = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = s[i];
        if (c == 0x1b) { while (i < s.size() && s[i] != 'm') ++i; }
        else if ((c & 0xC0) != 0x80) ++n;
    }
    return n;
}

// Обрезка обычного текста (без ANSI) до w символов с многоточием.
std::string cut(const std::string& s, size_t w) {
    if (vlen(s) <= w) return s;
    size_t n = 0, i = 0;
    for (; i < s.size(); ++i)
        if (((unsigned char)s[i] & 0xC0) != 0x80) {
            if (n == (w ? w - 1 : 0)) break;
            ++n;
        }
    return s.substr(0, i) + "…";
}

std::string fit(const std::string& s, size_t w) {
    std::string c = cut(s, w);
    return c + std::string(w - vlen(c), ' ');
}

std::string rep(const std::string& s, size_t n) {
    std::string o;
    while (n--) o += s;
    return o;
}

std::string gradient(const std::string& s) {
    const Theme& th = themes[theme];
    size_t n = vlen(s), k = 0;
    std::string o;
    for (size_t i = 0; i < s.size();) {
        size_t l = 1;
        while (i + l < s.size() && ((unsigned char)s[i + l] & 0xC0) == 0x80) ++l;
        double t = n > 1 ? (double)k / (n - 1) : 0;
        o += fg(th.a[0] + (int)((th.b[0] - th.a[0]) * t), th.a[1] + (int)((th.b[1] - th.a[1]) * t),
                th.a[2] + (int)((th.b[2] - th.a[2]) * t)) + s.substr(i, l);
        i += l;
        ++k;
    }
    return o;
}

void term(int& w, int& h) {
    CONSOLE_SCREEN_BUFFER_INFO i;
    w = 100, h = 30;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &i)) {
        w = i.srWindow.Right - i.srWindow.Left + 1;
        h = i.srWindow.Bottom - i.srWindow.Top + 1;
    }
}

size_t width() {
    int w, h;
    term(w, h);
    return (size_t)std::clamp(w - 2, 40, 110);
}

void ln(std::string& o, const std::string& s) { o += s + RST + "\x1b[K\n"; }

void header(std::string& o, const std::string& crumb, size_t W) {
    std::string left = " " + BOLD + gradient("◆ FX16") + NOBOLD + DIM + "  " + tr("менеджер программ", "program manager");
    std::string right = ACC + crumb + " ";
    size_t pad = W > vlen(left) + vlen(right) ? W - vlen(left) - vlen(right) : 1;
    ln(o, left + std::string(pad, ' ') + right);
    ln(o, " " + gradient(rep("━", W)));
}

// Рамка с заголовком в верхней границе. Строки должны быть не шире W-4.
void box(std::string& o, const std::string& title, const std::vector<std::string>& lines, size_t W, const std::string& c) {
    std::string t = title.empty() ? "" : " " + title + " ";
    size_t tl = vlen(t);
    ln(o, " " + c + "╭─" + TXT + BOLD + t + NOBOLD + c + rep("─", W > tl + 3 ? W - 3 - tl : 0) + "╮");
    for (auto& l : lines) {
        size_t v = vlen(l);
        ln(o, " " + c + "│ " + TXT + l + std::string(W - 4 > v ? W - 4 - v : 0, ' ') + c + " │");
    }
    ln(o, " " + c + "╰" + rep("─", W - 2) + "╯");
}

std::string kv(const std::string& k, const std::string& v) {
    return DIM + fit(k, 14) + TXT + cut(v, width() - 4 - 14);
}

std::string footer(const std::vector<std::pair<std::string, std::string>>& keys, const std::string& right) {
    int w, h;
    term(w, h);
    std::string o = "\x1b[" + std::to_string(h) + ";1H ";
    for (auto& k : keys) o += KEYBG + TXT + " " + k.first + " " + RST + " " + DIM + k.second + "  " + RST;
    return o + DIM + right + RST + "\x1b[K";
}

// Альтернативный экран: перерисовки не попадают в историю прокрутки.
void leave() { std::cout << "\x1b[?25h\x1b[?1049l" << std::flush; }

BOOL WINAPI onCtrlC(DWORD) {
    leave();
    return FALSE;  // дальше стандартное завершение процесса
}

int key() {
    int c = _getch();
    if (c == 3) std::exit(0);  // Ctrl-C (atexit вернёт обычный экран)
    return c;
}
