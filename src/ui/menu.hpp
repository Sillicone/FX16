// Экраны ввода/вывода: списки со стрелками и поиском, сообщения, ввод строки.
#pragma once
#include <functional>
#include <string>
#include <vector>

#include "ui/theme.hpp"

struct Item {
    std::string main, sub, tag;
    bool danger = false;
};

// Универсальное меню со списком. Возвращает индекс в items или -1 (Esc).
// info - карточка над списком, search - фильтр по набранному тексту, tabKey - Tab возвращает -2,
// hover - вызывается с индексом элемента под курсором перед каждой отрисовкой (предпросмотр темы).
int pick(const std::string& crumb, const std::vector<Item>& items, bool search = false,
         const std::vector<std::string>& info = {}, const std::string& infoTitle = "",
         const std::string& infoCol = ACC, bool tabKey = false, std::function<void(int)> hover = nullptr);

// kind: 0 - информация, 1 - успех, 2 - ошибка. Ждёт нажатия клавиши.
void msg(const std::string& t, int kind = 0);

// Страница с рамкой без ожидания (для шагов "ищу релиз", "загрузка").
void status(const std::string& crumb, const std::string& title, const std::vector<std::string>& lines);

// Запрос строки; пустой ввод возвращает def.
std::string ask(const std::string& crumb, const std::string& q, const std::string& def);
