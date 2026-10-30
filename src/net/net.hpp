// Сеть и источники программ: curl, GitHub Releases, Node.js / Go / Python / Docker.
#pragma once
#include <string>

bool run(const std::string& cmd);  // выполняет команду через cmd /c

// Определяет последнюю версию и ссылку на архив по источнику из config.toml:
// "owner/repo", "src:owner/repo" (исходники релиза) или "web:node|go|python|docker".
bool resolve(std::string repo, std::string& tag, std::string& url);

// "owner/repo" из ссылки на GitHub или из самой записи owner/repo; пусто, если не похоже.
std::string repoOf(std::string s);
