cmake_minimum_required(VERSION 3.7)
project(castlevania)

include_directories(stb include)

set(CMAKE_CXX_STANDARD 14)
find_package(SDL REQUIRED)

set(SOURCE_FILES
        src/Main.cpp
        src/Game.cpp
        src/MusicSDL.cpp
        src/LoadImageSDL.cpp
        include/LoadImage.h
        src/NativeBitmap.cpp
        include/NativeBitmap.h src/SDLRenderer.cpp include/Renderer.h
        )

find_package(OpenGL REQUIRED)
include (FindPkgConfig)

add_executable(castlevania ${SOURCE_FILES})
target_link_libraries(castlevania ${OPENGL_LIBRARIES} ${SDL_LIBRARY}   )