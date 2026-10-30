@echo off
cd /d "%~dp0"
rem Запущенный FX16.exe блокирует файл и ломает линковку - закрываем его (и старый cpkg.exe).
taskkill /f /im FX16.exe >nul 2>&1
taskkill /f /im cpkg.exe >nul 2>&1
rem Старый кэш CMake без языка C (до подключения QuickJS) пересоздаём с нуля.
if exist build\CMakeCache.txt (
    findstr /c:"CMAKE_C_COMPILER:" build\CMakeCache.txt >nul || rmdir /s /q build
)
if not exist build\build.ninja (
    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ || goto :fail
)
cmake --build build || goto :fail
copy /y build\FX16.exe FX16.exe >nul
if exist cpkg.exe del cpkg.exe
echo.
echo Done: %~dp0FX16.exe
pause
exit /b 0

:fail
echo.
echo Build failed.
pause
exit /b 1
