cmake_minimum_required(VERSION 3.7)
project(castlevania)

include_directories(stb)

set(CMAKE_CXX_STANDARD 14)
find_package(SDL REQUIRED)

set(SOURCE_FILES
        Main.cpp
        Game.cpp
        MusicSDL.cpp
        LoadImageSDL.cpp
        LoadImage.h
        NativeBitmap.cpp
        NativeBitmap.h SDLRenderer.cpp Renderer.h )

find_package(OpenGL REQUIRED)
include (FindPkgConfig)

add_executable(castlevania ${SOURCE_FILES})
target_link_libraries(castlevania ${OPENGL_LIBRARIES} ${SDL_LIBRARY}   )