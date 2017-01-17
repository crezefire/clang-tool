set(currsources
	src/stat.h
	src/colorprint.cc
	src/string_util.h
	src/sysinfo.h
	src/re.h
	src/json_reporter.cc
	src/csv_reporter.cc
	src/timers.h
	src/console_reporter.cc
	src/sleep.h
	src/mutex.h
	src/log.h
	src/complexity.h
	src/cycleclock.h
	src/sleep.cc
	src/colorprint.h
	src/timers.cc
	src/complexity.cc
	src/internal_macros.h
	src/arraysize.h
	src/sysinfo.cc
	src/benchmark_api_internal.h
	src/benchmark.cc
	src/commandlineflags.h
	src/benchmark_register.cc
	src/string_util.cc
	src/commandlineflags.cc
	src/CMakeLists.txt
	src/reporter.cc
	src/check.h
)

set(source_files ${source_files} ${currsources})

source_group(${curr_lib_name} FILES ${currsources})



