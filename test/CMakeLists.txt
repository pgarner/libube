#
# Copyright 2015 by Philip N. Garner
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, June 2015
#

# Testing
#
# The tests get built here, but they are run from a different cmake
# file.  This lets us compare output with a reference output rather
# than just test that something runs.

add_executable(test-ind test-ind.cpp)
add_test(
  NAME ind
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/test-ind.cmake
  )

add_executable(test-var test-var.cpp)
target_link_libraries(test-var shared-lib)
add_test(
  NAME var
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/test-var.cmake
  )

add_executable(test-file test-file.cpp)
target_link_libraries(test-file shared-lib)
add_test(
  NAME file
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/test-file.cmake
  )

add_executable(test-math test-math.cpp)
target_link_libraries(test-math shared-lib)
add_test(
  NAME math
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/test-math.cmake
  )

add_executable(test-path test-path.cpp)
target_link_libraries(test-path shared-lib)
add_test(
  NAME path
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/test-path.cmake
  )

add_executable(test-lapack test-lapack.cpp)
target_link_libraries(test-lapack shared-lib)
add_test(
  NAME lapack
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/test-lapack.cmake
  )

add_executable(test-config test-config.cpp)
target_link_libraries(test-config shared-lib)
add_test(
  NAME config
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/test-config.cmake
  )

add_executable(test-graph test-graph.cpp)
target_link_libraries(test-graph shared-lib)
add_test(
  NAME graph
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/test-graph.cmake
  )
