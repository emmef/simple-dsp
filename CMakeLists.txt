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
find_package(Threads)

set(SIMPLE_DSP_HEADERS include/simple-dsp/core/index.h include/simple-dsp/core/attributes.h include/simple-dsp/core/circular.h include/simple-dsp/core/alignment.h include/simple-dsp/core/denormal.h include/simple-dsp/util/arrayops.h include/simple-dsp/core/bounds.h include/simple-dsp/util/aligneddata.h include/simple-dsp/integration.h include/simple-dsp/iir.h include/simple-dsp/samplerate.h include/simple-dsp/biquad.h include/simple-dsp/butterworth.h include/simple-dsp/crossoverplan.h include/simple-dsp/linkwitz-riley-plan.h include/simple-dsp/sample-frame.h include/simple-dsp/util/queue.h include/simple-dsp/processnode.h include/simple-dsp/util/lock-fee-owner.hpp include/simple-dsp/util/timeout.h include/simple-dsp/core/size.h include/simple-dsp/core/bits.h include/simple-dsp/core/power2.h include/simple-dsp/core/sse.h include/simple-dsp/util/frame.h)
set(SIMPLE_DSP_SRC src/simple-dsp/placeholder.cc)
set(SIMPLE_DSP_TESTS test/test-helper.h test/test.cc test/power2-tests.cc test/ranges-tests.cc test/circular-tests.cc test/integration-tests.cc test/butterworth-tests.cc test/crossoverplan-tests.cc test/guards-tests.cc test/queue-tests.cc test/lock-free-owner-tests.cc test/samplerate-tests.cc test/frame-tests.cc)

# Create the library

# Define library library
add_library(${PROJECT_NAME} SHARED ${SIMPLE_DSP_SRC} ${SIMPLE_DSP_HEADERS})

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
install(TARGETS ${PROJECT_NAME}
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
add_executable(${TEST_PROJECT} ${SIMPLE_DSP_TESTS})
target_include_directories(${TEST_PROJECT} BEFORE PRIVATE ${SIMPLE_DSP_HEADERS})
target_link_libraries(${TEST_PROJECT} ${PROJECT_NAME} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
include_directories(${TEST_PROJECT} ${CMAKE_SOURCE_DIR}/include)
