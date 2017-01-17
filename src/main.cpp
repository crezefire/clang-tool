// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

//For AST matching
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

//#include <iostream>

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

constexpr auto classBindName = "class";
auto ClassDeclMatcher = cxxRecordDecl(isDefinition()).bind(classBindName);

constexpr auto methodBindName = "method";
auto MemberFunctionMatcher = cxxMethodDecl().bind(methodBindName);

class MatchProcessor : public MatchFinder::MatchCallback {
    raw_ostream& OS{ llvm::errs() };
    SmallString<20> ClassName;
    SmallVector<SmallVector<SmallString<20>, 10>, 10> MemberFunctions;

public:
    void run(const MatchFinder::MatchResult &Result) override {

        const auto *classTree = Result.Nodes.getNodeAs<clang::CXXRecordDecl>(classBindName);

        if (classTree && ClassName.empty()) {
            ClassName = classTree->getNameAsString();
        }

        if (const auto *methodTree = Result.Nodes.getNodeAs<clang::CXXMethodDecl>(methodBindName)) {

            if (isa<clang::CXXConstructorDecl>(methodTree)) { return; }

            auto returnType = methodTree->getReturnType()->getUnqualifiedDesugaredType();
            //auto typePtr = returnType.getTypePtr();

            auto ptr = methodTree->getReturnType()->getCanonicalTypeUnqualified();

            //auto print = methodTree->getReturnType()->getCanonicalTypeInternal()->getTypeClassName();

            //OS << methodTree->getReturnType()->decl <<  "========================\n";
            returnType->dump();

            if (!returnType->isBuiltinType()) {
                //auto print = methodTree->getReturnType()->getTypeClassName();// returnType->getTypeClassName();
                //OS << print << " " << methodTree->getDeclName() << "\n\n";
            }

        }

    }

    void printData() {
        //OS << ClassName;

        for (auto& function : MemberFunctions) {

        }

        OS << "\n";
    }
};

int main(int argc, const char **argv) {
    CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
    ClangTool Tool(OptionsParser.getCompilations(),
        OptionsParser.getSourcePathList());

    MatchProcessor Printer;
    MatchFinder Finder;
    Finder.addMatcher(ClassDeclMatcher, &Printer);
    Finder.addMatcher(MemberFunctionMatcher, &Printer);

    auto ret = Tool.run(newFrontendActionFactory(&Finder).get());

    Printer.printData();

    system("pause");

    return ret;
}