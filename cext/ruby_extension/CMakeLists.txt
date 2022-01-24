include(RubyExtension)

find_package(SketchUpRuby 2022.0 REQUIRED)

# CMake recommends not using GLOB to collect a list of source files from
# the source tree.
# https://cmake.org/cmake/help/latest/command/file.html#filesystem
set(HEADERS
  ruby/framework.h
  utilities.h
)
set(SOURCES
  example_extension.cpp
  utilities.cpp
)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR} PREFIX "Header Files" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} PREFIX "Source Files" FILES ${SOURCES})

# TODO(Example): Name your extension here. This will be the filename of the
# binaries:
set(EXTENSION_NAME example)

# TODO(Example): Set the versions of Ruby you build against here:
# This should be a list of versions with major and minor denotation:
#
#   set(EXTENSION_RUBY_VERSIONS "2.2" "2.5")
set(EXTENSION_RUBY_VERSIONS "2.5" "2.7")
foreach(RUBY_VERSION ${EXTENSION_RUBY_VERSIONS})

  message(STATUS "Configuring Ruby extension for Ruby ${RUBY_VERSION}")

  add_ruby_extension_library(
    ${EXTENSION_NAME} EXTENSION_LIB
    ${RUBY_VERSION} SketchUpRuby_LIBRARY
    "${HEADERS};${SOURCES}"
  )

  target_link_libraries(${EXTENSION_LIB} PRIVATE
    imagelib_static
  )

  # Generate version info header:
  configure_file(example_config.h.in example_config.h)

  # Copy binaries to directories where they are easier to locate.
  # RUNTIME DESTINATION is needed for Windows (SHARED).
  # RUNTIME DESTINATION is needed for Mac (MODULE).
  install(TARGETS ${EXTENSION_LIB}
    RUNTIME DESTINATION ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>/${RUBY_VERSION}
    LIBRARY DESTINATION ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>/${RUBY_VERSION}
  )

  # Copy binaries into the Ruby extension's path.
  install(TARGETS ${EXTENSION_LIB}
    RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/ruby/live-c-api/cext/${RUBY_VERSION}
    LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/ruby/live-c-api/cext/${RUBY_VERSION}
  )

endforeach() # EXTENSION_RUBY_VERSIONS
