cmake_minimum_required(VERSION 3.20)
project(MyCS2Module LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# DLL проект
add_library(MyCS2Module SHARED
    src/dllmain.cpp
    include/OffsetWatcher.hpp
    include/Memory.hpp
    include/GameTypes.hpp
    src/EntityReader.hpp
    src/Renderer.hpp
)

target_include_directories(MyCS2Module PRIVATE
    include
    src
)

# Системные библиотеки Windows
target_link_libraries(MyCS2Module PRIVATE
    user32
    gdi32
)
