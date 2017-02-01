set(currsources
	src/BindingsGen/EegeoTypeData.cpp
	src/BindingsGen/EegeoTypeData.h
	src/BindingsGen/EegeoASTMatcher.cpp
	src/BindingsGen/EegeoASTMatcher.h
)

set(source_files ${source_files} ${currsources})

source_group(\\src\\BindingsGen\\ FILES ${currsources})

