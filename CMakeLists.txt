# CMake 3.15.4 contains updates to the default configuration of Windows projects
# that makes it easier to manage warning levels and run-time libraries.
# https://gitlab.kitware.com/cmake/cmake/issues/19084
cmake_minimum_required(VERSION 3.15.4)

# set(CMAKE_DEBUG_TARGET_PROPERTIES INCLUDE_DIRECTORIES)

# SketchUp 2020.0 only supports macOS 10.13 and newer.
# https://stackoverflow.com/a/34208904/486990
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")

# https://crascit.com/2015/03/28/enabling-cxx11-in-cmake/
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# https://gitlab.com/CLIUtils/modern-cmake/blob/master/examples/extended-project/apps/CMakeLists.txt
# target_compile_features(app PRIVATE cxx_std_17)

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
project(TraceUp VERSION 0.1.0 LANGUAGES CXX C)
configure_file(traceup_config.hpp.in traceup_config.hpp)

# Specify the list of directories to search for cmake modules.
# https://cliutils.gitlab.io/modern-cmake/chapters/basics/structure.html
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Hide symbols by default.
# https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

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

find_package(SketchUpRuby 2019.0 REQUIRED)
# TODO: Can we import selected targets from Find Modules?
# https://cmake.org/cmake/help/latest/manual/cmake-developer.7.html#find-modules
# find_package(SketchUpRuby 2019.0 REQUIRED
#   COMPONENTS
#     Ruby220 Ruby250
# )

# https://github.com/google/googletest/blob/master/googletest/README.md
# https://crascit.com/2015/07/25/cmake-gtest/
# Ensure we build static CRT.
set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)

# Don't want to install GoogleTest.
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

add_subdirectory(traceup)
add_subdirectory(third-party/googletest)
add_subdirectory(tests)

# CMake does not recommend using GLOB to collect a list of source files from
# the source tree.
# https://cmake.org/cmake/help/latest/command/file.html#filesystem
# file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/**/*.cpp")
# file(GLOB_RECURSE  SOURCES "*.cpp")
set(SOURCES
  ruby/traceup_extension.cpp
  ruby/utilities.cpp
)

set(TRACEUP traceup)

set(TRACEUP_RUBY_VERSIONS "2.2" "2.5")
foreach(RUBY_VERSION ${TRACEUP_RUBY_VERSIONS})
  # Generate a "250" style string from "2.5" style string:
  # TODO: Offer utility functions to generate the variables.
  string(REPLACE "." "" RUBY_VERSION_ID ${RUBY_VERSION})
  set(RUBY_VERSION_ID "${RUBY_VERSION_ID}0")

  # Ensure a lib with unique name for each Ruby version:
  set(TRACEUP_LIB ${TRACEUP}_${RUBY_VERSION_ID})

  # Resolve the target name this Ruby version:
  set(SketchUpRuby_LIBRARY "SketchUp::SketchUpRuby_${RUBY_VERSION_ID}")

  if(APPLE)
    # For Mac, the library needs to be of Mach-O type: Bundle
    add_library(${TRACEUP_LIB} MODULE ${SOURCES})
  else()
    add_library(${TRACEUP_LIB} SHARED ${SOURCES})
  endif()

  target_link_libraries(${TRACEUP_LIB} PRIVATE traceup_static)

  # Need to correct the names since the name of each library for each Ruby
  # version is exactly the same. By default CMake uses the CMake target name
  # for the file names.
  set_target_properties(${TRACEUP_LIB} PROPERTIES
    OUTPUT_NAME ${TRACEUP}
    DEFINE_SYMBOL ${TRACEUP}_EXPORTS
  )
  set_target_properties(${TRACEUP_LIB} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${RUBY_VERSION}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${RUBY_VERSION}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${RUBY_VERSION}"
  )

  # Generate export header:
  # https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html
  include("GenerateExportHeader")
  # TODO: Set visibility?
  generate_export_header(${TRACEUP_LIB}
    BASE_NAME ${TRACEUP}
    # EXPORT_MACRO_NAME ${TRACEUP}_EXPORTS
  )

  # Include path for CMake auto-generated files:
  target_include_directories(${TRACEUP_LIB} PUBLIC ${CMAKE_BINARY_DIR})

  # The projects include paths:
  target_include_directories(${TRACEUP_LIB} PUBLIC ${PROJECT_SOURCE_DIR})

  # Third party libraries:
  target_link_libraries(${TRACEUP_LIB} PUBLIC ${SketchUpRuby_LIBRARY})

  # https://arne-mertz.de/2018/07/cmake-properties-options/
  if(MSVC)
    # Elevate warning level.
    # Ignore unknown pragmas (mac).
    target_compile_options(${TRACEUP_LIB} PRIVATE
      /W4 /WX
      /wd4068
    )

    # In SketchUp we found that when we upgraded from v140 toolkit, both v141
    # and v142 would yield random runtime crashes when `rb_raise` was called.
    # Not all locations of `rb_raise` would cause this, only some. We were not
    # able to determine a predictable pattern for this.
    # This would only occur with optimisation enabled. It appear the compiler
    # would in some cases get confused by that Ruby long-jumps when it's throws
    # errors. We observed that objects that in source code was declared later
    # than the triggering `rb_raise` would unexpectantly be attempted
    # deallocated. Investigations indicated that enabling /EHc would prevent
    # this from happening as it instructs the compiler that C functions might
    # throw. Ruby doesn't really throw, but the long jump behaved similary.
    #
    # https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model?view=vs-2019
    #
    # > The exception-handling model that catches synchronous (C++) exceptions
    # > only and tells the compiler to assume that functions declared as
    # > extern "C" may throw an exception.
    #
    # Related, long jump behaviour in C++ standard doesn't guarranty to support
    # C++ object semantics. Microsofts mention this in their documentation and
    # stresses that it's not portable. They also mention the Microsoft spesific
    # way long jump are handled where they explain that the stack will be
    # unwound similar to try/catch.
    #
    # >In Microsoft C++ code on Windows, longjmp uses the same stack-unwinding
    # > semantics as exception-handling code. It is safe to use in the same
    # > places that C++ exceptions can be raised. However, this usage is not
    # > portable
    #
    # https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/longjmp?view=vs-2019
    #
    # Further the documentation describes:
    #
    # > If you decide to use setjmp and longjmp in a C++ program, also include
    # > <setjmp.h> or <setjmpex.h> to assure correct interaction between the
    # > functions and Structured Exception Handling (SEH) or C++ exception
    # > handling.
    #
    # https://docs.microsoft.com/en-us/cpp/cpp/using-setjmp-longjmp?view=vs-2019
    target_compile_options(${TRACEUP_LIB} PRIVATE /EHs)

    # Ruby require the library to have .so extension on Windows.
    # https://cmake.org/cmake/help/v3.0/command/set_target_properties.html
    # https://cmake.org/cmake/help/latest/command/set_target_properties.html
    set_target_properties(${TRACEUP_LIB} PROPERTIES SUFFIX ".so" )

    # On Windows we must link to sketchup.lib to use the Live C API.
    # target_link_libraries(${TRACEUP_LIB} ${SketchUpAPI_LIBRARY})
  else()
    # Elevate warning level.
    # Ignore unknown pragmas (win) and Ruby's usage of `register`.
    target_compile_options(${TRACEUP_LIB} PRIVATE
      -Wall -Wextra -pedantic -Werror
      -Wno-unknown-pragmas -Wno-register
    )

    # Ruby require the library to have .bundle extension on OSX.
    set_target_properties(${TRACEUP_LIB} PROPERTIES SUFFIX ".bundle" )
    # Remove the "lib" prefix as the name must match the Init_* function.
    set_target_properties(${TRACEUP_LIB} PROPERTIES PREFIX "" )
  endif()


  set(EXTENSION_DIR ${PROJECT_SOURCE_DIR}/../src)
  set(EXTENSION_STAGE_DIR ${EXTENSION_DIR}/tt_traceup/cext/stage)
  # RUNTIME DESTINATION is needed for Windows (SHARED).
  # RUNTIME DESTINATION is needed for Mac (MODULE).
  install(TARGETS ${TRACEUP_LIB}
    RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/${RUBY_VERSION}
    LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/${RUBY_VERSION}
  )
  install(TARGETS ${TRACEUP_LIB}
    RUNTIME DESTINATION ${EXTENSION_STAGE_DIR}/${RUBY_VERSION}
    LIBRARY DESTINATION ${EXTENSION_STAGE_DIR}/${RUBY_VERSION}
  )
  # TODO: Clean extension cext directory for anything except the stage directory.
  # https://cmake.org/cmake/help/latest/command/install.html#custom-installation-logic

endforeach() # TRACEUP_RUBY_VERSIONS


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
