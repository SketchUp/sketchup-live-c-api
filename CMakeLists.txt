# Look for TODO(Example) for where you need to provide your own input for your
# own project if you use this example as a template.

# CMake 3.15.4 contains updates to the default configuration of Windows projects
# that makes it easier to manage warning levels and run-time libraries.
# https://gitlab.kitware.com/cmake/cmake/issues/19084
cmake_minimum_required(VERSION 3.15.4)

# SketchUp 2019.0 supports macOS 10.12 and newer.
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")

# https://crascit.com/2015/03/28/enabling-cxx11-in-cmake/
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Since SketchUp extensions are created to run against multiple SketchUp
# versions CRT dependency can be a pain if using dynamic linking.
# If the extension was build against the same CRT as SketchUp then it would be
# fine. But if the versions are different and the user doesn't have the
# extension's CRT installed the Ruby C Extension will fail to load.
# Different SketchUp versions using the same Ruby version might also have been
# built with different CRT, making it difficult to rely on SketchUp's CRT.
# To simplify this, build the Ruby C Extension with static CRT.
# Set here to apply as default for all targets.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Validate platform.
if(NOT (WIN32 OR APPLE))
  message(FATAL_ERROR "This project only works on Windows or macOS.")
endif()

# https://cmake.org/cmake/help/latest/guide/tutorial/index.html
# TODO(Example): Name your project:
project(Example VERSION 0.1.0 LANGUAGES CXX C)

# Specify the list of directories to search for cmake modules.
# https://cliutils.gitlab.io/modern-cmake/chapters/basics/structure.html
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Hide symbols by default.
# https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

# Set some Windows only properties.
if(WIN32)
  # Use the Unicode version of the Windows API.
  add_definitions(-DUNICODE -D_UNICODE)
  # Avoid global pollution from Windows API.
  add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
endif()

# https://stackoverflow.com/a/50469013/486990
# https://gitlab.kitware.com/cmake/community/wikis/doc/ctest/Testing-With-CTest
include(CTest)
enable_testing()

# To tell this project where to find the SketchUp C API either:
# 1. `cmake -D "SketchUpAPI_SEARCH_DIR=<path_to/slapi>"
# 2. Set an "SketchUpAPI_SEARCH_DIR" environment variable
# 3. Extract the SDK zip files into the /third-party/slapi directory of this
#    project.
#
# In all these cases it's assumed that the directory contains (possibly)
# multiple versions of the C API, each in their own directory matching the
# original .zip files distrobuted.
#
# For example, the SketchUp 2020.0 SDK distrobuted the following files:
# * SDK_WIN_x64_2020-0-363.zip
# * SDK_Mac_2020-0-362.zip
#
# Extract these such that you have:
# * <path_to/slapi>/SDK_WIN_x64_2020-0-363/
# * <path_to/slapi>/SDK_Mac_2020-0-362/
#
# If you have multiple versions for Windows it might look like this:
# * <path_to/slapi>/SDK_WIN_x64_2020-0-363/
# * <path_to/slapi>/SDK_WIN_x64_2019-3-253/
# * <path_to/slapi>/SDK_WIN_x64_2019-2-222/
#
# The CMake module in this project will look for these spesific names and
# map them to their appropriate SketchUp versions when looking for the C API.
message(DEBUG "SketchUpAPI_SEARCH_DIR: ${SketchUpAPI_SEARCH_DIR}")
message(DEBUG "ENV SketchUpAPI_SEARCH_DIR: $ENV{SketchUpAPI_SEARCH_DIR}")
if(DEFINED SketchUpAPI_SEARCH_DIR OR DEFINED ENV{SketchUpAPI_SEARCH_DIR})
  message(DEBUG "Using pre-defined SLAPI path: ${SketchUpAPI_SEARCH_DIR}$ENV{SketchUpAPI_SEARCH_DIR}")
else()
  set(SketchUpAPI_SEARCH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/slapi)
  message(DEBUG "Using project-defined SLAPI path: ${SketchUpAPI_SEARCH_DIR} $ENV{SketchUpAPI_SEARCH_DIR}")
endif()

# Enable CMake to find the Ruby frameworks bundles in this project as a
# submodule.
list(PREPEND CMAKE_PREFIX_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/third-party/ruby-c-extension-examples
)

# Ensure we build static CRT for GoogleTest to match the rest of the project.
# https://github.com/google/googletest/blob/master/googletest/README.md
# https://crascit.com/2015/07/25/cmake-gtest/
set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)

# Don't want to install GoogleTest.
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

# TODO: Move the utility functions to their own files.

# Utility function to set base warning properties.
function(set_default_warnings TARGET_NAME)
  # https://arne-mertz.de/2018/07/cmake-properties-options/
  if(MSVC)
    # Elevate warning level.
    # Ignore unknown pragmas.
    target_compile_options(${TARGET_NAME} PRIVATE
      /W4 /WX
      /wd4068
    )
  else()
    # Elevate warning level.
    # Ignore unknown pragmas.
    target_compile_options(${TARGET_NAME} PRIVATE
      -Wall -Wextra -pedantic -Werror
      -Wno-unknown-pragmas
    )
  endif()
endfunction()

# Utility function to validate dependencies.
function(validate_dependencies TARGET_NAME BINARY_FILE EXPECTED UNEXPECTED)
  # TODO: Make EXPECTED and UNEXPECT named arguments.
  set(SCRIPTS_PATH "${PROJECT_SOURCE_DIR}/cext/scripts")
  if(MSVC)
    find_program(DUMPBIN "dumpbin")
    message(DEBUG dumpbin: ${DUMPBIN})
    add_custom_command(TARGET ${TARGET_NAME}
      POST_BUILD
      COMMAND ${SCRIPTS_PATH}/depcheck.bat
        ${DUMPBIN} ${BINARY_FILE} ${EXPECTED} ${UNEXPECTED}
      COMMENT "Validate dependencies of ${BINARY_FILE}>" VERBATIM
    )
  endif()
endfunction()

# Find the SketchUp C API SDK package:
find_package(SketchUpAPI 2019.3 REQUIRED)

# TODO: Restructure folders:
# * cext/image_lib
# * cext/ruby_extension
# * cext/tests
#
# add_subdirectory(third-party/google-test)
# add_subdirectory(cext/image_lib)
# add_subdirectory(cext/ruby_extension)
# add_subdirectory(cext/tests)
add_subdirectory(third-party/google-test)
add_subdirectory(cext)
