set(curr_lib_name header-only)

set(currsources
	libs/header-only/HeaderOnly.h
)

set(source_files ${source_files} ${currsources})

source_group(libs\\${curr_lib_name} FILES ${currsources})