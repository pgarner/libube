#
# Copyright 2015 by Philip N. Garner
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, June 2015
#

include(LubeTest)
exe_diff_test(
  CMD ./test-graph
  REF ${TEST_DIR}/test-graph-ref.txt
  OUT test-graph-out.txt
  )
