set(currsources
	src/gtest.cc
	src/gtest_main.cc
	src/gtest-death-test.cc
	src/gtest-filepath.cc
	src/gtest-internal-inl.h
	src/gtest-port.cc
	src/gtest-printers.cc
	src/gtest-test-part.cc
	src/gtest-typed-test.cc
)

set(source_files ${source_files} ${currsources})

source_group(${curr_lib_name} FILES ${currsources})