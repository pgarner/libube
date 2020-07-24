#
# Copyright 2020 by Philip N. Garner
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, July 2020
#

macro(exe_diff_test)
  set(monoVal REF OUT)
  set(manyVal CMD)
  cmake_parse_arguments(EXE "" "${monoVal}" "${manyVal}" ${ARGN})

  # Run the test
  execute_process(
    COMMAND ${EXE_CMD}
    OUTPUT_FILE ${EXE_OUT}
    RESULT_VARIABLE RETURN_TESTS
    )
  if(RETURN_TESTS)
    message(FATAL_ERROR "Test returned non-zero value ${RETURN_TESTS}")
  endif()

  # Use CMake to compare the reference and output files
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E compare_files ${EXE_OUT} ${EXE_REF}
    RESULT_VARIABLE RETURN_COMPARE
    )
  if(RETURN_COMPARE)
    message(FATAL_ERROR "Test failed: ${EXE_REF} and ${EXE_OUT} differ")
  endif()
endmacro()
