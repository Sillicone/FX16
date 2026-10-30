#include "plugins/plugins.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

#include "quickjs-libc.h"
#include "quickjs.h"

#include "core/config.hpp"
#include "core/db.hpp"
#include "core/embedded.hpp"
#include "core/globals.hpp"
#include "net/net.hpp"
#include "pkg/installer.hpp"
#include "ui/menu.hpp"

namespace {

struct Entry {
    PluginEntry info;
    JSValue fn;
    std::string plugin;  // имя файла плагина
};

JSRuntime* rt = nullptr;
JSContext* ctx = nullptr;
std::vector<Entry> entries;
std::vector<PluginEntry> publicEntries;
std::vector<std::string> errors;
std::string curPlugin;  // плагин, чей код сейчас выполняется
fs::path dir;


void logLine(const std::string& s) { std::ofstream(dir / "plugins.log", std::ios::app) << s << "\n"; }

std::string str(JSContext* c, JSValueConst v) {
    const char* s = JS_ToCString(c, v);
    std::string r = s ? s : "";
    if (s) JS_FreeCString(c, s);
    return r;
}

std::string prop(JSContext* c, JSValueConst o, const char* k) {
    JSValue v = JS_GetPropertyStr(c, o, k);
    std::string r = JS_IsUndefined(v) || JS_IsNull(v) ? "" : str(c, v);
    JS_FreeValue(c, v);
    return r;
}

bool flag(JSContext* c, JSValueConst o, const char* k) {
    JSValue v = JS_GetPropertyStr(c, o, k);
    bool b = JS_ToBool(c, v) > 0;
    JS_FreeValue(c, v);
    return b;
}

// Текст последнего исключения: сообщение и стек вызовов.
std::string errText() {
    JSValue e = JS_GetException(ctx);
    std::string msgText = prop(ctx, e, "message"), stack = prop(ctx, e, "stack");
    std::string r = msgText.empty() ? str(ctx, e) : msgText;
    if (!stack.empty()) r += "\n" + stack;
    JS_FreeValue(ctx, e);
    return r;
}

// Доводит результат до конца (Promise, async-функции); false и текст ошибки при исключении.
bool settle(JSValue v, std::string& err) {
    if (!JS_IsException(v) && JS_IsPromise(v)) v = js_std_await(ctx, v);
    bool bad = JS_IsException(v);
    if (bad) err = errText();
    else JS_FreeValue(ctx, v);
    return !bad;
}

std::vector<std::string> lines(const std::string& t) {
    std::vector<std::string> v;
    std::stringstream ss(t);
    for (std::string l; std::getline(ss, l);) v.push_back(l);
    return v;
}

// ---- fx16.menu / fx16.ui / fx16.packages / fx16.installed / fx16.install

JSValue jsMenuAdd(JSContext* c, JSValueConst, int argc, JSValueConst* argv) {
    if (argc < 1 || !JS_IsObject(argv[0])) return JS_ThrowTypeError(c, "fx16.menu.add(item): item must be an object");
    JSValue fn = JS_GetPropertyStr(c, argv[0], "onSelect");
    std::string title = prop(c, argv[0], "title");
    if (title.empty() || !JS_IsFunction(c, fn)) {
        JS_FreeValue(c, fn);
        return JS_ThrowTypeError(c, "fx16.menu.add: 'title' (string) and 'onSelect' (function) are required");
    }
    entries.push_back({{title, prop(c, argv[0], "description"), prop(c, argv[0], "tag")}, fn, curPlugin});
    publicEntries.push_back(entries.back().info);
    return JS_UNDEFINED;
}

JSValue jsMessage(JSContext* c, JSValueConst, int argc, JSValueConst* argv) {
    std::string kind = argc > 1 ? str(c, argv[1]) : "";
    msg(argc > 0 ? str(c, argv[0]) : "", kind == "ok" || kind == "success" ? 1 : kind == "error" ? 2 : 0);
    return JS_UNDEFINED;
}

JSValue jsStatus(JSContext* c, JSValueConst, int argc, JSValueConst* argv) {
    status(tr("Плагины", "Plugins") + " › " + curPlugin, argc > 0 ? str(c, argv[0]) : "", argc > 1 ? lines(str(c, argv[1])) : std::vector<std::string>{});
    return JS_UNDEFINED;
}

// fx16.ui.pick(title, items, {search}) -> индекс или -1; items: строки или {title, description, tag}
JSValue jsPick(JSContext* c, JSValueConst, int argc, JSValueConst* argv) {
    if (argc < 2 || !JS_IsObject(argv[1])) return JS_ThrowTypeError(c, "fx16.ui.pick(title, items): items must be an array");
    std::vector<Item> items;
    JSValue lenV = JS_GetPropertyStr(c, argv[1], "length");
    int32_t n = 0;
    JS_ToInt32(c, &n, lenV);
    JS_FreeValue(c, lenV);
    for (int32_t i = 0; i < n; ++i) {
        JSValue it = JS_GetPropertyUint32(c, argv[1], i);
        if (JS_IsObject(it)) items.push_back({prop(c, it, "title"), prop(c, it, "description"), prop(c, it, "tag")});
        else items.push_back({str(c, it), "", ""});
        JS_FreeValue(c, it);
    }
    bool search = argc > 2 && JS_IsObject(argv[2]) && flag(c, argv[2], "search");
    return JS_NewInt32(c, pick(tr("Плагины", "Plugins") + " › " + str(c, argv[0]), items, search));
}

JSValue jsAsk(JSContext* c, JSValueConst, int argc, JSValueConst* argv) {
    std::string r = ask(tr("Плагины", "Plugins") + " › " + curPlugin, argc > 0 ? str(c, argv[0]) : "", argc > 1 ? str(c, argv[1]) : "");
    return JS_NewString(c, r.c_str());
}

JSValue jsPackages(JSContext* c, JSValueConst, int, JSValueConst*) {
    JSValue arr = JS_NewArray(c);
    uint32_t i = 0;
    for (auto& p : loadPackages()) {
        JSValue o = JS_NewObject(c);
        JS_SetPropertyStr(c, o, "name", JS_NewString(c, p[0].c_str()));
        JS_SetPropertyStr(c, o, "source", JS_NewString(c, p[1].c_str()));
        JS_SetPropertyStr(c, o, "description", JS_NewString(c, p[2].c_str()));
        JS_SetPropertyStr(c, o, "usage", JS_NewString(c, p[3].c_str()));
        JS_SetPropertyUint32(c, arr, i++, o);
    }
    return arr;
}

JSValue jsInstalled(JSContext* c, JSValueConst, int, JSValueConst*) {
    JSValue arr = JS_NewArray(c);
    uint32_t i = 0;
    for (auto& r : readDb(installedFile)) {
        JSValue o = JS_NewObject(c);
        JS_SetPropertyStr(c, o, "name", JS_NewString(c, r[0].c_str()));
        JS_SetPropertyStr(c, o, "version", JS_NewString(c, r[1].c_str()));
        JS_SetPropertyStr(c, o, "folder", JS_NewString(c, r[2].c_str()));
        JS_SetPropertyStr(c, o, "installedAt", JS_NewString(c, col(r, 3).c_str()));
        JS_SetPropertyStr(c, o, "url", JS_NewString(c, col(r, 4).c_str()));
        JS_SetPropertyUint32(c, arr, i++, o);
    }
    return arr;
}

// fx16.install("ripgrep") или fx16.install("https://github.com/owner/repo") - обычный диалог установки.
JSValue jsInstall(JSContext* c, JSValueConst, int argc, JSValueConst* argv) {
    std::string what = argc > 0 ? str(c, argv[0]) : "";
    for (auto& p : loadPackages())
        if (lower(p[0]) == lower(what)) { install(p); return JS_UNDEFINED; }
    std::string repo = repoOf(what);
    if (repo.empty()) return JS_ThrowTypeError(c, "fx16.install: not a catalog name or GitHub link: %s", what.c_str());
    install({repo.substr(repo.find('/') + 1), repo, "plugin"});
    return JS_UNDEFINED;
}

// console.log/info/warn/error пишут в plugins.log, чтобы не ломать экран.
JSValue jsLog(JSContext* c, JSValueConst, int argc, JSValueConst* argv) {
    std::string s = "[" + curPlugin + "]";
    for (int i = 0; i < argc; ++i) {
        JSValue j = JS_IsObject(argv[i]) ? JS_JSONStringify(c, argv[i], JS_UNDEFINED, JS_UNDEFINED) : JS_UNDEFINED;
        s += " " + (JS_IsString(j) ? str(c, j) : str(c, argv[i]));  // объекты - как JSON
        JS_FreeValue(c, j);
    }
    logLine(s);
    return JS_UNDEFINED;
}

void setFn(JSValueConst obj, const char* name, JSCFunction* fn, int len) {
    JS_SetPropertyStr(ctx, obj, name, JS_NewCFunction(ctx, fn, name, len));
}

void setupGlobals() {
    JSValue g = JS_GetGlobalObject(ctx);
    JSValue fx = JS_NewObject(ctx), menu = JS_NewObject(ctx), ui = JS_NewObject(ctx), con = JS_NewObject(ctx);
    setFn(menu, "add", jsMenuAdd, 1);
    setFn(ui, "message", jsMessage, 2);
    setFn(ui, "status", jsStatus, 2);
    setFn(ui, "pick", jsPick, 3);
    setFn(ui, "ask", jsAsk, 2);
    setFn(fx, "packages", jsPackages, 0);
    setFn(fx, "installed", jsInstalled, 0);
    setFn(fx, "install", jsInstall, 1);
    for (const char* n : {"log", "info", "warn", "error", "debug"}) setFn(con, n, jsLog, 1);
    JS_SetPropertyStr(ctx, fx, "menu", menu);
    JS_SetPropertyStr(ctx, fx, "ui", ui);
    JS_SetPropertyStr(ctx, fx, "version", JS_NewString(ctx, "1.0"));
    JS_SetPropertyStr(ctx, fx, "home", JS_NewString(ctx, home.string().c_str()));
    JS_SetPropertyStr(ctx, fx, "pluginsDir", JS_NewString(ctx, dir.string().c_str()));
    JS_SetPropertyStr(ctx, g, "fx16", fx);
    JS_SetPropertyStr(ctx, g, "console", con);
    JS_FreeValue(ctx, g);
}

void setLang() {
    JSValue g = JS_GetGlobalObject(ctx), fx = JS_GetPropertyStr(ctx, g, "fx16");
    JS_SetPropertyStr(ctx, fx, "lang", JS_NewString(ctx, lang ? "en" : "ru"));
    JS_FreeValue(ctx, fx);
    JS_FreeValue(ctx, g);
}

void loadFile(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    std::string code = ss.str(), name = p.generic_string(), err;
    curPlugin = p.filename().string();
    if (!settle(JS_Eval(ctx, code.c_str(), code.size(), name.c_str(), JS_EVAL_TYPE_MODULE), err)) {
        errors.push_back(curPlugin + ":\n" + err);
        logLine("[load error] " + curPlugin + ": " + err);
    }
}

}  // namespace

void initPlugins() {
    // Папка plugins лежит рядом с папкой config; если её нет, создаём и кладём пример (вшит при сборке из plugins/example.js).
    fs::path base = configFile.empty() ? appDir() : configFile.parent_path().parent_path();
    dir = base / "plugins";
    bool fresh = !fs::exists(dir);
    std::error_code ec;
    fs::create_directories(dir, ec);
    if (fresh) std::ofstream(dir / "example.js", std::ios::binary) << kExamplePlugin;

    rt = JS_NewRuntime();
    js_std_init_handlers(rt);
    JS_SetModuleLoaderFunc2(rt, nullptr, js_module_loader, js_module_check_attributes, nullptr);
    ctx = JS_NewContext(rt);
    js_init_module_std(ctx, "std");
    js_init_module_os(ctx, "os");
    setupGlobals();
    setLang();

    std::vector<fs::path> files;
    for (auto& e : fs::directory_iterator(dir, ec))
        if (e.is_regular_file() && e.path().extension() == ".js" && e.path().filename().string()[0] != '_') files.push_back(e.path());
    std::sort(files.begin(), files.end());
    for (auto& f : files) loadFile(f);
}

const std::vector<PluginEntry>& pluginEntries() { return publicEntries; }
const std::vector<std::string>& pluginErrors() { return errors; }
std::string pluginsDir() { return dir.string(); }

std::string pluginsSummary() {
    return tr("пунктов меню: ", "menu items: ") + std::to_string(publicEntries.size()) +
           (errors.empty() ? "" : tr(" · ошибок: ", " · errors: ") + std::to_string(errors.size()));
}

void runPluginEntry(size_t i) {
    if (i >= entries.size()) return;
    setLang();
    curPlugin = entries[i].plugin;
    std::string err;
    if (!settle(JS_Call(ctx, entries[i].fn, JS_UNDEFINED, 0, nullptr), err)) {
        logLine("[runtime error] " + entries[i].info.title + ": " + err);
        msg(entries[i].info.title + "\n" + err, 2);
    }
}
