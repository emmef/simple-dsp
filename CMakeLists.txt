cmake_minimum_required(VERSION 3.6)
project(simple-dsp)

set(CMAKE_CXX_STANDARD 17)
if (MSVC)
    # warning level 4
    add_compile_options(/W4)
    # all warnings as errors
    add_compile_options(/WX)
else()
    # lots of warnings
    add_compile_options(-Wall -Wextra -pedantic)
    # all warnings as errors
    add_compile_options(-Werror)
endif()

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

include_directories(include)

set(SIMPLE_DSP_HEADERS include/simple-dsp/algorithm/size.h include/simple-dsp/algorithm/ranges.h include/simple-dsp/attributes.h)
set(SIMPLE_DSP_SOURCES)
set(SIMPLE_DSP_TESTS test/test.cc test/ranges-tests.cc test/size-tests.cc)

add_library(simple-dsp INTERFACE)
install(TARGETS simple-dsp)

add_executable(test ${SIMPLE_DSP_HEADERS} ${SIMPLE_DSP_TESTS})
target_link_libraries(test ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
