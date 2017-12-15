cmake_minimum_required(VERSION 3.7)
project(castleofviana)

include_directories(stb include
        ${SDL_INCLUDE_DIRS} ${SDL_MIXER_INCLUDE_DIRS} ${SDL_IMAGE_INCLUDE_DIRS} )



set(CMAKE_CXX_STANDARD 14)
find_package(SDL REQUIRED)
find_package(SDL_mixer REQUIRED)
find_package(SDL_image REQUIRED)

set(SOURCE_FILES
        src/Main.cpp
        src/Game.cpp
        src/MusicSDL.cpp
        src/LoadImageSDL.cpp
        include/LoadImage.h
        src/NativeBitmap.cpp
        src/CPackedFileReader.cpp
        src/IFileLoaderDelegate.cpp
        src/Common.cpp
        include/NativeBitmap.h src/SDLRenderer.cpp include/Renderer.h
        )

find_package(OpenGL REQUIRED)
include (FindPkgConfig)

add_executable(castleofviana ${SOURCE_FILES})
target_link_libraries(castleofviana ${OPENGL_LIBRARIES} ${SDL_LIBRARY}  ${SDL_MIXER_LIBRARY} ${SDL_IMAGE_LIBRARY} ${OPENGL_LIBRARIES}  )