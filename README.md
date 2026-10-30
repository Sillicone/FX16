<div align="center">

# ◆ FX16

**Консольный менеджер программ для Windows в духе Chocolatey: выбирайте утилиты стрелками, ставьте, обновляйте, переносите и удаляйте.**

`C++17` · `CMake` · `Windows 10/11` · без внешних зависимостей

</div>

---

## Возможности

- **Каталог из 42 программ** (ripgrep, fd, bat, fzf, neovim, helix, gh, lazygit, uv, ruff, ffmpeg, yt-dlp, rclone, Node.js, Bun, Deno, Python, Go, Docker и другие) с поиском «на лету» и сортировкой по клавише `Tab`.
- **Всегда последние версии**: программа сама находит свежий релиз и нужный Windows x64 `.zip`, ничего не нужно обновлять в каталоге вручную.
- **Установка по ссылке**: вставьте адрес любого GitHub-репозитория, и FX16 скачает его последний релиз.
- **Обновление** одной программы или сразу всех, с отчётом о том, что поменялось.
- **Карточка программы**: версия, дата установки, папка, размер, источник и готовые примеры команд для запуска.
- **Перенос и удаление**: переместите программу в другую папку (в том числе на другой диск) или удалите её вместе с файлами.
- **Темы и языки**: 7 цветовых тем с живым предпросмотром, русский и English.
- **Каталог в `config.toml`**: добавляйте свои программы обычным блоком текста.
- **Проверка записей**: режим `--check` пробно ставит программу, смотрит содержимое и сразу удаляет.

## Быстрый старт

Нужны Windows 10/11 (в них уже есть `curl` и `tar`), CMake, Ninja и компилятор C++17 (MinGW `g++` или MSVC).

```bat
build.bat
FX16.exe
```

`build.bat` сам закрывает запущенный `FX16.exe`, собирает проект и кладёт `FX16.exe` рядом с папкой `config`.

Ручная сборка:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Управление

| Клавиша | Действие |
|---|---|
| `↑` `↓`, `Home`, `End`, `PgUp`, `PgDn` | перемещение по списку |
| `Enter` | выбрать |
| `Esc` | назад |
| любые буквы | поиск в каталоге и в списке установленных |
| `Tab` | смена сортировки в каталоге |
| `Ctrl+C` | выход из любой точки программы |

## Как это работает

1. FX16 читает `config\config.toml` и показывает каталог по алфавиту.
2. При установке он спрашивает у GitHub (или у официального сайта) последний релиз, выбирает подходящий архив, скачивает его через `curl` и распаковывает через `tar`.
3. Старая версия при обновлении заменяется только после успешной загрузки новой.
4. Список установленного хранится в `installed.txt`.

> Программы **не добавляются в `PATH`**. Карточка программы показывает команду `cd /d <папка>` и примеры запуска.

## Каталог: `config/config.toml`

```toml
[[package]]
name = "ripgrep"
source = "BurntSushi/ripgrep"
description = "Fast recursive grep"
usage = "rg --version ; rg текст . ; rg -i -t cpp main"
```

Порядок блоков не важен, FX16 сортирует их сам. Поле `source` бывает трёх видов:

| Вид | Значение |
|---|---|
| `owner/repo` | последний релиз GitHub с Windows x64 `.zip` |
| `src:owner/repo` | исходники последнего релиза, для репозиториев без готовых сборок |
| `web:node` · `web:go` · `web:python` · `web:docker` | официальные сайты (Node.js LTS, Go, Python из NuGet, статический Docker) |

В `usage` примеры команд разделяются точкой с запятой. Они показываются на карточке установленной программы.

### Проверить новую запись

Перед добавлением в каталог проверьте, что программа вообще устанавливается:

```bat
FX16.exe --check https://github.com/owner/repo
FX16.exe --check            :: весь каталог
```

Результат: `OK` (установилось, внутри есть `.exe`), `WARN` (нет `.exe`) или `FAIL` (нет подходящего архива). Пробная установка удаляется сразу.

## Настройки и данные

Язык и тема выбираются в пункте **«Настройки»** главного меню и запоминаются.

Данные лежат в `%USERPROFILE%\FX16` (или в папке из переменной `FX16_HOME`):

| Файл | Что внутри |
|---|---|
| `installed.txt` | установленные программы |
| `settings.txt` | язык и тема |
| `apps\` | программы, если не выбрана другая папка |

Переменная `GITHUB_TOKEN` (необязательно) поднимает лимит запросов к GitHub API с 60 до 5000 в час.

## Структура проекта

```
config/config.toml   каталог программ
src/
  main.cpp           точка входа, главное меню, режим --check
  core/              состояние, база installed.txt, config.toml, настройки
  ui/                темы, рамки и вывод в консоль, списки и диалоги
  net/               curl, GitHub Releases, официальные сайты
  pkg/               установка, обновление, проверка
  screens/           экраны: каталог, установленные, настройки
build.bat            сборка в один клик
```

Новые `.cpp` файлы в `src/` подхватываются CMake автоматически.

## Ограничения

- Поддерживаются только программы в виде `.zip`. Установщики `.exe` и `.msi` (например, 7-Zip или Docker Desktop) не поддерживаются.
- Берётся только последний обычный релиз, без предрелизов и без закрепления версии.
- Docker ставится как набор статических бинарников (`docker` и `dockerd`), без Docker Desktop.
- Python ставится из пакета NuGet: запускайте его из папки `tools`.
- Поиск принимает только латиницу.
- Пути с кириллицей могут работать нестабильно из-за кодировок консоли.

---

<details>
<summary><b>English summary</b></summary>

**FX16** is a console package manager for portable Windows tools, in the spirit of Chocolatey. It has a colorful keyboard-driven TUI (arrow keys, Enter, live search), 7 color themes, Russian/English UI and a catalog of 42 programs defined in `config/config.toml`.

It always installs the latest release: it queries GitHub Releases (or nodejs.org, go.dev, NuGet, docker.com), downloads the Windows x64 `.zip` with `curl` and unpacks it with `tar`. You can install any GitHub repository by link, update one or all programs, move or delete them, and see ready-made usage commands on each program card.

Build: run `build.bat` (needs CMake, Ninja and a C++17 compiler), then start `FX16.exe`. Check a new catalog entry with `FX16.exe --check owner/repo`.

</details>
