# Support CMake version shipping with Ubuntu 18.04 LTS
cmake_minimum_required(VERSION 3.10)

# By default use in-repo vcpkg for package searches
set(CMAKE_TOOLCHAIN_FILE "lib/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# Must come after toolchain file stuff
project(helgoboss-learn VERSION 0.1.0 LANGUAGES CXX)

# Main target
# Header-only library (= interface library) GSL doesn't offer find_package(), so we need to find its include directory
# via find_path()
find_path(GSL_INCLUDE_DIR gsl/gsl)
set(WDL_EEL2 ON CACHE INTERNAL "")
add_subdirectory(lib/WDL/WDL)
add_library(helgoboss-learn STATIC
    src/math-util.cpp
    src/MidiClockTransportMessageType.cpp
    src/Mode.cpp
    src/ModeProcessor.cpp
    src/ModeType.cpp
    src/Source.cpp
    src/SourceCharacter.cpp
    src/SourceType.cpp
    src/SourceValue.cpp
    src/source-util.cpp
    src/string-util.cpp
    src/Tempo.cpp
    )
find_package(helgoboss-midi 0.1.0 CONFIG REQUIRED)
find_package(rxcpp CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
    helgoboss-midi::helgoboss-midi
    wdl-eel2
    rxcpp
    )
# Use generator syntax for INTERFACE-scoped includes to support usage of installed library (typical "Modern CMake"
# pattern, see https://pabloariasal.github.io/2018/02/19/its-time-to-do-cmake-right/)
target_include_directories(helgoboss-learn
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/wdl-ol/WDL>
    $<INSTALL_INTERFACE:lib/wdl-ol/WDL>
    PRIVATE
    src
    ${GSL_INCLUDE_DIR}
    )
# We want GSL Expects() to throw instead of aborting in order to enable clients to handle exceptions gracefully
target_compile_definitions(helgoboss-learn PRIVATE GSL_THROW_ON_CONTRACT_VIOLATION)
# Disable those terrible min max macros in windows.h
target_compile_definitions(helgoboss-learn PRIVATE NOMINMAX)
# We want strict C++-17 (as PUBLIC because we use C++-17 nested namespaces in public headers)
target_compile_features(helgoboss-learn PUBLIC cxx_std_17)
set_target_properties(helgoboss-learn PROPERTIES CXX_EXTENSIONS OFF)
add_library(helgoboss-learn::helgoboss-learn ALIAS helgoboss-learn)

# Install
#include(CMakePackageConfigHelpers)
#include(GNUInstallDirs)
#write_basic_package_version_file(helgoboss-learn-config-version.cmake COMPATIBILITY SameMajorVersion)
#install(DIRECTORY include/helgoboss-learn DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
#install(TARGETS helgoboss-learn
#    EXPORT helgoboss-learn-targets
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    )
#install(EXPORT helgoboss-learn-targets
#    NAMESPACE helgoboss-learn::
#    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/helgoboss-learn
#    )
#install(FILES
#    cmake/helgoboss-learn-config.cmake
#    ${CMAKE_CURRENT_BINARY_DIR}/helgoboss-learn-config-version.cmake
#    DESTINATION
#    ${CMAKE_INSTALL_LIBDIR}/cmake/helgoboss-learn
#    )

# Tests
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
  if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
  endif ()
endif ()
