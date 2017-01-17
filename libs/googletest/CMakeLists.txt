project(googletest)
set(curr_lib_name googletest)

set(folder_executable Executables)
set(folder_libraries Libraries)

# set(currsources
  # Header.h
  # Implementation.cpp
# )

# source_group(src FILES ${currsources})

include(src/CMakeLists.txt)

if (WIN32)
	include(../../cmake/WinCompilerFlags.txt)
	
	string(REPLACE "/Za" "" REMOVE_FLAG ${CMAKE_CXX_FLAGS})
	set(CMAKE_CXX_FLAGS ${REMOVE_FLAG})
	
	string(REPLACE "/GL" "" REMOVE_FLAG_R ${CMAKE_CXX_FLAGS_RELEASE})
	set(CMAKE_CXX_FLAGS_RELEASE ${REMOVE_FLAG_R})
elseif(UNIX)
	include(../../cmake/LinCompilerFlags.txt)
	find_package (Threads REQUIRED)
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

include_directories(
	include/
	.
)
#link_directories()

#Add SHARED for .so / dll
add_library(${curr_lib_name} ${currsources})

if (UNIX)
	target_link_libraries (${curr_lib_name} ${CMAKE_THREAD_LIBS_INIT})
endif()

set_property(TARGET ${curr_lib_name} PROPERTY FOLDER ${folder_libraries})

set(additional_includes
	${additional_includes}
	libs/googletest/include/
	PARENT_SCOPE
)
