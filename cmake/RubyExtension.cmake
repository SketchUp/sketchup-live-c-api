# Assumes:
# * The Ruby package has been found:
#     find_package(SketchUpRuby 20??.0 REQUIRED)
#
# Arguments:
# * EXTENSION_NAME - Base target name. Final name will depend on RUBY_VERSION
# * OUT_EXTENSION_LIB - Resolved name of the Ruby extension target.
#
# * RUBY_VERSION - Ruby version to link against; "2.2", "2.5" etc.
# * OUT_RUBY_LIB - Resolved name of the Ruby target being linked to.
#
# * SOURCES - Sources to be added to the library.
function(add_ruby_extension_library
    EXTENSION_NAME OUT_EXTENSION_LIB
    RUBY_VERSION OUT_RUBY_LIB
    SOURCES
  )

  # Generate a "250" style string from "2.5" style string:
  string(REPLACE "." "" RUBY_VERSION_ID ${RUBY_VERSION})
  set(RUBY_VERSION_ID "${RUBY_VERSION_ID}0")

  # Ensure a lib with unique name for each Ruby version:
  set(EXTENSION_LIB ${EXTENSION_NAME}_${RUBY_VERSION_ID})
  message(DEBUG "EXTENSION_LIB: ${EXTENSION_LIB}")

  # Resolve the target name this Ruby version:
  set(SketchUpRuby_LIBRARY "SketchUp::SketchUpRuby_${RUBY_VERSION_ID}")
  message(DEBUG "SketchUpRuby_LIBRARY: ${SketchUpRuby_LIBRARY}")

  if(APPLE)
    # For Mac, the library needs to be of Mach-O type: Bundle
    add_library(${EXTENSION_LIB} MODULE ${SOURCES})
  else()
    add_library(${EXTENSION_LIB} SHARED ${SOURCES})
  endif()

  target_link_libraries(${EXTENSION_LIB} PRIVATE
    project_options
    project_warnings
  )

  # Generate export header:
  set_target_properties(${EXTENSION_LIB} PROPERTIES
    DEFINE_SYMBOL "${EXTENSION_NAME}_EXPORTS"
  )
  include("GenerateExportHeader")
  generate_export_header(${EXTENSION_LIB}
    BASE_NAME ${EXTENSION_NAME}
  )

  # Include path for CMake auto-generated files:
  target_include_directories(${EXTENSION_LIB} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

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

  # The projects include paths:
  target_include_directories(${EXTENSION_LIB} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

  # Third party libraries:
  target_link_libraries(${EXTENSION_LIB} PUBLIC ${SketchUpRuby_LIBRARY})

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
  endif()

  if(WIN32)
    # Ruby require the library to have .so extension on Windows.
    set_target_properties(${EXTENSION_LIB} PROPERTIES SUFFIX ".so" )
  elseif(APPLE)
    # Ruby require the library to have .bundle extension on OSX.
    set_target_properties(${EXTENSION_LIB} PROPERTIES SUFFIX ".bundle" )
    # Remove the "lib" prefix as the name must match the Init_* function.
    set_target_properties(${EXTENSION_LIB} PROPERTIES PREFIX "" )
  endif()

  # Verify targets link against live C API.
  # TODO: Account for platform differences in names.
  include("DependencyTools")
  validate_dependencies(${EXTENSION_LIB}
    $<TARGET_FILE:${EXTENSION_LIB}>
    EXPECTED sketchup.exe
    UNEXPECTED SketchUpAPI.dll
  )

  # Make key variables available to parent scope:
  set(${OUT_EXTENSION_LIB} "${EXTENSION_LIB}" PARENT_SCOPE)
  set(${OUT_RUBY_LIB} "${SketchUpRuby_LIBRARY}" PARENT_SCOPE)

endfunction()
