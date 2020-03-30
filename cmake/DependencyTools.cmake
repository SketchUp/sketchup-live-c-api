# Utility function to validate dependencies.
function(validate_dependencies TARGET_NAME BINARY_FILE)
  # TODO: Make EXPECTED and UNEXPECT named arguments.

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
  endif()
endfunction()
