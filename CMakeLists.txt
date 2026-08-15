# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Tommi Tauriainen

cmake_minimum_required(VERSION 3.21)

project(wrenium-f32math
    VERSION 0.3.1
    DESCRIPTION "Header-only, float-only math approximations for single-precision-FPU embedded targets"
    LANGUAGES CXX
)

add_library(wrenium-f32math INTERFACE)
add_library(Wrenium::f32math ALIAS wrenium-f32math)

target_include_directories(wrenium-f32math INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(wrenium-f32math INTERFACE cxx_std_17)

add_library(wrenium_f32math_warnings INTERFACE)
if(MSVC)
    target_compile_options(wrenium_f32math_warnings INTERFACE /W4)
else()
    target_compile_options(wrenium_f32math_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

if(PROJECT_IS_TOP_LEVEL)
    include(CTest)
endif()

if(PROJECT_IS_TOP_LEVEL AND BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.5.3
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(doctest)

    add_subdirectory(tests)
endif()
