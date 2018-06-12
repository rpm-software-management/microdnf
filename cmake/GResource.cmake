find_program (GLIB_COMPILE_RESOURCES_EXECUTABLE glib-compile-resources)
mark_as_advanced (GLIB_COMPILE_RESOURCES_EXECUTABLE)

include (CMakeParseArguments)

function (GLIB_COMPILE_RESOURCES output_var input)
  cmake_parse_arguments (
      ARGS
      "MANUAL_REGISTER;INTERNAL"
      "C_PREFIX"
      "DATA_DIR"
      ${ARGN})

  set (in_file ${CMAKE_CURRENT_SOURCE_DIR}/${input})
  get_filename_component (WORKING_DIR ${in_file} PATH)
  string (REGEX REPLACE "\\.xml" ".c" input ${input})
  set (out_file "${CMAKE_CURRENT_BINARY_DIR}/${input}")
  get_filename_component (OUTPUT_DIR ${out_file} PATH)
  file (MAKE_DIRECTORY ${OUTPUT_DIR})

  execute_process (
    COMMAND
      ${GLIB_COMPILE_RESOURCES_EXECUTABLE}
        --generate-dependencies
        ${in_file}
    WORKING_DIRECTORY ${WORKING_DIR}
    OUTPUT_VARIABLE in_file_dep)
  string (REGEX REPLACE "\r?\n$" ";" in_file_dep "${in_file_dep}")
  set (in_file_dep_path "")
  foreach (dep ${in_file_dep})
    list (APPEND in_file_dep_path "${WORKING_DIR}/${dep}")
  endforeach ()

  set (additional_args "")
  if (ARGS_MANUAL_REGISTER)
    list (APPEND additional_args "--manual-register")
  endif ()
  if (ARGS_INTERNAL)
    list (APPEND additional_args "--internal")
  endif ()
  if (ARGS_C_PREFIX)
    list (APPEND additional_args "--c-name=${ARGS_C_PREFIX}")
  endif ()
  add_custom_command (
    OUTPUT ${out_file}
    WORKING_DIRECTORY ${WORKING_DIR}
    COMMAND
      ${GLIB_COMPILE_RESOURCES_EXECUTABLE}
    ARGS
      "--generate-source"
      "--target=${out_file}"
      ${additional_args}
      ${in_file}
    DEPENDS
      ${in_file};${in_file_dep_path})
  set (${output_var} ${out_file} PARENT_SCOPE)
endfunction ()
