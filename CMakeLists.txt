cmake_minimum_required(VERSION 3.16)
project(emuchip8)

set(CMAKE_CXX_STANDARD 17)

set(INCLUDE_FILES include/chip8.h)
set(SRC_FILES src/chip8.cpp)

find_package(SDL2 CONFIG REQUIRED)

add_executable(emuchip8 src/main.cpp ${INCLUDE_FILES} ${SRC_FILES})
target_include_directories(emuchip8 PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(emuchip8 PRIVATE SDL2::SDL2main SDL2::SDL2-static)
