# TODO: Rename to LiveSketchUpAPI ?

# https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html
# https://cmake.org/cmake/help/latest/manual/cmake-developer.7.html#manual:cmake-developer(7)

# TODO: Imported Target
# https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#imported-targets

# ${SketchUpAPI_DIR}
# ${SketchUpAPI_SEARCH_DIR}
# $ENV{SketchUpAPI_DIR}
# $ENV{SketchUpAPI_SEARCH_DIR}

set(_SketchUpAPI_2019_3
  2019.3
  SDK_WIN_x64_2019-3-253
  SDK_Mac_2019-3-252
)
set(_SketchUpAPI_2019_2
  2019.2
  SDK_WIN_x64_2019-2-222
  SDK_Mac_2019-2-221
)

set(_SketchUpAPIs
  ${_SketchUpAPI_2019_3}
  ${_SketchUpAPI_2019_2}
)

set(_num_versions 2)
set(_version_size 3)

LIST(LENGTH _SketchUpAPIs _length)
MATH(EXPR _range_max "(${_version_size}*${_num_versions})-1")

foreach(_i RANGE 0 ${_range_max} ${_version_size})
  list(SUBLIST _SketchUpAPIs ${_i} ${_version_size} _SketchUpAPI)
  message(DEBUG "i: ${_i}, length: ${_length}, range_max: ${_range_max}")
  message(DEBUG ${_SketchUpAPI})

  list(GET _SketchUpAPI 0 _SketchUpAPI_VERSION)
  list(GET _SketchUpAPI 1 _SketchUpAPI_WIN_DIR)
  list(GET _SketchUpAPI 2 _SketchUpAPI_MAC_DIR)

  if(WIN32)
    set(_SketchUpAPI_BASENAME_DIR ${_SketchUpAPI_WIN_DIR})
    elseif(APPLE)
    set(_SketchUpAPI_BASENAME_DIR ${_SketchUpAPI_MAC_DIR})
  else()
    message(FATAL_ERROR "SketchUpAPI is only available on Win/Mac")
  endif()

  # TODO: Rename to SketchUpAPI_SEARCH_PATH?
  message(DEBUG "SketchUpAPI_SEARCH_DIR: ${SketchUpAPI_SEARCH_DIR}")
  message(DEBUG "ENV SketchUpAPI_SEARCH_DIR: $ENV{SketchUpAPI_SEARCH_DIR}")
  message(DEBUG "_SketchUpAPI_BASENAME_DIR: ${_SketchUpAPI_BASENAME_DIR}")

  find_file(_SketchUpAPI_DIR
    NAMES
      ${_SketchUpAPI_BASENAME_DIR}
    HINTS
      ${SketchUpAPI_SEARCH_DIR}
      $ENV{SketchUpAPI_SEARCH_DIR}
  )
  message(DEBUG "_SketchUpAPI_DIR: ${_SketchUpAPI_DIR}")

  if(NOT IS_DIRECTORY ${_SketchUpAPI_DIR})
    continue()
  endif()

  if(SketchUpAPI_FIND_VERSION_EXACT)
    set(_SketchUpAPI_VERSION_COMPARE "VERSION_EQUAL")
  else()
    set(_SketchUpAPI_VERSION_COMPARE "VERSION_GREATER_EQUAL")
  endif()

  if(SketchUpAPI_FIND_VERSION_EXACT)
    if(NOT PACKAGE_FIND_VERSION VERSION_EQUAL ${_SketchUpAPI_VERSION})
      continue()
    endif()
  else()
    if(NOT PACKAGE_FIND_VERSION VERSION_GREATER_EQUAL ${_SketchUpAPI_VERSION})
      continue()
    endif()
  endif()

  set(SketchUpAPI_DIR ${_SketchUpAPI_DIR})
  set(SketchUpAPI_VERSION ${_SketchUpAPI_VERSION})
  break()

endforeach()

if(WIN32)
  set(_SketchUpAPI_LIBRARY_NAME "sketchup") # Live C API
  set(_SketchUpAPI_LIBRARY_DIR ${_SketchUpAPI_DIR}/binaries/sketchup/x64)
  set(_SketchUpAPI_INCLUDE_DIR ${_SketchUpAPI_DIR}/headers)
elseif(APPLE)
  set(_SketchUpAPI_LIBRARY_NAME "SketchUpAPI")
  set(_SketchUpAPI_LIBRARY_DIR ${_SketchUpAPI_DIR})
  set(_SketchUpAPI_INCLUDE_DIR ${_SketchUpAPI_DIR})
endif()

message(DEBUG "_SketchUpAPI_LIBRARY_DIR: ${_SketchUpAPI_LIBRARY_DIR}")
message(DEBUG "_SketchUpAPI_INCLUDE_DIR: ${_SketchUpAPI_INCLUDE_DIR}")

find_path(SketchUpAPI_INCLUDE_DIR "SketchUpAPI/sketchup.h"
  HINTS ${_SketchUpAPI_INCLUDE_DIR}
)
find_library(SketchUpAPI_LIBRARY ${_SketchUpAPI_LIBRARY_NAME}
  HINTS ${_SketchUpAPI_LIBRARY_DIR}
)

if(APPLE)
    # TODO: Search multiple version paths
    # TODO: Make this requred var on macOS?
    set(SketchUpAPI_BUNDLE_LOADER
      "/Applications/SketchUp 2019/SketchUp.app/Contents/MacOS/SketchUp"
      CACHE PATH "Bundle Loader"
    )
endif()

mark_as_advanced(
  SketchUpAPI_INCLUDE_DIR
  SketchUpAPI_LIBRARY
)

message(DEBUG "SketchUpAPI_LIBRARY: ${SketchUpAPI_LIBRARY}")
message(DEBUG "SketchUpAPI_INCLUDE_DIR: ${SketchUpAPI_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SketchUpAPI
  REQUIRED_VARS SketchUpAPI_LIBRARY SketchUpAPI_INCLUDE_DIR
  VERSION_VAR SketchUpAPI_VERSION)

if(SketchUpAPI_FOUND)
  set(SketchUpAPI_LIBRARIES ${SketchUpAPI_LIBRARY})
  set(SketchUpAPI_INCLUDE_DIRS ${SketchUpAPI_INCLUDE_DIR})
endif()

  # https://cmake.org/cmake/help/latest/manual/cmake-developer.7.html#a-sample-find-module
if(SketchUpAPI_FOUND AND NOT TARGET SketchUp::SketchUpAPI)
  add_library(SketchUp::SketchUpAPI UNKNOWN IMPORTED)
  set_target_properties(SketchUp::SketchUpAPI PROPERTIES
    IMPORTED_LOCATION "${SketchUpAPI_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SketchUpAPI_INCLUDE_DIR}"
  )
endif()
