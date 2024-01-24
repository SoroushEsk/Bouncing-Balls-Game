cmake_minimum_required(VERSION 3.26)
project(Bouncing_Balls)


set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules)
set(SDL2_gfx_DIR "C:\\LI\\MinGW\\include\\SDL2")
set(SDL2_PATH "C:\\LI")

find_package(SDL2 REQUIRED)

# Include directories for SDL2 and SDL2_gfx
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_gfx_DIR})



set(CMAKE_CXX_STANDARD 17)
add_executable(Bouncing_Balls main.cpp)

target_link_libraries(Bouncing_Balls ${SDL2_LIBRARY} -lSDL2_image -lSDL2 -lSDL2_ttf -lSDL2_mixer )

add_custom_command(TARGET Bouncing_Balls POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${SDL2_gfx_DIR}/SDL2_gfx.dll"
        "$<TARGET_FILE_DIR:Bouncing_Balls>"
        COMMENT "Copying SDL2_gfx.dll to the build directory"
)