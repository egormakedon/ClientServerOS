cmake_minimum_required(VERSION 3.8)
project(Server)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c)
add_executable(Server ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} pthread)