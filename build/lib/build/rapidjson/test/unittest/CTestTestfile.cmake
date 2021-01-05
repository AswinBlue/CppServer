# CMake generated Testfile for 
# Source directory: /home/munjunwoo/server/lib/rapidjson/test/unittest
# Build directory: /home/munjunwoo/server/build/lib/build/rapidjson/test/unittest
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(unittest "/home/munjunwoo/server/build/bin/unittest")
set_tests_properties(unittest PROPERTIES  WORKING_DIRECTORY "/home/munjunwoo/server/bin")
add_test(valgrind_unittest "valgrind" "--suppressions=/home/munjunwoo/server/test/valgrind.supp" "--leak-check=full" "--error-exitcode=1" "/home/munjunwoo/server/build/bin/unittest" "--gtest_filter=-SIMD.*")
set_tests_properties(valgrind_unittest PROPERTIES  WORKING_DIRECTORY "/home/munjunwoo/server/bin")
