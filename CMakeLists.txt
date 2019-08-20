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

string(LENGTH "${CMAKE_SOURCE_DIR}/" SDSP_SOURCE_PATH_SIZE)
add_definitions("-DSDSP_SOURCE_PATH_SIZE=${SDSP_SOURCE_PATH_SIZE}")

set(SIMPLE_DSP_HEADERS include/simple-dsp/algorithm/size.h include/simple-dsp/algorithm/ranges.h include/simple-dsp/attributes.h include/simple-dsp/algorithm/circular.h include/simple-dsp/delay.h )

set(SIMPLE_DSP_SOURCES)


set(SIMPLE_DSP_TESTS test/test-helper.h test/test.cc test/power2-tests.cc test/ranges-tests.cc test/size-tests.cc  test/circular-tests.cc test/delay-tests.cc)

add_library(simple-dsp INTERFACE)
install(TARGETS simple-dsp)

add_executable(test ${SIMPLE_DSP_HEADERS} ${SIMPLE_DSP_TESTS} ${SIMPLE_DSP_SOURCES})
target_link_libraries(test ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
