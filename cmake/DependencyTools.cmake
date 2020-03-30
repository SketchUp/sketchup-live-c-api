# Utility function to validate dependencies.
function(validate_dependencies TARGET_NAME BINARY_FILE EXPECTED UNEXPECTED)
  # TODO: Make EXPECTED and UNEXPECT named arguments.
  set(SCRIPTS_PATH "${PROJECT_SOURCE_DIR}/cext/scripts")
  if(MSVC)
    find_program(DUMPBIN "dumpbin")
    message(DEBUG dumpbin: ${DUMPBIN})
    add_custom_command(TARGET ${TARGET_NAME}
      POST_BUILD
      COMMAND ${SCRIPTS_PATH}/depcheck.bat
        ${DUMPBIN} ${BINARY_FILE} ${EXPECTED} ${UNEXPECTED}
      COMMENT "Validate dependencies of ${BINARY_FILE}>" VERBATIM
    )
  endif()
endfunction()
