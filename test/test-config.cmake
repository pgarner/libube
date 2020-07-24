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
  CMD ./test-config -a -c argument file1 file2
  REF ${TEST_DIR}/test-config-ref.txt
  OUT test-config-out.txt
  )
