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
	LLVMSupport.lib
	clangAST.lib
	clangASTMatchers.lib
	clangBasic.lib
	clangTidy.lib
	clangTidyBoostModule.lib
	clangTidyCERTModule.lib
	clangTidyCppCoreGuidelinesModule.lib
	clangTidyGoogleModule.lib
	clangTidyLLVMModule.lib
	clangTidyMiscModule.lib
	clangTidyModernizeModule.lib
	clangTidyMPIModule.lib
	clangTidyPerformanceModule.lib
	clangTidyReadabilityModule.lib
	clangTooling.lib
	clangTidyUtils.lib
	clangFormat.lib
	clangStaticAnalyzerFrontend.lib
	clangFrontend.lib
	clangDriver.lib
	version.lib
	LLVMOption.lib
	clangParse.lib
	LLVMMCParser.lib
	clangSerialization.lib
	clangSema.lib
	clangEdit.lib
	LLVMBitReader.lib
	LLVMProfileData.lib
	clangStaticAnalyzerCheckers.lib
	clangStaticAnalyzerCore.lib
	clangAnalysis.lib
	clangToolingCore.lib
	clangRewrite.lib
	clangLex.lib
	LLVMCore.lib
	LLVMMC.lib
	psapi.lib
	LLVMDemangle.lib
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
