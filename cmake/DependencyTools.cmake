# Utility function to validate dependencies.
function(validate_dependencies TARGET_NAME BINARY_FILE)
  # Define the supported set of keywords.
  set(noValues "")
  set(singleValues EXPECTED UNEXPECTED)
  set(multiValues "")

  # Process the arguments passed in.
  cmake_parse_arguments(ARG
    "${noValues}"
    "${singleValues}"
    "${multiValues}"
    ${ARGN}
  )

  if(NOT DEFINED ARG_EXPECTED OR ARG_EXPECTED STREQUAL "")
    message(SEND_ERROR "Required argument EXPECTED missing.")
  endif()

  set(SCRIPTS_PATH "${PROJECT_SOURCE_DIR}/cext/scripts")
  if(MSVC)
    find_program(DUMPBIN "dumpbin")
    message(DEBUG dumpbin: ${DUMPBIN})
    add_custom_command(TARGET ${TARGET_NAME}
      POST_BUILD
      COMMAND ${SCRIPTS_PATH}/depcheck.bat
        ${DUMPBIN} ${BINARY_FILE} ${ARG_EXPECTED} ${ARG_UNEXPECTED}
      COMMENT "Validate dependencies of ${BINARY_FILE}>" VERBATIM
    )
  else()
    # TODO: Add macOS support.
    # otool -L <binary>
    #
    #   tthomas-us-la:2.5 tthomas2$ otool -L example.bundle
    # example.bundle:
    # 	@executable_path/../Frameworks/Ruby.framework/Versions/Current/Ruby (compatibility version 2.5.0, current version 2.5.1)
    # 	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.4)
    # 	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.250.1)
    #
    #   tthomas-us-la:image_lib tthomas2$ otool -L imagelib_tests
    # imagelib_tests:
    # 	@rpath/SketchUpAPI.framework/Versions/A/SketchUpAPI (compatibility version 1.0.0, current version 1.0.0)
    # 	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.4)
    #   /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.250.1)

    message(WARNING "Unable to validate dependencies: Unsupported compiler.")
  endif()
endfunction()
