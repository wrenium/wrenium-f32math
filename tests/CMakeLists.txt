# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Tommi Tauriainen

add_executable(tests
    test_main.cpp
    test_sincos.cpp
    test_atan2.cpp
    test_asin.cpp
)

target_link_libraries(tests PRIVATE Wrenium::f32math doctest::doctest wrenium_f32math_warnings)
target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(tests PRIVATE cxx_std_17)

# Matches the MCU target: no exceptions/RTTI.
target_compile_definitions(tests PRIVATE DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS)
if(MSVC)
    target_compile_options(tests PRIVATE /EHs-c- /GR-)
else()
    target_compile_options(tests PRIVATE -fno-exceptions -fno-rtti)
endif()

set_target_properties(tests PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_test(NAME tests COMMAND tests)
