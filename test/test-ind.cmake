#
# Copyright 2013 by Philip N. Garner
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, December 2013
#

include(LubeTest)
exe_diff_test(
  CMD ./test-ind
  REF ${TEST_DIR}/test-ind-ref.txt
  OUT test-ind-out.txt
  )
