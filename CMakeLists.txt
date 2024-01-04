cmake_minimum_required(VERSION 3.26)
project(Bouncing_Balls)


set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules)

set(SDL2_PATH "C:\\LI")

find_package(SDL2 REQUIRED)
include_directories(${SDL_INCLUDE_DIR})
set(CMAKE_CXX_STANDARD 17)
add_executable(Bouncing_Balls main.cpp)

target_link_libraries(Bouncing_Balls ${SDL2_LIBRARY} -lSDL2_image )