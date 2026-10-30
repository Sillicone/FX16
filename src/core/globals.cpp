#include "core/globals.hpp"

fs::path home, configFile, installedFile;
bool quiet = false;
int lang = 0;
int theme = 0;

std::string tr(const char* ru, const char* en) { return lang ? en : ru; }
