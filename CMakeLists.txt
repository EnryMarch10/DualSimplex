cmake_minimum_required(VERSION 3.20)
project(DualSimplex VERSION 0.1.0 LANGUAGES C CXX)

set(SRC src/DualSimplex/src)

include_directories(
    include
    ${SRC}
    ${SRC}/utils
    ${SRC}/Tableau
    ${SRC}/Solver
)

set(SOURCES
    ${SRC}/main.cpp
    ${SRC}/utils/utils.cpp
    ${SRC}/Tableau/TableauFilesManager.cpp
    ${SRC}/Solver/DualSolver.cpp
)

find_package (Eigen3 3.4 REQUIRED NO_MODULE)

add_executable(DualSimplex ${SOURCES})

target_link_libraries (DualSimplex Eigen3::Eigen)

# Set compiler flags
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(DualSimplex PRIVATE -Wall -Wextra -Werror -Wpedantic)
elseif (CMAKE_C_COMPILER_ID MATCHES "MSVC")
    target_compile_options(DualSimplex PRIVATE /W4 /Wall /WX)
endif()
