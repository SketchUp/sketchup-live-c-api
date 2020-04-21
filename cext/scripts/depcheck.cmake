cmake_minimum_required(VERSION 3.17.0)

message(DEBUG "BINARY_FILE: ${BINARY_FILE}")
message(DEBUG "EXPECTED: ${EXPECTED}")
message(DEBUG "UNEXPECTED: ${UNEXPECTED}")

if(NOT DEFINED BINARY_FILE)
  message(FATAL_ERROR "Required variable BINARY_FILE missing.")
endif()

function(is_valid_for_platform FILENAME OUT)
  set(VALID FALSE)
  if(WIN32)
    set(VALID_EXTENSIONS .exe .dll)
  elseif(APPLE)
    set(VALID_EXTENSIONS .bundle .framework)
  else()
    message(FATAL_ERROR "Unsupported platform.")
  endif()
  # message(DEBUG "is_valid_for_platform")
  # message(DEBUG "FILENAME: ${FILENAME}")
  get_filename_component(FILE_EX ${FILENAME} LAST_EXT)
  # message(DEBUG "FILE_EX: ${FILE_EX}")
  list(FIND VALID_EXTENSIONS ${FILE_EX} INDEX)
  # message(DEBUG "INDEX: ${INDEX}")
  # message(DEBUG "valid_for_platform: FILE_EX: ${FILE_EX} - INDEX: ${INDEX}")
  if(NOT ${INDEX} EQUAL -1)
    set(VALID TRUE)
  endif()
  set(${OUT} ${VALID} PARENT_SCOPE)
endfunction()

function(has_dependency FILE DEPENDENCY OUT)
  set(FOUND NO)
  if(WIN32)
    find_program(DUMPBIN "dumpbin") # MSVC compiler setup required?
    message(DEBUG "dumpbin: ${DUMPBIN}")
    if(NOT DUMPBIN)
      message(WARNING "Unable to locate `dumpbin`.")
    endif()

    message(DEBUG "${DUMPBIN} /DEPENDENTS ${BINARY_FILE}")
    execute_process(
      COMMAND ${DUMPBIN} /DEPENDENTS "${BINARY_FILE}"
      RESULT_VARIABLE EXIT_CODE
      OUTPUT_VARIABLE STDOUT
      # ERROR_VARIABLE STDERR
      # OUTPUT_QUIET
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_STRIP_TRAILING_WHITESPACE
    )
    message(DEBUG "Exit Code: ${EXIT_CODE}")
    # message(DEBUG "stdout: ${STDOUT}")
    # message(DEBUG "stderr: ${STDERR}")
    string(FIND ${STDOUT} ${DEPENDENCY} MATCH)
    message(DEBUG "MATCH: ${MATCH}")
    if(NOT ${MATCH} EQUAL -1)
      set(FOUND YES)
    endif()

  elseif(APPLE)
    # Multiple COMMAND in execute_process pipes the output of the first COMMAND
    # to the input of the next.
    message(DEBUG "otool -L ${BINARY_FILE} | grep ${DEPENDENCY}")
    execute_process(
      COMMAND otool -L "${BINARY_FILE}"
      COMMAND grep "${DEPENDENCY}"
      RESULT_VARIABLE EXIT_CODE
      # OUTPUT_VARIABLE STDOUT
      # ERROR_VARIABLE STDERR
      OUTPUT_QUIET
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_STRIP_TRAILING_WHITESPACE
    )
    message(DEBUG "Exit Code: ${EXIT_CODE}")
    # message(DEBUG "stdout: ${STDOUT}")
    # message(DEBUG "stderr: ${STDERR}")
    if(${EXIT_CODE} EQUAL 0)
      set(FOUND YES)
    endif()
  else()
    message(WARNING "Unable to check dependency, unsupported platform.")
  endif()
  set(${OUT} ${FOUND} PARENT_SCOPE)
endfunction()

function(check_dependencies FILE IS_EXPECTED OUT_STATUS)
  set(DEPENDENCIES ${ARGN})
  set(PASSED TRUE)
  # message(DEBUG "")
  # message(DEBUG "check_dependencies")
  # message(DEBUG "FILE: ${FILE}")
  # message(DEBUG "IS_EXPECTED: ${IS_EXPECTED}")
  # message(DEBUG "DEPENDENCIES: ${DEPENDENCIES}")
  foreach(DEPENDENCY ${DEPENDENCIES})
    # Only check dependencies relevent for the current platform.
    message(DEBUG "DEPENDENCY: ${DEPENDENCY}")
    is_valid_for_platform(${DEPENDENCY} VALID)
    if(NOT VALID)
      continue()
    endif()

    message(CHECK_START "Checking for dependency (Expected: ${IS_EXPECTED}): ${DEPENDENCY}")
    has_dependency(${BINARY_FILE} ${DEPENDENCY} DEPENDENCY_FOUND)
    message(DEBUG "Found: ${DEPENDENCY_FOUND}")
    if((${IS_EXPECTED} AND DEPENDENCY_FOUND) OR (NOT ${IS_EXPECTED} AND NOT DEPENDENCY_FOUND))
      message(CHECK_PASS "Passed!")
    else()
      set(PASSED FALSE)
      message(CHECK_PASS "Failed!")
    endif()
  endforeach()

  if(NOT DEFINED ${OUT_STATUS})
    set(${OUT_STATUS} ${PASSED} PARENT_SCOPE)
  elseif(NOT PASSED)
    set(${OUT_STATUS} FALSE PARENT_SCOPE)
  endif()
endfunction()

message(CHECK_START "Checking dependencies for ${BINARY_FILE}")

if(EXPECTED)
  check_dependencies(${BINARY_FILE} YES PASSED_STATUS ${EXPECTED})
endif()

if(UNEXPECTED)
  check_dependencies(${BINARY_FILE} NO PASSED_STATUS ${UNEXPECTED})
endif()

if(PASSED_STATUS)
  message(CHECK_PASS "Passed!")
else()
  message(CHECK_FAIL "Failed!")
  message(FATAL_ERROR "Dependency check failed")
endif()
