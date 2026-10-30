#include "ui/menu.hpp"

#include <conio.h>

#include <algorithm>
#include <iostream>
#include <sstream>

#include "core/db.hpp"
#include "core/globals.hpp"
#include "ui/term.hpp"

int pick(const std::string& crumb, const std::vector<Item>& items, bool search, const std::vector<std::string>& info,
         const std::string& infoTitle, const std::string& infoCol, bool tabKey, std::function<void(int)> hover) {
    if (items.empty() && !search) return -1;
    std::string q;
    size_t cur = 0, start = 0;
    size_t nw = 0, tagw = 0;
    for (auto& it : items) {
        nw = std::max(nw, std::min<size_t>(vlen(it.main), 26));
        tagw = std::max(tagw, vlen(it.tag));
    }
    for (;;) {
        int tw, th;
        term(tw, th);
        size_t W = width();
        std::vector<size_t> vis;
        for (size_t i = 0; i < items.size(); ++i)
            if (lower(items[i].main + " " + items[i].sub + " " + items[i].tag).find(lower(q)) != std::string::npos)
                vis.push_back(i);
        size_t used = 3 + (info.empty() ? 0 : info.size() + 3) + (search ? 2 : 0) + 2;
        size_t H = (size_t)th > used + 3 ? th - used : 3;
        if (cur >= vis.size()) cur = vis.empty() ? 0 : vis.size() - 1;
        // Окно списка двигается, только когда курсор упирается в его край.
        if (cur < start) start = cur;
        if (cur >= start + H) start = cur - H + 1;
        if (start + H > vis.size()) start = vis.size() > H ? vis.size() - H : 0;
        if (hover && !vis.empty()) hover((int)vis[cur]);

        std::string o = "\x1b[H";
        header(o, crumb, W);
        ln(o);
        if (!info.empty()) {
            box(o, infoTitle, info, W, hover ? ACC : infoCol);
            ln(o);
        }
        if (search) {
            ln(o, " " + ACC + "› " + tr("Поиск", "Search") + "  " + TXT + q + ACC + "▏");
            ln(o);
        }
        size_t extra = nw + 4 + (tagw ? tagw + 1 : 0);
        size_t subw = W > extra ? W - extra : 0;
        for (size_t k = start; k < vis.size() && k < start + H; ++k) {
            const Item& it = items[vis[k]];
            bool sel = k == cur;
            std::string row = sel ? SELBG + ACC + BOLD + " ▌ " : "   ";
            row += (it.danger ? RED : TXT) + BOLD + fit(it.main, nw) + NOBOLD + " " + (sel ? SUB : DIM) + fit(it.sub, subw);
            if (tagw) row += " " + GRN + fit(it.tag, tagw);
            ln(o, row);
        }
        if (vis.empty()) ln(o, "   " + DIM + tr("ничего не найдено", "nothing found"));
        o += "\x1b[J";
        std::vector<std::pair<std::string, std::string>> keys = {{"↑↓", tr("выбор", "select")}, {"Enter", tr("ок", "ok")}, {"Esc", tr("назад", "back")}};
        if (tabKey) keys.push_back({"Tab", tr("сортировка", "sort")});
        keys.push_back({"Ctrl+C", tr("выход", "quit")});
        o += footer(keys, (search ? tr("печатайте для поиска", "type to search") + "   " : std::string()) +
                              (vis.empty() ? std::string() : std::to_string(cur + 1) + "/" + std::to_string(vis.size())));
        std::cout << o << std::flush;

        int c = key();
        if (c == 0 || c == 224) {
            c = _getch();
            if (c == 72 && cur > 0) --cur;
            else if (c == 80 && cur + 1 < vis.size()) ++cur;
            else if (c == 71) cur = 0;
            else if (c == 79 && !vis.empty()) cur = vis.size() - 1;
            else if (c == 73) cur = cur > H ? cur - H : 0;
            else if (c == 81 && !vis.empty()) cur = std::min(cur + H, vis.size() - 1);
        } else if (c == 13) {
            if (!vis.empty()) return (int)vis[cur];
        } else if (c == 27) return -1;
        else if (c == 9 && tabKey) return -2;
        else if (search && c == 8 && !q.empty()) q.pop_back();
        else if (search && c >= 32 && c < 127) q += (char)c;
    }
}

// Страница: шапка + рамка. Курсор остаётся под рамкой.
static std::string page(const std::string& crumb, const std::string& title, const std::vector<std::string>& lines, const std::string& c) {
    size_t W = width();
    std::string o = "\x1b[H";
    header(o, crumb, W);
    ln(o);
    box(o, title, lines, W, c);
    ln(o);
    return o + "\x1b[J";
}

void msg(const std::string& t, int kind) {
    std::string c = kind == 1 ? GRN : kind == 2 ? RED : ACC;
    std::string title = kind == 1 ? tr("✓ Готово", "✓ Done") : kind == 2 ? tr("✗ Ошибка", "✗ Error") : tr("◆ Сообщение", "◆ Message");
    std::vector<std::string> lines;
    std::stringstream ss(t);
    std::string l;
    while (std::getline(ss, l)) lines.push_back(cut(l, width() - 4));
    std::cout << page(tr("Результат", "Result"), title, lines, c)
              << footer({{tr("любая клавиша", "any key"), tr("продолжить", "continue")}, {"Ctrl+C", tr("выход", "quit")}}) << std::flush;
    key();
}

void status(const std::string& crumb, const std::string& title, const std::vector<std::string>& lines) {
    std::cout << page(crumb, title, lines, VIO) << std::flush;
}

std::string ask(const std::string& crumb, const std::string& q, const std::string& def) {
    size_t W = width();
    std::cout << page(crumb, tr("Ввод", "Input"),
                      {BOLD + cut(q, W - 4), DIM + cut(tr("Enter без ввода - по умолчанию:", "Enter on empty input uses the default:"), W - 4), YEL + cut(def, W - 4)}, VIO)
              << "\x1b[?25h " << ACC << BOLD << "❯ " << NOBOLD << TXT << std::flush;
    std::string s;
    std::getline(std::cin, s);
    std::cout << RST << "\x1b[?25l" << std::flush;
    return s.empty() ? def : s;
}
