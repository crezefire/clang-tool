project(sample-lib)
set(curr_lib_name sample-lib)

set(folder_executable Executables)
set(folder_libraries Libraries)

set(currsources
  Header.h
  Implementation.cpp
)

source_group(src FILES ${currsources})

if (WIN32)
	include(../../cmake/WinCompilerFlags.txt)
elseif(UNIX)
	include(../../cmake/LinCompilerFlags.txt)
endif()

#TODO(vim): Check this is the same on Linux
#need to set relative to where this is going to be run from
#in this case will be run from build/sample-lib/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ../../bin/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE  ../../bin/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL  ../../bin/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO  ../../bin/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG  ../../bin/)
set(CMAKE_BINARY_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

#include_directories(.)
#link_directories()

#Add SHARED for .so / dll
add_library(${curr_lib_name} ${currsources})

set_property(TARGET ${curr_lib_name} PROPERTY FOLDER ${folder_libraries})

# set(additional_includes
	# ${additional_includes}
	# libs/sample-lib/
	# PARENT_SCOPE
# )