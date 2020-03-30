# TODO: This find-module is an intermediate solution until we get around to
#       add CMake configuration directly in the SketchUp Ruby C Extension
#       example project.

# https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html
# https://cmake.org/cmake/help/latest/manual/cmake-developer.7.html#manual:cmake-developer(7)
# https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#imported-targets

set(SketchUpRuby_VERSION "2019.0")

find_path(_SketchUpRuby_ROOT_DIR "SketchUp Ruby C Extension Examples.sln")
set(_SketchUpRuby_THIRD_PARTY_DIR ${_SketchUpRuby_ROOT_DIR}/ThirdParty)

# Include paths and library names:

if(WIN32)
  # Ruby 2.2
  find_path(_SketchUpRuby_220_INCLUDE_ROOT_DIR "ruby.h"
    HINTS
      ${_SketchUpRuby_THIRD_PARTY_DIR}/include/ruby/2.2/win32_x64
    NO_DEFAULT_PATH # Needed to ensure system Ruby isn't picked up
  )
  find_path(_SketchUpRuby_220_INCLUDE_SUPPORT_DIR "ruby/config.h"
    HINTS
      ${_SketchUpRuby_THIRD_PARTY_DIR}/include/ruby/2.2/win32_x64/x64-mswin64_140
    NO_DEFAULT_PATH # Needed to ensure system Ruby isn't picked up
  )
  set(_SketchUpRuby_220_INCLUDE_DIR
    ${_SketchUpRuby_220_INCLUDE_ROOT_DIR}
    ${_SketchUpRuby_220_INCLUDE_SUPPORT_DIR}
  )
  set(_SketchUpRuby_220_LIB_NAME x64-msvcrt-ruby220.lib)

  # Ruby 2.5
  find_path(_SketchUpRuby_250_INCLUDE_DIR "ruby.h"
    HINTS
      ${_SketchUpRuby_THIRD_PARTY_DIR}/include/ruby/2.5/win32_x64
    NO_DEFAULT_PATH # Needed to ensure system Ruby isn't picked up
  )
  set(_SketchUpRuby_250_LIB_NAME x64-msvcrt-ruby250.lib)
elseif(APPLE)
  # Ruby 2.2
  find_path(_SketchUpRuby_220_INCLUDE_ROOT_DIR "ruby.h"
    HINTS
      ${_SketchUpRuby_THIRD_PARTY_DIR}/include/ruby/2.2/mac
    NO_DEFAULT_PATH # Needed to ensure system Ruby isn't picked up
  )
  find_path(_SketchUpRuby_220_INCLUDE_SUPPORT_DIR "ruby/config.h"
    HINTS
      ${_SketchUpRuby_THIRD_PARTY_DIR}/include/ruby/2.2/mac/x86_64-darwin14
    NO_DEFAULT_PATH # Needed to ensure system Ruby isn't picked up
  )
  set(_SketchUpRuby_220_INCLUDE_DIR
    ${_SketchUpRuby_220_INCLUDE_ROOT_DIR}
    ${_SketchUpRuby_220_INCLUDE_SUPPORT_DIR}
  )
  set(_SketchUpRuby_220_LIB_NAME Ruby)

  # Ruby 2.5
  find_path(_SketchUpRuby_250_INCLUDE_ROOT_DIR "ruby.h"
    HINTS
      ${_SketchUpRuby_THIRD_PARTY_DIR}/include/ruby/2.5/mac
    NO_DEFAULT_PATH # Needed to ensure system Ruby isn't picked up
  )
  find_path(_SketchUpRuby_250_INCLUDE_SUPPORT_DIR "ruby/config.h"
    HINTS
      ${_SketchUpRuby_THIRD_PARTY_DIR}/include/ruby/2.5/mac/x86_64-darwin17
    NO_DEFAULT_PATH # Needed to ensure system Ruby isn't picked up
  )
  set(_SketchUpRuby_250_INCLUDE_DIR
    ${_SketchUpRuby_250_INCLUDE_ROOT_DIR}
    ${_SketchUpRuby_250_INCLUDE_SUPPORT_DIR}
  )
  set(_SketchUpRuby_250_LIB_NAME Ruby)
endif()

# Libraries:

# https://cmake.org/cmake/help/latest/prop_tgt/IMPORTED_LOCATION.html
# """
# For frameworks on macOS this is the location of the library file symlink just
# inside the framework folder. For DLLs this is the location of the .dll part of
# the library. For UNKNOWN libraries this is the location of the file to b
# linked.
# """
#
# Not sure why `SketchUpRuby_220_LIBRARY` can be used directly with
# `target_link_libraries`, but with `IMPORTED_LOCATION` it needs to point to
# the symlink inside.

# Ruby 2.2
find_library(SketchUpRuby_220_LIBRARY ${_SketchUpRuby_220_LIB_NAME}
  HINTS
    ${_SketchUpRuby_THIRD_PARTY_DIR}/lib/mac/2.2
    ${_SketchUpRuby_THIRD_PARTY_DIR}/lib/win32
  NO_DEFAULT_PATH # Needed to ensure system Ruby isn't picked up
)
set(SketchUpRuby_220_INCLUDE_DIR ${_SketchUpRuby_220_INCLUDE_DIR})
if(APPLE)
  set(_SketchUpRuby_220_IMPORTED_LIBRARY ${SketchUpRuby_220_LIBRARY}/Ruby)
else()
  set(_SketchUpRuby_220_IMPORTED_LIBRARY ${SketchUpRuby_220_LIBRARY})
endif()

# Ruby 2.5
find_library(SketchUpRuby_250_LIBRARY ${_SketchUpRuby_250_LIB_NAME}
  HINTS
    ${_SketchUpRuby_THIRD_PARTY_DIR}/lib/mac/2.5
    ${_SketchUpRuby_THIRD_PARTY_DIR}/lib/win32
  NO_DEFAULT_PATH # Needed to ensure system Ruby isn't picked up
)
set(SketchUpRuby_250_INCLUDE_DIR ${_SketchUpRuby_250_INCLUDE_DIR})
if(APPLE)
  set(_SketchUpRuby_250_IMPORTED_LIBRARY ${SketchUpRuby_250_LIBRARY}/Ruby)
else()
  set(_SketchUpRuby_250_IMPORTED_LIBRARY ${SketchUpRuby_250_LIBRARY})
endif()


message(DEBUG "SketchUpRuby_220_LIBRARY: ${SketchUpRuby_220_LIBRARY}")
message(DEBUG "SketchUpRuby_220_INCLUDE_DIR: ${SketchUpRuby_220_INCLUDE_DIR}")

message(DEBUG "SketchUpRuby_250_LIBRARY: ${SketchUpRuby_250_LIBRARY}")
message(DEBUG "SketchUpRuby_250_INCLUDE_DIR: ${SketchUpRuby_250_INCLUDE_DIR}")


mark_as_advanced(
  # Ruby 2.2
  SketchUpRuby_220_LIBRARY
  SketchUpRuby_220_INCLUDE_DIR
  # Ruby 2.5
  SketchUpRuby_250_LIBRARY
  SketchUpRuby_250_INCLUDE_DIR
)


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SketchUpRuby
  REQUIRED_VARS
    SketchUpRuby_220_LIBRARY SketchUpRuby_220_INCLUDE_DIR
    SketchUpRuby_250_LIBRARY SketchUpRuby_250_INCLUDE_DIR
  VERSION_VAR
    SketchUpRuby_VERSION)

# https://cmake.org/cmake/help/latest/manual/cmake-developer.7.html#a-sample-find-module
if(SketchUpRuby_FOUND AND NOT TARGET SketchUp::SketchUpRuby)
  # Ruby 2.2
  add_library(SketchUp::SketchUpRuby_220 UNKNOWN IMPORTED)
  set_target_properties(SketchUp::SketchUpRuby_220 PROPERTIES
    IMPORTED_LOCATION "${_SketchUpRuby_250_IMPORTED_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SketchUpRuby_220_INCLUDE_DIR}"
  )
  # Ruby 2.5
  add_library(SketchUp::SketchUpRuby_250 UNKNOWN IMPORTED)
  set_target_properties(SketchUp::SketchUpRuby_250 PROPERTIES
    IMPORTED_LOCATION "${_SketchUpRuby_250_IMPORTED_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SketchUpRuby_250_INCLUDE_DIR}"
  )
endif()
