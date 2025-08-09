cmake_minimum_required(VERSION 3.15)

project(nanodiff VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} nanodiff.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Werror=pedantic
    -pedantic-errors)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -fno-omit-frame-pointer
        -fsanitize=address,undefined)
    target_link_options(${PROJECT_NAME} PRIVATE
        -fsanitize=address,undefined)
endif ()
