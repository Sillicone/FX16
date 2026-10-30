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
- **Catalog in `config.toml`**: add your own programs with a plain block of text.
- **Entry checker**: the `--check` mode trial-installs a program, inspects the result and removes it right away.

## Quick start

You need Windows 10/11 (they ship with `curl` and `tar`), CMake, Ninja and a C++17 compiler (MinGW `g++` or MSVC).

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

The order of blocks does not matter, FX16 sorts them itself. The `source` field comes in three kinds:

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

## Project structure

```
config/config.toml   program catalog
src/
  main.cpp           entry point, main menu, --check mode
  core/              state, installed.txt database, config.toml, settings
  ui/                themes, boxes and console output, lists and dialogs
  net/               curl, GitHub Releases, official websites
  pkg/               install, update, check
  screens/           screens: catalog, installed, settings
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

[MIT](LICENSE)
