cmake_minimum_required(VERSION 3.10)

project(iterators)
set(CMAKE_C_STANDARD 11)
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_SOURCE_DIR}/Modules/
)

include(CTest)
include(GroupFiles)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# FIXME: make this portable. Right now it will only work
# for gcc and clang.
add_compile_options(
        -Wcast-align
        -Wcast-qual
        -Wconversion
        -Wexit-time-destructors
        -Wglobal-constructors
        -Wmissing-noreturn
        -Wmissing-prototypes
        -Wno-missing-braces
        -Wold-style-cast
        -Wshadow
        -Wweak-vtables
        -Werror
        -Wall
        -O3
)

add_subdirectory(libs)
add_subdirectory(cps)


