cmake_minimum_required(VERSION 3.20)
project(EspressoCompiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${PROJECT_SOURCE_DIR}/compiler)

file(GLOB_RECURSE SOURCES
    "${PROJECT_SOURCE_DIR}/compiler/*.cpp"
)

add_executable(espresso main.cpp ${SOURCES})

# Debug + AddressSanitizer
target_compile_options(espresso PRIVATE -g -fsanitize=address)
target_link_options(espresso PRIVATE -fsanitize=address)
