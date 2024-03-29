cmake_minimum_required(VERSION 3.7)
project(castleofviana)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_find")

include_directories(
		${SDL2_INCLUDE_DIR}
)

include_directories(stb include
		${SDL_INCLUDE_DIRS} ${SDL_MIXER_INCLUDE_DIRS} ${SDL_IMAGE_INCLUDE_DIRS})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -W -pedantic")

set(CMAKE_CXX_STANDARD 98)
find_package(SDL2 REQUIRED)

set(SOURCE_FILES
		src/Main.cpp
		src/Game.cpp
		src/NullMusic.cpp
		src/LoadBitmap.cpp
		src/CPackedFileReader.cpp
		src/Common.cpp
		src/SDL2Renderer.cpp
		)

add_executable(castleofviana ${SOURCE_FILES})

include(FindPkgConfig)

target_include_directories(castleofviana PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(castleofviana LINK_PUBLIC ${SDL2_LIBRARY} m)
