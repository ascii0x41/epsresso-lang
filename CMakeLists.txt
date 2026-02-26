cmake_minimum_required(VERSION 3.20)
project(EspressoCompiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

# NOTE: if you add/remove files, re-run cmake manually
file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/compiler/*.cpp")

add_executable(espresso main.cpp ${SOURCES})

target_include_directories(espresso PRIVATE ${PROJECT_SOURCE_DIR}/compiler)

target_compile_options(espresso PRIVATE
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Release>:-O2>
)

if(ENABLE_ASAN)
    target_compile_options(espresso PRIVATE -fsanitize=address)
    target_link_options(espresso PRIVATE -fsanitize=address)
endif()