#Sourced files should use add_sub for a separate project
#add_subdirectory(libs/sample-lib/ sample-lib)
#add_subdirectory(libs/googletest/ googletest)
#add_subdirectory(libs/googlebenchmark/ googlebenchmark)
include(libs/header-only/CMakeLists.txt)

set(additional_libs
	${additional_libs}
#	sample-lib
#	googletest
#	googlebenchmark	
	kernel32.lib
	user32.lib
	gdi32.lib
	winspool.lib
	shell32.lib
	ole32.lib
	oleaut32.lib
	uuid.lib
	comdlg32.lib
	advapi32.lib
	psapi.lib
	version.lib
	clangAST.lib
	clangASTMatchers.lib
	clangBasic.lib
	clangTooling.lib
	clangParse.lib
	clangToolingCore.lib
	clangLex.lib
	clangFrontend.lib
	clangDriver.lib
	LLVMCore.lib
	LLVMMC.lib
	LLVMSupport.lib
	LLVMOption.lib
	clangSerialization.lib
	clangSema.lib
	clangEdit.lib
	LLVMBitReader.lib
	clangAnalysis.lib
	LLVMProfileData.lib
	LLVMMCParser.lib
)

set(additional_includes
	${additional_includes}
	libs/
	libs/clang-include
	libs/llvm-include
)

set(linker_includes
	${linker_includes}
	libs/clang-libs
#	googletest
#	googlebenchmark
)
