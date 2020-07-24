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
  CMD ./test-json
  REF ${TEST_DIR}/test-json-ref.txt
  OUT test-json-out.txt
  )
