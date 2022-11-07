cmake_minimum_required(VERSION 3.1) # Threads package

project(jack-asynchronize
    VERSION 0.0.0
    LANGUAGES CXX
    DESCRIPTION "Friendly synchronization tools for modern C++"
    HOMEPAGE_URL https://github.com/jallen-cse/asynchronize)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)          # will want concepts
endif()

option(JACK_ASYNCHRONIZE_BUILD_EXAMPLES "Build example executables" OFF)

find_package(Threads REQUIRED)  # cross platform for std::thread

add_library(asynchronize INTERFACE)
target_include_directories(asynchronize INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(asynchronize INTERFACE Threads::Threads)

if (JACK_ASYNCHRONIZE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
