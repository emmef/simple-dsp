cmake_minimum_required(VERSION 3.6)
project(
        "simple-dsp"
        VERSION 0.0.1
        DESCRIPTION "A header-only simple library for time-domain DSP purposes"
        HOMEPAGE_URL "https://github.com/emmef/simple-dsp")

set(CMAKE_CXX_STANDARD 17)

if (MSVC)
    # warning level 4
    add_compile_options(/W4)
    # all warnings as errors
    add_compile_options(/WX)
else ()
    # lots of warnings
    add_compile_options(-Wall -Wextra -pedantic)
    # all warnings as errors
    add_compile_options(-Werror)
endif ()

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

set(SIMPLE_DSP_HEADERS include/simple-dsp/addressing.h include/simple-dsp/attributes.h include/simple-dsp/circular.h include/simple-dsp/delay.h include/simple-dsp/alignment.h include/simple-dsp/denormal.h include/simple-dsp/arrayops.h include/simple-dsp/algorithm.h include/simple-dsp/aligneddata.h include/simple-dsp/integration.h include/simple-dsp/iir.h include/simple-dsp/samplerate.h include/simple-dsp/biquad.h include/simple-dsp/butterworth.h include/simple-dsp/crossoverplan.h include/simple-dsp/linkwitz-riley-plan.h include/simple-dsp/sample-frame.h include/simple-dsp/queue.h)

set(SIMPLE_DSP_TESTS test/test-helper.h test/test.cc test/power2-tests.cc test/ranges-tests.cc test/circular-tests.cc test/delay-tests.cc test/integration-tests.cc test/butterworth-tests.cc test/crossoverplan-tests.cc test/spinguard-tests.cc test/queue-tests.cc)

# Create the library

# Define library as an interface (header-only) library
add_library(${PROJECT_NAME} INTERFACE)

# Requires C++17 or higher
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)

# Define include directories. If another project uses the package, it will
# use the source code include directory over the installed include directory if
# the code also exists on the machine that uses the library
target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Helper for system paths
include(GNUInstallDirs)

# Define target directories
install(TARGETS  ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Make sure the library is recognisable and can be found as a package.
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)
configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)
install(EXPORT ${PROJECT_NAME}_Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)
install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

# Same for pkg-config
configure_file(pkg-config/${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

#install the header directory
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME} DESTINATION include)

# Create unit tests. This executable will not be installed.
set(TEST_PROJECT "test")
add_executable(${TEST_PROJECT} ${SIMPLE_DSP_HEADERS} ${SIMPLE_DSP_TESTS})
target_link_libraries(${TEST_PROJECT} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
include_directories(${TEST_PROJECT}  ${CMAKE_SOURCE_DIR}/include)
