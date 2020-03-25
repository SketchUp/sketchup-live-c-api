# https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html
# https://cmake.org/cmake/help/latest/manual/cmake-developer.7.html#manual:cmake-developer(7)

# ${SketchUpAPI_DIR}
# ${SketchUpAPI_SEARCH_DIR}
# $ENV{SketchUpAPI_DIR}
# $ENV{SketchUpAPI_SEARCH_DIR}

set(_SketchUpAPI_2020_0
  2020.0
  SDK_WIN_x64_2020-0-363
  SDK_Mac_2020-0-362
)
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
  ${_SketchUpAPI_2020_0}
  ${_SketchUpAPI_2019_3}
  ${_SketchUpAPI_2019_2}
)

set(_num_versions 3) # UPDATE: Should match items in `_SketchUpAPIs`
set(_version_size 3) # Size of the version arrays.

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
  set(_SketchUpAPI_LIBRARY_NAME "SketchUpAPI") # Standalone C API
  set(_SketchUpAPI_LIVE_LIBRARY_NAME "sketchup") # Live C API
  set(_SketchUpAPI_LIBRARY_DIR ${_SketchUpAPI_DIR}/binaries/sketchup/x64)
  set(_SketchUpAPI_INCLUDE_DIR ${_SketchUpAPI_DIR}/headers)
elseif(APPLE)
  set(_SketchUpAPI_LIBRARY_NAME "SketchUpAPI") # Standalone C API
  set(_SketchUpAPI_LIVE_LIBRARY_NAME "SketchUpAPI") # Live C API (Don't link!)
  set(_SketchUpAPI_LIBRARY_DIR ${_SketchUpAPI_DIR})
  set(_SketchUpAPI_INCLUDE_DIR ${_SketchUpAPI_DIR})
endif()
set(_SketchUpAPI_COMMON_PREFERENCES_LIBRARY_NAME "SketchUpCommonPreferences")

message(DEBUG "_SketchUpAPI_LIBRARY_DIR: ${_SketchUpAPI_LIBRARY_DIR}")
message(DEBUG "_SketchUpAPI_INCLUDE_DIR: ${_SketchUpAPI_INCLUDE_DIR}")

find_path(SketchUpAPI_INCLUDE_DIR "SketchUpAPI/sketchup.h"
  HINTS ${_SketchUpAPI_INCLUDE_DIR}
)
# Standalone C API
find_library(SketchUpAPI_LIBRARY ${_SketchUpAPI_LIBRARY_NAME}
  HINTS ${_SketchUpAPI_LIBRARY_DIR}
)
# Live C API
find_library(SketchUpAPI_LIVE_LIBRARY ${_SketchUpAPI_LIVE_LIBRARY_NAME}
  HINTS ${_SketchUpAPI_LIBRARY_DIR}
)

if(APPLE)
    # TODO: Pick path based on requested version.
    # TODO: Search multiple version paths
    # TODO: Make this requred var on macOS?
    # TODO: Can the logic of setting this on the target that links to it be
    #       done here in this module?
    set(SketchUpAPI_BUNDLE_LOADER
      "/Applications/SketchUp 2019/SketchUp.app/Contents/MacOS/SketchUp"
      CACHE PATH "Bundle Loader"
    )
endif()

mark_as_advanced(
  SketchUpAPI_INCLUDE_DIR
  SketchUpAPI_LIBRARY
  SketchUpAPI_LIVE_LIBRARY
)

message(DEBUG "SketchUpAPI_LIBRARY: ${SketchUpAPI_LIBRARY}")
message(DEBUG "SketchUpAPI_LIVE_LIBRARY: ${SketchUpAPI_LIVE_LIBRARY}")
message(DEBUG "SketchUpAPI_INCLUDE_DIR: ${SketchUpAPI_INCLUDE_DIR}")
message(DEBUG "SketchUpAPI_COMMON_PREFERENCES_LIBRARY: ${SketchUpAPI_COMMON_PREFERENCES_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SketchUpAPI
  REQUIRED_VARS
    SketchUpAPI_LIBRARY
    SketchUpAPI_LIVE_LIBRARY
    SketchUpAPI_INCLUDE_DIR
  VERSION_VAR
    SketchUpAPI_VERSION)

if(SketchUpAPI_FOUND)
  set(SketchUpAPI_LIBRARIES ${SketchUpAPI_LIBRARY} ${SketchUpAPI_COMMON_PREFERENCES_LIBRARY})
  set(SketchUpAPI_LIVE_LIBRARIES ${SketchUpAPI_LIVE_LIBRARY} ${SketchUpAPI_COMMON_PREFERENCES_LIBRARY})
  set(SketchUpAPI_INCLUDE_DIRS ${SketchUpAPI_INCLUDE_DIR})

  # These DLLs needs to be loaded by the consumer of the standalone SketchUp C API.
  if(WIN32)
    set(SketchUpAPI_BINARIES
      ${_SketchUpAPI_LIBRARY_DIR}/${_SketchUpAPI_LIBRARY_NAME}.dll
      ${_SketchUpAPI_LIBRARY_DIR}/${_SketchUpAPI_COMMON_PREFERENCES_LIBRARY_NAME}.dll
    )
  endif()
endif()

# https://cmake.org/cmake/help/latest/manual/cmake-developer.7.html#a-sample-find-module
# https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#imported-targets
# Professional CMake - Chapter 5.2.
if(SketchUpAPI_FOUND AND NOT TARGET SketchUp::SketchUpAPI)
  if(APPLE)
    set(SketchUpAPI_LIBRARY "${SketchUpAPI_LIBRARY}/${_SketchUpAPI_LIBRARY_NAME}")
  endif()

  # Standalone C API
  add_library(SketchUp::SketchUpAPI SHARED IMPORTED)
  set_target_properties(SketchUp::SketchUpAPI PROPERTIES
    IMPORTED_LOCATION "${SketchUpAPI_LIBRARY}"
    IMPORTED_IMPLIB "${SketchUpAPI_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SketchUpAPI_INCLUDE_DIR}"
  )

  # Live C API
  # TODO: Can UNKNOWN be omitted?
  # TODO: Mac should not link to the SketchUpAPI framework - only use include dir.
  if(WIN32)
    # On Windows we must link to sketchup.lib to use the Live C API.
    add_library(SketchUp::SketchUpLiveAPI UNKNOWN IMPORTED)
    set_target_properties(SketchUp::SketchUpLiveAPI PROPERTIES
      IMPORTED_LOCATION "${SketchUpAPI_LIVE_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${SketchUpAPI_INCLUDE_DIR}"
    )
  elseif(APPLE)
    # TODO: Can this be SHARED?
    # TODO: Can INTERFACE be used on Windows as well?
    #       Just omit IMPORTED_LOCATION on mac.
    add_library(SketchUp::SketchUpLiveAPI INTERFACE IMPORTED)
    set_target_properties(SketchUp::SketchUpLiveAPI PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${SketchUpAPI_INCLUDE_DIR}"
    )
    # Since we aren't linking to the SketchUpAPI.framework we must set the
    # bundle loader to a version of SketchUp.
    # NOTE: This doesn't lock the compiled binary to this SketchUp version, it's
    #       only needed during linking. The only requirement is that the version
    #       used support the SLAPI features used.
    target_link_options(SketchUp::SketchUpLiveAPI INTERFACE
      LINKER:-bundle -bundle_loader ${SketchUpAPI_BUNDLE_LOADER}
    )
    # TODO: Check @executable_path etc...
  endif()
endif()
