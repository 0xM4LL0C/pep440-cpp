cmake_minimum_required(VERSION 3.10)

project(pypkgm LANGUAGES CXX)

option(BUILD_TESTS "Build tests" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(cmake/CPM.cmake)

add_library(pep440 STATIC src/pep440/version.cpp)
target_include_directories(pep440 PUBLIC include)

if(BUILD_TESTS)
    CPMAddPackage("gh:catchorg/Catch2@3.8.1")
    FetchContent_GetProperties(Catch2)
    include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)

    add_executable(tests tests/version_tests.cpp)
    target_link_libraries(tests PRIVATE pep440 Catch2::Catch2WithMain)

    include(CTest)
    catch_discover_tests(tests)
endif()
