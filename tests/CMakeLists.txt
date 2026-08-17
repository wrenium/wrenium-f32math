# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Tommi Tauriainen

set(WRENIUM_F32MATH_TEST_SOURCES
    test_main.cpp
    test_sincos.cpp
    test_atan2.cpp
    test_asin.cpp
    test_atanh.cpp
    test_tanh.cpp
    test_f32math_umbrella.cpp
)

add_executable(tests ${WRENIUM_F32MATH_TEST_SOURCES})

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

# Same test sources, built with WRENIUM_F32MATH_USE_STD -- every "matches
# <cmath> within kTolerance" check in the sources above holds trivially in
# this build (each function *is* the <cmath> one here), so this exists to
# catch a future regression in the macro plumbing itself (a new
# approximated function added without wiring up the switch, or a broken
# #ifdef), not to re-prove accuracy.
add_executable(tests_use_std ${WRENIUM_F32MATH_TEST_SOURCES})

target_link_libraries(tests_use_std PRIVATE Wrenium::f32math doctest::doctest wrenium_f32math_warnings)
target_include_directories(tests_use_std PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(tests_use_std PRIVATE cxx_std_17)
target_compile_definitions(tests_use_std PRIVATE WRENIUM_F32MATH_USE_STD DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS)
if(MSVC)
    target_compile_options(tests_use_std PRIVATE /EHs-c- /GR-)
else()
    target_compile_options(tests_use_std PRIVATE -fno-exceptions -fno-rtti)
endif()

set_target_properties(tests_use_std PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_test(NAME tests_use_std COMMAND tests_use_std)
