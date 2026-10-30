#include <algorithm>

#include "core/config.hpp"
#include "core/db.hpp"
#include "pkg/installer.hpp"
#include "screens/screens.hpp"
#include "ui/menu.hpp"

void catalog() {
    static int mode = 0;  // выбранная сортировка живёт до выхода из программы
    for (;;) {
        std::vector<std::string> modes = {tr("имя А→Я", "name A→Z"), tr("имя Я→А", "name Z→A"), tr("установленные сверху", "installed first"),
                                          tr("неустановленные сверху", "not installed first"), tr("как в config.toml", "as in config.toml")};
        auto cat = loadPackages();
        if (cat.empty())
            return msg(tr("Каталог пуст: не найден или пуст файл config\\config.toml\nрядом с FX16.exe.",
                          "Catalog is empty: config\\config.toml is missing or empty\nnext to FX16.exe."), 2);
        auto inst = readDb(installedFile);
        std::stable_sort(cat.begin(), cat.end(), [&](const Row& a, const Row& b) {
            bool ia = findRow(inst, a[0]) >= 0, ib = findRow(inst, b[0]) >= 0;
            if (mode == 4) return false;
            if (mode >= 2 && ia != ib) return mode == 2 ? ia : !ia;
            return mode == 1 ? lower(a[0]) > lower(b[0]) : lower(a[0]) < lower(b[0]);
        });
        std::vector<Item> items;
        for (auto& p : cat) {
            int i = findRow(inst, p[0]);
            items.push_back({p[0], p[2], i >= 0 ? "✓ " + inst[i][1] : ""});
        }
        int i = pick(tr("Каталог · ", "Catalog · ") + std::to_string(cat.size()) + tr(" программ · сортировка: ", " programs · sort: ") + modes[mode],
                     items, true, {}, "", ACC, true);
        if (i == -2) { mode = (mode + 1) % 5; continue; }
        if (i < 0) return;
        install(cat[i]);
    }
}
