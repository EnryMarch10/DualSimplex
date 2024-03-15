cmake_minimum_required(VERSION 3.20)
project(DualSimplex VERSION 0.1.0 LANGUAGES C CXX)

include_directories(
    include
    src
    src/utils
    src/List
    src/DualSimplex
)

set(SOURCES
    src/main.cpp
    src/utils/utils.cpp
    src/List/ArrayList.cpp
    src/DualSimplex/DualSimplex.cpp
)

add_executable(DualSimplex ${SOURCES})
