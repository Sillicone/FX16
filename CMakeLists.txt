cmake_minimum_required(VERSION 3.15)
project(FX16 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Все исходники лежат в src/ по подпапкам: core, ui, net, pkg, screens.
file(GLOB_RECURSE FX16_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(FX16 ${FX16_SOURCES})
target_include_directories(FX16 PRIVATE src)
target_compile_definitions(FX16 PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
if(MSVC)
  target_compile_options(FX16 PRIVATE /utf-8)
endif()
