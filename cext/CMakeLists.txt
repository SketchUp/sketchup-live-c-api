add_subdirectory(image_lib)
add_subdirectory(tests)

find_package(SketchUpRuby 2019.0 REQUIRED)
# TODO: Can we import selected targets from Find Modules?
# https://cmake.org/cmake/help/latest/manual/cmake-developer.7.html#find-modules
# find_package(SketchUpRuby 2019.0 REQUIRED
#   COMPONENTS
#     Ruby220 Ruby250
# )

# CMake recommends not using GLOB to collect a list of source files from
# the source tree.
# https://cmake.org/cmake/help/latest/command/file.html#filesystem
set(SOURCES
  ruby/example_extension.cpp
  ruby/utilities.cpp
)

# TODO(Example): Name your extension here. This will be the filename of the
# binaries:
set(EXTENSION_NAME example)

# TODO(Example): Set the versions of Ruby you build against here:
set(EXTENSION_RUBY_VERSIONS "2.2" "2.5")
foreach(RUBY_VERSION ${EXTENSION_RUBY_VERSIONS})
  # Generate a "250" style string from "2.5" style string:
  # TODO: Offer utility functions to generate the variables.
  string(REPLACE "." "" RUBY_VERSION_ID ${RUBY_VERSION})
  set(RUBY_VERSION_ID "${RUBY_VERSION_ID}0")

  # Ensure a lib with unique name for each Ruby version:
  set(EXTENSION_LIB ${EXTENSION_NAME}_${RUBY_VERSION_ID})

  # Resolve the target name this Ruby version:
  set(SketchUpRuby_LIBRARY "SketchUp::SketchUpRuby_${RUBY_VERSION_ID}")

  if(APPLE)
    # For Mac, the library needs to be of Mach-O type: Bundle
    add_library(${EXTENSION_LIB} MODULE ${SOURCES})
  else()
    add_library(${EXTENSION_LIB} SHARED ${SOURCES})
  endif()

  target_link_libraries(${EXTENSION_LIB} PRIVATE imagelib_static)

  # Need to correct the names since the name of each library for each Ruby
  # version is exactly the same. By default CMake uses the CMake target name
  # for the file names.
  set_target_properties(${EXTENSION_LIB} PROPERTIES
    OUTPUT_NAME ${EXTENSION_NAME}
    DEFINE_SYMBOL ${EXTENSION_NAME}_EXPORTS
  )
  set_target_properties(${EXTENSION_LIB} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${RUBY_VERSION}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${RUBY_VERSION}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${RUBY_VERSION}"
  )

  # Generate version info header:
  configure_file(example_config.h.in example_config.h)

  # Generate export header:
  # https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html
  include("GenerateExportHeader")
  generate_export_header(${EXTENSION_LIB}
    BASE_NAME ${EXTENSION_NAME}
  )

  # Include path for CMake auto-generated files:
  target_include_directories(${EXTENSION_LIB} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

  # The projects include paths:
  target_include_directories(${EXTENSION_LIB} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

  # Third party libraries:
  target_link_libraries(${EXTENSION_LIB} PUBLIC ${SketchUpRuby_LIBRARY})

  set_default_warnings(${EXTENSION_LIB})

  # TODO: Wrap this into a utility function that can be re-used.
  # https://arne-mertz.de/2018/07/cmake-properties-options/
  if(MSVC)
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
    target_compile_options(${EXTENSION_LIB} PRIVATE /EHs)

    # Ruby require the library to have .so extension on Windows.
    # https://cmake.org/cmake/help/v3.0/command/set_target_properties.html
    # https://cmake.org/cmake/help/latest/command/set_target_properties.html
    set_target_properties(${EXTENSION_LIB} PROPERTIES SUFFIX ".so" )

    # On Windows we must link to sketchup.lib to use the Live C API.
    # TODO:
    # target_link_libraries(${EXTENSION_LIB} ${SketchUpAPI_LIBRARY})
  else()
    # Ruby require the library to have .bundle extension on OSX.
    set_target_properties(${EXTENSION_LIB} PROPERTIES SUFFIX ".bundle" )
    # Remove the "lib" prefix as the name must match the Init_* function.
    set_target_properties(${EXTENSION_LIB} PROPERTIES PREFIX "" )
  endif()

  # Verify targets link against live C API.
  validate_dependencies(${EXTENSION_LIB}
    $<TARGET_FILE:${EXTENSION_LIB}>
    sketchup.exe
    SketchUpAPI.dll
  )


  set(EXTENSION_DIR ${PROJECT_SOURCE_DIR}/src)
  set(EXTENSION_STAGE_DIR ${EXTENSION_DIR}/example/cext/stage)
  # RUNTIME DESTINATION is needed for Windows (SHARED).
  # RUNTIME DESTINATION is needed for Mac (MODULE).
  install(TARGETS ${EXTENSION_LIB}
    RUNTIME DESTINATION ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>/${RUBY_VERSION}
    LIBRARY DESTINATION ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>/${RUBY_VERSION}
  )
  install(TARGETS ${EXTENSION_LIB}
    RUNTIME DESTINATION ${EXTENSION_STAGE_DIR}/${RUBY_VERSION}
    LIBRARY DESTINATION ${EXTENSION_STAGE_DIR}/${RUBY_VERSION}
  )
  # TODO: Clean extension cext directory for anything except the stage directory.
  # https://cmake.org/cmake/help/latest/command/install.html#custom-installation-logic

  # TODO: Windows: Verify linking to sketchup.exe and not SketchUpAPI.dll
  # dumpbin example.so /DEPENDENTS
  # TODO: macOS: Verify linking to sketchup.exe and not SketchUpAPI.dll
  # ...

endforeach() # EXTENSION_RUBY_VERSIONS
