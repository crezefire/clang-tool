#include "BindingsGen/EegeoASTMatcher.h"

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;
using namespace llvm;

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory MyToolCategory("my-tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...");

auto ClassDeclMatcher = clang::ast_matchers::cxxRecordDecl(clang::ast_matchers::isDefinition()).bind(Eegeo::classBindName);
auto MemberFunctionMatcher = clang::ast_matchers::cxxMethodDecl().bind(Eegeo::methodBindName);

int main(int argc, const char **argv) {
    CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
    ClangTool Tool(OptionsParser.getCompilations(),
        OptionsParser.getSourcePathList());

    Eegeo::EegeoASTMatcher Printer(Eegeo::removePath(OptionsParser.getSourcePathList()[0]));
    MatchFinder Finder;

    Finder.addMatcher(ClassDeclMatcher, &Printer);
    Finder.addMatcher(MemberFunctionMatcher, &Printer);

    auto ret = Tool.run(newFrontendActionFactory(&Finder).get());

    Printer.DumpJSON();

    //system("pause");

    return ret;
}
