#include "core/globals.hpp"
#include "core/settings.hpp"
#include "screens/screens.hpp"
#include "ui/menu.hpp"
#include "ui/term.hpp"

// Настройки: язык интерфейса и цветовая тема (с живым предпросмотром при движении курсора).
void settings() {
    for (;;) {
        const Theme& th = themes[theme];
        std::vector<Item> items = {{tr("Язык", "Language"), tr("язык интерфейса", "interface language"), lang ? "English" : "Русский"},
                                   {tr("Тема", "Theme"), tr("цвета и градиент", "colors and gradient"), lang ? th.en : th.ru},
                                   {tr("Назад", "Back"), ""}};
        std::string crumb = tr("Настройки", "Settings");
        int c = pick(crumb, items, false,
                     {gradient(rep("█", 40)), ACC + "● " + VIO + "● " + GRN + "● " + YEL + "● " + RED + "●"}, tr("Предпросмотр", "Preview"));
        if (c == 0) {
            int l = pick(crumb + " › " + tr("Язык", "Language"), {{"Русский", "", lang == 0 ? "✓" : ""}, {"English", "", lang == 1 ? "✓" : ""}});
            if (l >= 0) { lang = l; saveSettings(); }
        } else if (c == 1) {
            int old = theme;
            std::vector<Item> ti;
            for (int i = 0; i < nThemes; ++i)
                ti.push_back({lang ? themes[i].en : themes[i].ru, lang ? themes[i].descEn : themes[i].descRu, i == old ? "✓" : ""});
            int t = pick(crumb + " › " + tr("Тема", "Theme"), ti, false, {}, "", ACC, false, [](int i) { applyTheme(i); });
            applyTheme(t >= 0 ? t : old);  // Esc возвращает прежнюю тему
            if (t >= 0) saveSettings();
        } else return;
    }
}
