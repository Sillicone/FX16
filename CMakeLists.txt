cmake_minimum_required(VERSION 3.15)
project(FX16 C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Все исходники лежат в src/ по подпапкам: core, ui, net, pkg, plugins, screens.
file(GLOB_RECURSE FX16_SOURCES CONFIGURE_DEPENDS src/*.cpp)

# Движок JavaScript для плагинов: QuickJS-ng (единый файл), см. third_party/quickjs/README.md.
set(QJS_DIR ${CMAKE_SOURCE_DIR}/third_party/quickjs)
add_library(quickjs STATIC ${QJS_DIR}/quickjs-amalgam.c)
target_include_directories(quickjs PUBLIC ${QJS_DIR})
target_compile_definitions(quickjs PRIVATE QJS_BUILD_LIBC)  # включает встроенные модули std и os
if(NOT MSVC)
  target_compile_options(quickjs PRIVATE -w)
endif()
if(WIN32)
  target_link_libraries(quickjs PUBLIC ws2_32)
endif()
find_package(Threads REQUIRED)
target_link_libraries(quickjs PUBLIC Threads::Threads)

# Каталог по умолчанию и пример плагина вшиваются в программу из resources/:
# если config\config.toml или папки plugins рядом с FX16.exe нет, они создаются при запуске.
# Рабочие папки config/ и plugins/ сборке не нужны, их можно удалять.
file(READ ${CMAKE_SOURCE_DIR}/resources/default-config.toml FX16_DEFAULT_CONFIG)
file(READ ${CMAKE_SOURCE_DIR}/resources/example-plugin.js FX16_EXAMPLE_PLUGIN)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
             ${CMAKE_SOURCE_DIR}/resources/default-config.toml ${CMAKE_SOURCE_DIR}/resources/example-plugin.js)
configure_file(${CMAKE_SOURCE_DIR}/cmake/embedded.cpp.in ${CMAKE_BINARY_DIR}/generated/embedded.cpp @ONLY)
list(APPEND FX16_SOURCES ${CMAKE_BINARY_DIR}/generated/embedded.cpp)

add_executable(FX16 ${FX16_SOURCES})
target_include_directories(FX16 PRIVATE src)
target_link_libraries(FX16 PRIVATE quickjs)
target_compile_definitions(FX16 PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
if(MSVC)
  target_compile_options(FX16 PRIVATE /utf-8)
else()
  # Без зависимости от DLL MinGW: FX16.exe запускается на компьютере, где MinGW не установлен.
  target_link_options(FX16 PRIVATE -static)
endif()
