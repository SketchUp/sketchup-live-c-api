message(DEBUG PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR})
message(DEBUG SketchUpAPI_INCLUDE_DIR: ${SketchUpAPI_INCLUDE_DIR})

# https://gitlab.com/CLIUtils/modern-cmake/blob/master/examples/extended-project/src/CMakeLists.txt
set(HEADER_LIST
  slapi/color.h
  slapi/utilities.h
  image.h
  trace.h
)
set(SOURCES
  slapi/color.cpp
  slapi/utilities.cpp
  image.cpp
  trace.cpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${SOURCES} ${HEADER_LIST})

# TODO(Example): Set static library name here:
set(LIBRARY_BASE_NAME imagelib)
# This is the library name for the library linking against the Live C API.
set(LIBRARY_NAME ${LIBRARY_BASE_NAME}_static)
# This is a static lib used for unit tests - which must link against the
# standalone version of the C API since they don't run under SketchUp.
set(TEST_LIBRARY_NAME ${LIBRARY_BASE_NAME}_test_static)

# Create a library for Live C API usage:
add_library(${LIBRARY_NAME} STATIC ${SOURCES} ${HEADER_LIST})
# Create a library for Standalone C API usage (tests etc.):
add_library(${TEST_LIBRARY_NAME} STATIC ${SOURCES} ${HEADER_LIST})

# The projects include paths:
target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(${TEST_LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# Link libraries:
target_link_libraries(${LIBRARY_NAME} SketchUp::SketchUpLiveAPI)
target_link_libraries(${TEST_LIBRARY_NAME} SketchUp::SketchUpAPI)

# All users of this library will need at least C++17
# TODO: Other properties?
target_compile_features(${LIBRARY_NAME} PUBLIC cxx_std_17)
target_compile_features(${TEST_LIBRARY_NAME} PUBLIC cxx_std_17)

# IDEs should put the headers in a nice place
# TODO:
# source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})

# Use the Unicode version of the Windows API.
# add_definitions(-DUNICODE -D_UNICODE)

# Avoid global pollution from Windows API.
# add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)

set_default_warnings(${LIBRARY_NAME})
set_default_warnings(${TEST_LIBRARY_NAME})

if(APPLE)
  # Since we aren't linking to the SketchUpAPI.framework we must set the
  # bundle loader to a version of SketchUp.
  # NOTE: This doesn't lock the compiled binary to this SketchUp version, it's
  #       only needed during linking. The only requirement is that the version
  #       used support the SLAPI features used.
  # TODO: Can this be performed by the find-module?
  # target_link_options(${LIBRARY_NAME} PUBLIC
  #   LINKER:-bundle -bundle_loader ${SketchUpAPI_BUNDLE_LOADER}
  # )
endif()
