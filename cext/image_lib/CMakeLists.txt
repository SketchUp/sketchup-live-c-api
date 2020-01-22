find_package(SketchUpAPI 2019.3 REQUIRED)

message(DEBUG ${PROJECT_SOURCE_DIR})
message(DEBUG ${SketchUpAPI_INCLUDE_DIR})

# https://gitlab.com/CLIUtils/modern-cmake/blob/master/examples/extended-project/src/CMakeLists.txt
set(HEADER_LIST
  slapi/utilities.hpp
  slapi/color.hpp
  image.hpp
  trace.hpp
)
set(SOURCES
  slapi/color.cpp
  image.cpp
  trace.cpp
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${SOURCES} ${HEADER_LIST})

# TODO(Example): Set static library name here:
set(LIBRARY_NAME example_static)

add_library(${LIBRARY_NAME} STATIC ${SOURCES} ${HEADER_LIST})

# The projects include paths:
target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# Third party include paths:
target_include_directories(${LIBRARY_NAME} PUBLIC ${SketchUpAPI_INCLUDE_DIR})

# All users of this library will need at least C++17
# TODO:
# target_compile_features(modern_library PUBLIC cxx_std_17)

# IDEs should put the headers in a nice place
# TODO:
# source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})

# https://arne-mertz.de/2018/07/cmake-properties-options/
if(MSVC)
  # Use the Unicode version of the Windows API.
  # add_definitions(-DUNICODE -D_UNICODE)

  # Avoid global pollution from Windows API.
  # add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)

  # Elevate warning level.
  # Ignore unknown pragmas.
  target_compile_options(${LIBRARY_NAME} PRIVATE
    /W4 /WX
    /wd4068
  )

  # On Windows we must link to sketchup.lib to use the Live C API.
  # TODO: Isn't this linking against SketchUpAPI.lib? (standalone API)
  target_link_libraries(${LIBRARY_NAME} ${SketchUpAPI_LIBRARY})
else()
  # Elevate warning level.
  # Ignore unknown pragmas.
  target_compile_options(${LIBRARY_NAME} PRIVATE
    -Wall -Wextra -pedantic -Werror
    -Wno-unknown-pragmas
  )

  # Since we aren't linking to the SketchUpAPI.framework we must set the
  # bundle loader to a version of SketchUp.
  # NOTE: This doesn't lock the compiled binary to this SketchUp version, it's
  #       only needed during linking. The only requirement is that the version
  #       used support the SLAPI features used.
  target_link_options(${LIBRARY_NAME} PUBLIC
    LINKER:-bundle -bundle_loader ${SketchUpAPI_BUNDLE_LOADER}
  )
endif()
