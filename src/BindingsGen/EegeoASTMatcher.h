#pragma once

// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

//For AST matching
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "EegeoTypeData.h"

namespace Eegeo {

    constexpr auto classBindName = "class";
    constexpr auto methodBindName = "method";

    const auto RemovePath = [](const std::string& str) {
        auto name = str.find_last_of('\\');

        if (name == std::string::npos) {
            name = str.find_last_of('/');
        }

        if (name == std::string::npos) {
            //ERROR!!
        }

        return str.substr(name + 1);
    };

    class MatchProcessor : public clang::ast_matchers::MatchFinder::MatchCallback {
        llvm::raw_ostream& OS{ llvm::errs() };
        llvm::raw_ostream& Dump{ llvm::outs() };
        llvm::SmallVector<llvm::SmallVector<llvm::SmallString<20>, 10>, 10> MemberFunctions;
        StringRef SourceFile;

        llvm::SmallString<20> ClassName;
        llvm::SmallString<30> QualifiedClassName;
        llvm::SmallVector<Eegeo::InterfaceMethod, 10> Methods;
    public:
        MatchProcessor(StringRef ref)
            : SourceFile(std::move(ref)) {}

        void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override;

        void Print();
    };
}
