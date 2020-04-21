#[=======================================================================[.rst:
DependencyTools
---------------

Functions to check for expected and unexpected dependencies.

It uses various tools to obtain the list of required shared library
files:

::

   dumpbin (Windows)
   otool (Mac OSX)

The following functions are provided by this module:

::

   validate_dependencies

::

  VALIDATE_DEPENDENCIES(<target>
                        EXPECTED <expected>...
                        [UNEXPECTED <unexpected>...])

Adds a post-build command to <target> that checks for <expected> and
<unexpected> dependencies.

If any of the <expected> dependencie are missing the command will exit with
an error code.

If any <unexpected> dependencies are found the command will exit with
an error code.

The function will use only the items in <expected> and <unexpected> that are
relevant for the current platform. The supported filetypes are:

   Windows
   - .dll
   - .exe

   macOS
   - .bundle
   - .framework

Example:

::

  include("DependencyTools")
  validate_dependencies(${EXTENSION_LIB}
    EXPECTED sketchup.exe
    UNEXPECTED SketchUpAPI.dll SketchUpAPI.framework
  )

#]=======================================================================]

function(validate_dependencies TARGET_NAME)
  # Define the supported set of keywords.
  set(noValues "")
  set(singleValues "")
  set(multiValues EXPECTED UNEXPECTED)

  # Process the arguments passed in.
  cmake_parse_arguments(ARG
    "${noValues}"
    "${singleValues}"
    "${multiValues}"
    ${ARGN}
  )

  message(DEBUG "")
  message(DEBUG "ARG_EXPECTED: ${ARG_EXPECTED}")
  message(DEBUG "ARG_UNEXPECTED: ${ARG_UNEXPECTED}")

  if(NOT DEFINED ARG_EXPECTED OR ARG_EXPECTED STREQUAL "")
    message(SEND_ERROR "Required argument EXPECTED missing.")
  endif()

  set(TARGET_BINARY $<TARGET_FILE:${TARGET_NAME}>)
  message(DEBUG "TARGET_BINARY: ${TARGET_BINARY}")

  # https://stackoverflow.com/a/20989991/486990
  string (REPLACE ";" "$<SEMICOLON>" CMD_EXPECTED "${ARG_EXPECTED}")
  string (REPLACE ";" "$<SEMICOLON>" CMD_UNEXPECTED "${ARG_UNEXPECTED}")
  add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
      -DBINARY_FILE=${TARGET_BINARY}
      -DEXPECTED=${CMD_EXPECTED}
      -DUNEXPECTED=${CMD_UNEXPECTED}
      -P ${PROJECT_SOURCE_DIR}/cext/scripts/depcheck.cmake
      # --log-level=DEBUG
    COMMENT "Validate dependencies of ${TARGET_BINARY}" VERBATIM
  )
endfunction()
