cmake_minimum_required(VERSION 3.8)
project(SDLKeyDisplay)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add SDL and SDL_ttf
find_package(SDL REQUIRED)
find_package(SDL_ttf REQUIRED)

include_directories(${SDL_INCLUDE_DIR} ${SDL_TTF_INCLUDE_DIR})

option(FLIP "Flip the display content 180 degrees" OFF)
option(PORTRAIT "Portrait orientation mode" OFF)

if(FLIP)
    add_definitions(-DFLIP)
endif()

if(PORTRAIT)
    add_definitions(-DPORTRAIT)
endif()

add_executable(SDLKeyDisplay main.cpp)
target_link_libraries(SDLKeyDisplay ${SDL_LIBRARY} ${SDL_TTF_LIBRARY})

