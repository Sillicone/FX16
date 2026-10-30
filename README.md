<div align="center">

# ◆ FX16

**A console package manager for Windows in the spirit of Chocolatey: pick tools with the arrow keys, then install, update, move and delete them.**

`C++17` · `CMake` · `Windows 10/11` · no external dependencies

</div>

---

## Features

- **A catalog of 42 programs** (ripgrep, fd, bat, fzf, neovim, helix, gh, lazygit, uv, ruff, ffmpeg, yt-dlp, rclone, Node.js, Bun, Deno, Python, Go, Docker and more) with live search and sorting on the `Tab` key.
- **Always the latest versions**: FX16 finds the newest release and the right Windows x64 `.zip` itself, so there is nothing to update in the catalog by hand.
- **Install by link**: paste the address of any GitHub repository and FX16 downloads its latest release.
- **Updates** for one program or for all of them at once, with a report of what changed.
- **Program card**: version, install date, folder, size, source and ready-made commands to run it.
- **Move and delete**: relocate a program to another folder (even to another drive) or remove it together with its files.
- **Themes and languages**: 7 color themes with a live preview, Russian and English.
- **JavaScript plugins**: drop a `.js` file into the `plugins` folder and add your own items to the main menu, with your own screens, lists and dialogs (powered by [QuickJS](https://github.com/quickjs-ng/quickjs)).
- **Catalog in `config.toml`**: add your own programs with a plain block of text.
- **Entry checker**: the `--check` mode trial-installs a program, inspects the result and removes it right away.

## Quick start

You need Windows 10/11 (they ship with `curl` and `tar`), CMake, Ninja and C and C++17 compilers (MinGW `gcc`/`g++` or MSVC). The JavaScript engine is bundled in `third_party`, nothing to install.

```bat
build.bat
FX16.exe
```

`build.bat` closes a running `FX16.exe`, builds the project and puts `FX16.exe` next to the `config` folder.

Manual build:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Controls

| Key | Action |
|---|---|
| `↑` `↓`, `Home`, `End`, `PgUp`, `PgDn` | move through the list |
| `Enter` | select |
| `Esc` | go back |
| any letters | search the catalog and the installed list |
| `Tab` | change the catalog sort order |
| `Ctrl+C` | quit from anywhere in the program |

## How it works

1. FX16 reads `config\config.toml` and shows the catalog sorted A to Z.
2. On install it asks GitHub (or an official website) for the latest release, picks a suitable archive, downloads it with `curl` and unpacks it with `tar`.
3. When updating, the old version is replaced only after the new one has been downloaded successfully.
4. The list of installed programs is kept in `installed.txt`.

> Programs are **not added to `PATH`**. The program card shows a `cd /d <folder>` command and usage examples.

## Catalog: `config/config.toml`

```toml
[[package]]
name = "ripgrep"
source = "BurntSushi/ripgrep"
description = "Fast recursive grep"
usage = "rg --version ; rg text . ; rg -i -t cpp main"
```

If the `config` folder or `config.toml` is missing, FX16 recreates them on startup from the default catalog (kept in [resources/default-config.toml](resources/default-config.toml) and embedded into the program at build time). The order of blocks does not matter, FX16 sorts them itself. The `source` field comes in three kinds:

| Kind | Meaning |
|---|---|
| `owner/repo` | the latest GitHub release with a Windows x64 `.zip` |
| `src:owner/repo` | the source archive of the latest release, for repositories without ready-made builds |
| `web:node` · `web:go` · `web:python` · `web:docker` | official websites (Node.js LTS, Go, Python from NuGet, static Docker) |

In `usage`, example commands are separated by semicolons. They are shown on the card of an installed program.

### Check a new entry

Before adding something to the catalog, check that it can actually be installed:

```bat
FX16.exe --check https://github.com/owner/repo
FX16.exe --check            :: the whole catalog
```

The result is `OK` (installed, an `.exe` is inside), `WARN` (no `.exe` inside) or `FAIL` (no suitable archive). The trial install is removed immediately.

## Settings and data

Language and theme are chosen in the **Settings** item of the main menu and are remembered.

Data lives in `%USERPROFILE%\FX16` (or in the folder from the `FX16_HOME` variable):

| File | Contents |
|---|---|
| `installed.txt` | installed programs |
| `settings.txt` | language and theme |
| `apps\` | programs, unless another folder is chosen |

The optional `GITHUB_TOKEN` variable raises the GitHub API rate limit from 60 to 5000 requests per hour.

## Plugins

Plugins live in the `plugins` folder next to `FX16.exe` (and next to `config`); its path is shown on the main screen. If the folder does not exist, FX16 creates it on startup and puts `example.js` into it. Every `*.js` file in the folder (for example your own `menu.js`) is loaded as an ES module at startup and its menu items appear in the main menu; files whose names start with `_` are skipped. A plugin adds an item to the main menu, and everything that happens after the user presses it is plain JavaScript.

The example is an ordinary file, [resources/example-plugin.js](resources/example-plugin.js), not code inside the program: it is embedded into `FX16.exe` at build time only to restore the `plugins` folder when it is missing.

```js
// plugins/menu.js
fx16.menu.add({
  title: "Quick actions",
  description: "my own menu item",
  tag: "js",                              // optional badge on the right
  onSelect() {                            // may be async
    const i = fx16.ui.pick("Quick actions", ["Say hello", "Ask a question"]);
    if (i === 0) fx16.ui.message("Hello from JavaScript!", "ok");
    if (i === 1) fx16.ui.message("You said: " + fx16.ui.ask("Say something", "hi"));
  },
});
```

### API (global `fx16`)

| Call | What it does |
|---|---|
| `fx16.menu.add({ title, description, tag, onSelect })` | adds an item to the main menu; `title` and `onSelect` are required |
| `fx16.ui.message(text, kind)` | shows a message box and waits for a key; `kind` is `"info"` (default), `"ok"` or `"error"` |
| `fx16.ui.pick(title, items, { search })` | shows a list and returns the chosen index, or `-1` on Esc; `items` are strings or `{ title, description, tag }`; `search: true` enables type-to-filter |
| `fx16.ui.ask(question, default)` | asks for a line of text and returns it (the default on empty input) |
| `fx16.ui.status(title, text)` | draws a status page without waiting, handy before a long operation |
| `fx16.packages()` | the catalog: `[{ name, source, description, usage }]` |
| `fx16.installed()` | installed programs: `[{ name, version, folder, installedAt, url }]` |
| `fx16.install(nameOrLink)` | runs the normal install dialog for a catalog name or a GitHub link |
| `fx16.lang` | `"ru"` or `"en"`, the current interface language |
| `fx16.version`, `fx16.home`, `fx16.pluginsDir` | version, data folder, plugins folder |
| `console.log(...)` | writes to `plugins\plugins.log` (the screen is never touched) |

QuickJS built-in modules are available too, so a plugin can read and write files, run programs and use environment variables:

```js
import * as std from "std";   // std.open, std.getenv, std.loadFile, std.popen ...
import * as os from "os";     // os.exec, os.readdir, os.stat, os.platform ...
import { helper } from "./lib.js";   // plugins can import each other
```

Syntax errors and exceptions never crash FX16. A broken plugin shows up as a red **Plugin errors** item in the main menu with the message and line number, and a runtime error in `onSelect` is shown in an error box and written to `plugins.log`.

> Plugins are trusted local code with the same rights as FX16 itself (`os.exec` and file access are available). Only install plugins you trust.

## Project structure

```
config/config.toml   program catalog (recreated on startup if missing)
plugins/             your JavaScript plugins (recreated with example.js if missing)
resources/          default catalog and example plugin, embedded into FX16.exe at build time
cmake/               template used for embedding
src/
  main.cpp           entry point, main menu, --check mode
  core/              state, installed.txt database, config.toml, settings
  ui/                themes, boxes and console output, lists and dialogs
  net/               curl, GitHub Releases, official websites
  pkg/               install, update, check
  plugins/           JavaScript plugin host (QuickJS bindings)
  screens/           screens: catalog, installed, settings
third_party/quickjs  bundled QuickJS-ng engine (MIT)
build.bat            one-click build
```

New `.cpp` files placed in `src/` are picked up by CMake automatically.

## Limitations

- Only programs shipped as `.zip` are supported. `.exe` and `.msi` installers (for example 7-Zip or Docker Desktop) are not.
- Only the latest regular release is used: no pre-releases and no version pinning.
- Docker is installed as a set of static binaries (`docker` and `dockerd`), without Docker Desktop.
- Python is installed from a NuGet package: run it from the `tools` folder.
- Search accepts Latin letters only.
- Paths with Cyrillic characters may be unreliable because of console encodings.

## License

[MIT](LICENSE). The bundled QuickJS-ng engine is also MIT-licensed, see [third_party/quickjs/LICENSE](third_party/quickjs/LICENSE).
