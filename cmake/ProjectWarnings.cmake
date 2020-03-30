# Utility function to set base warning properties.
function(set_project_warnings TARGET_NAME)
  # https://arne-mertz.de/2018/07/cmake-properties-options/
  if(MSVC)
    # Elevate warning level.
    # Ignore unknown pragmas.
    target_compile_options(${TARGET_NAME} INTERFACE
      /W4 /WX
      /wd4068
    )
  else()
    # Elevate warning level.
    # Ignore unknown pragmas.
    target_compile_options(${TARGET_NAME} INTERFACE
      -Wall -Wextra -pedantic -Werror
      -Wno-unknown-pragmas
    )
  endif()
endfunction()
