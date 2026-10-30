#include "net/net.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>

#include "core/db.hpp"
#include "core/globals.hpp"

bool run(const std::string& cmd) {
    return std::system(("\"" + cmd + "\"").c_str()) == 0;  // внешние кавычки нужны cmd /c
}

// Значение строки JSON после ключа key, начиная с from; npos в from при отсутствии.
static std::string jsonStr(const std::string& s, const std::string& key, size_t& from) {
    size_t k = s.find(key, from);
    if (k == std::string::npos) { from = k; return ""; }
    size_t a = s.find('"', s.find(':', k)) + 1, b = s.find('"', a);
    from = b;
    return s.substr(a, b - a);
}

// Выбор лучшего Windows x64 .zip среди файлов релиза. Больше очков - лучше, 0 - не подходит.
static int score(const std::string& url) {
    std::string n = lower(url.substr(url.rfind('/') + 1));
    if (n.size() < 4 || n.compare(n.size() - 4, 4, ".zip") != 0 || n.find("win") == std::string::npos) return 0;
    for (const char* bad : {"arm", "aarch", "i686", "i386", "32", "sha256", "debug", "linux", "darwin", "macos", "shared", "pdb", "baseline", "profile"})
        if (n.find(bad) != std::string::npos) return 0;
    int s = 1;
    for (const char* good : {"x86_64", "amd64", "x64", "win64"})
        if (n.find(good) != std::string::npos) { s += 2; break; }
    if (n.find("msvc") != std::string::npos) ++s;
    return s;
}

static bool httpGet(const std::string& url, std::string& body, const std::string& extra = "") {
    fs::path js = home / "release.json";
    bool ok = run("curl -fsSL" + extra + " -o \"" + js.string() + "\" \"" + url + "\"");
    std::ifstream in(js, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    std::error_code ec;
    fs::remove(js, ec);
    body = ss.str();
    return ok;
}

// Официальные сайты, которых нет в GitHub Releases: Node.js, Go, Docker и Python (пакет NuGet).
// shortcut: парсинг текста вместо JSON, при смене формата сайтов потребуется правка.
static bool resolveWeb(const std::string& kind, std::string& tag, std::string& url) {
    std::string s;
    size_t p = 0;
    if (kind == "node") {  // последняя LTS-версия
        if (!httpGet("https://nodejs.org/dist/index.json", s)) return false;
        p = s.find("\"lts\":\"");
        if (p == std::string::npos || (p = s.rfind("\"version\"", p)) == std::string::npos) return false;
        tag = jsonStr(s, "\"version\"", p);
        url = "https://nodejs.org/dist/" + tag + "/node-" + tag + "-win-x64.zip";
    } else if (kind == "go") {
        if (!httpGet("https://go.dev/dl/?mode=json", s)) return false;
        tag = jsonStr(s, "\"version\"", p);
        url = "https://go.dev/dl/" + tag + ".windows-amd64.zip";
    } else if (kind == "python") {  // пакет "python" на nuget.org: полный CPython с pip в папке tools
        if (!httpGet("https://api.nuget.org/v3-flatcontainer/python/index.json", s)) return false;
        std::vector<std::string> v;
        for (p = s.find('['); (p = s.find('"', p)) != std::string::npos;) {
            size_t b = s.find('"', p + 1);
            if (b == std::string::npos) break;
            v.push_back(s.substr(p + 1, b - p - 1));
            p = b + 1;
        }
        for (auto it = v.rbegin(); it != v.rend() && tag.empty(); ++it)  // последняя стабильная X.Y.Z
            if (it->find_first_not_of("0123456789.") == std::string::npos && std::count(it->begin(), it->end(), '.') == 2) tag = *it;
        url = "https://api.nuget.org/v3-flatcontainer/python/" + tag + "/python." + tag + ".nupkg";
    } else if (kind == "docker") {  // статические бинарники docker + dockerd, без Docker Desktop
        const std::string base = "https://download.docker.com/win/static/stable/x86_64/";
        if (!httpGet(base, s)) return false;
        int best[3] = {0, 0, 0};
        for (; (p = s.find("docker-", p)) != std::string::npos; p += 7) {
            int a = 0, b = 0, c = 0, n = 0;
            if (sscanf(s.c_str() + p + 7, "%d.%d.%d.zip%n", &a, &b, &c, &n) == 3 && n &&
                std::vector<int>{a, b, c} > std::vector<int>(best, best + 3)) { best[0] = a; best[1] = b; best[2] = c; }
        }
        if (!best[0]) return false;
        tag = std::to_string(best[0]) + "." + std::to_string(best[1]) + "." + std::to_string(best[2]);
        url = base + "docker-" + tag + ".zip";
    }
    return !tag.empty() && !url.empty();
}

// shortcut: только последний релиз и только .zip; закрепление версии и .exe/.msi - при необходимости.
bool resolve(std::string repo, std::string& tag, std::string& url) {
    if (repo.rfind("web:", 0) == 0) return resolveWeb(repo.substr(4), tag, url);
    bool src = repo.rfind("src:", 0) == 0;  // "src:owner/repo" - репозиторий без готовых сборок, берём исходники релиза
    if (src) repo = repo.substr(4);
    const char* tok = std::getenv("GITHUB_TOKEN");  // необязательно: поднимает лимит API с 60 запросов/час
    std::string auth = tok ? std::string(" -H \"Authorization: Bearer ") + tok + "\"" : "";
    std::string s;
    if (!httpGet("https://api.github.com/repos/" + repo + "/releases/latest", s, auth)) return false;
    size_t p = 0;
    tag = jsonStr(s, "\"tag_name\"", p);
    if (src) {
        url = "https://github.com/" + repo + "/archive/refs/tags/" + tag + ".zip";
        return !tag.empty();
    }
    int best = 0;
    p = 0;
    for (;;) {
        std::string u = jsonStr(s, "\"browser_download_url\"", p);
        if (p == std::string::npos) break;
        if (score(u) > best) { best = score(u); url = u; }
    }
    return best > 0 && !tag.empty();
}

// Строка попадает в командную строку curl, поэтому допускаются только безопасные символы.
std::string repoOf(std::string s) {
    size_t g = s.find("github.com/");
    if (g != std::string::npos) s = s.substr(g + 11);
    else if (s.find("://") != std::string::npos) return "";
    Row seg;
    std::stringstream ss(s);
    std::string f;
    while (seg.size() < 2 && std::getline(ss, f, '/')) {
        f = f.substr(0, f.find_first_of("?# "));
        if (!f.empty()) seg.push_back(f);
    }
    if (seg.size() < 2) return "";
    if (seg[1].size() > 4 && seg[1].compare(seg[1].size() - 4, 4, ".git") == 0) seg[1].resize(seg[1].size() - 4);
    std::string r = seg[0] + "/" + seg[1];
    for (char c : r)
        if (!(std::isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '/')) return "";
    return r;
}
