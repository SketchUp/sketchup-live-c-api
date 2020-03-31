# Utility function to set base project properties.
function(set_project_options TARGET_NAME)
  target_compile_features(${TARGET_NAME} INTERFACE
    cxx_std_17
  )
endfunction()
