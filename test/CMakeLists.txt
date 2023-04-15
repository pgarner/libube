#
# Copyright 2015 by Philip N. Garner
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, June 2015
#

set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-DTEST_DIR="${TEST_DIR}")

# Download a test wav file
set(ARCTIC "http://festvox.org/cmu_arctic/cmu_arctic")
set(TESTWAV "${TEST_DIR}/test.wav")
if (NOT EXISTS "${TESTWAV}")
  message(STATUS "Downloading ${TESTWAV}")
  file(DOWNLOAD
    "${ARCTIC}/cmu_us_bdl_arctic/wav/arctic_a0001.wav"
    "${TESTWAV}"
    )
endif()

# cmake is not a great directory to test on, but it stable and predictable.
# Must be relative for the paths to work on different machines.
add_definitions(-DCMAKE_DIR="../../cmake")

# General form of a test that produces an output to compare with a reference.
# The tests get built here, but they are run from a different cmake file.  This
# lets us compare output with a reference output rather than just test that
# something runs.
macro(add_diff_test NAME)
  add_executable(test-${NAME} test-${NAME}.cpp)
  target_link_libraries(test-${NAME} lube-shared)
  target_include_directories(test-${NAME} PUBLIC ${CMAKE_SOURCE_DIR}/lube)
  add_test(
    NAME ${NAME}
    COMMAND ${CMAKE_COMMAND} -D TEST_DIR=${TEST_DIR}
                             -D CMAKE_MODULE_PATH=${TEST_DIR}
                             -P ${TEST_DIR}/test-${NAME}.cmake
    )
endmacro()

add_diff_test(ind)
add_diff_test(var)
add_diff_test(file)
add_diff_test(stream)
add_diff_test(math)
add_diff_test(path)
add_diff_test(lapack)
add_diff_test(config)
add_diff_test(graph)
add_diff_test(json)
add_diff_test(curl)

add_executable(test-qwt test-qwt.cpp)
target_link_libraries(test-qwt lube-shared)
target_include_directories(test-qwt PUBLIC ${CMAKE_SOURCE_DIR}/lube)
# No test yet as it's a window thing
