cmake_minimum_required(VERSION 3.29)
project(GnomeCSSamples C)

set(CMAKE_C_STANDARD 11)

include(FetchContent)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
        GIT_TAG "master"
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(raylib)

include_directories(${raylib_SOURCE_DIR}/raylib/include)

FetchContent_Declare(
        GnomeCSLib
        GIT_REPOSITORY "https://github.com/kvitkaalexandr/gnomecs.git"
        GIT_TAG "7eb16e8963b9ccac0a6f3b70c46cb3a03c1ceaaf"
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(GnomeCSLib)

#scrollShooter
add_executable(GnomeCSScrollShooter scrollShooter.c)
target_include_directories(GnomeCSScrollShooter PRIVATE ${PROJECT_INCLUDE})
target_link_libraries(GnomeCSScrollShooter PRIVATE raylib)
target_link_libraries(GnomeCSScrollShooter PRIVATE GnomeCSLib)