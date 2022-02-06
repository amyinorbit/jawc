cmake_minimum_required(VERSION 3.12)
project(jawc VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

set(SRC src/game.c src/main.c src/printing.c src/set.c src/stats.c src/dict.c)
# set(HDR src/game.h src/memory.h src/set.h)
set(ALL_SRC ${SRC})

add_subdirectory(lib/termutils)
add_executable(jawc ${ALL_SRC})
target_compile_options(jawc PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(jawc PRIVATE termutils::termutils)
