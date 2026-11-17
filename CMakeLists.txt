
cmake_minimum_required(VERSION 3.25)

project(RetroGamesLauncher C)

add_executable(RetroGamesLauncher
    src/main.c
)

set_target_properties(RetroGamesLauncher PROPERTIES
    C_STANDARD 17
)

target_compile_options(RetroGamesLauncher PRIVATE
    -Wall
    -Wextra
)

xsdk_add_xex(RetroGamesLauncher
    TITLE_ID 0x58590001
    NAME "Retro Games Launcher"
    VERSION 0.1.0.0
)
