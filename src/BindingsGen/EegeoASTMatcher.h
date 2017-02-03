#pragma once

//For AST matching
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "EegeoTypeData.h"

namespace Eegeo {

    constexpr auto classBindName = "class";
    constexpr auto methodBindName = "method";

    struct FileModule {
        std::string ClassName;
        std::string Namespace;
        llvm::SmallVector<Eegeo::InterfaceMethod, 10> Methods;
    };

    const auto removePath = [](const std::string& Str) {
        auto name = Str.find_last_of('\\');

        if (name == std::string::npos) {
            name = Str.find_last_of('/');
        }

        if (name == std::string::npos) {
            return Str;
        }

        return Str.substr(name + 1);
    };

    class EegeoASTMatcher : public clang::ast_matchers::MatchFinder::MatchCallback {
        llvm::raw_ostream& EStream{ llvm::errs() };
        llvm::raw_ostream& OStream{ llvm::outs() };
        llvm::StringRef SourceFile;

        llvm::SmallVector<FileModule, 1> Modules;

        llvm::Optional<int> getModuleIndexFromClassName(llvm::StringRef ClassName);

    public:
        EegeoASTMatcher(llvm::StringRef Ref);

        void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override;

        void DumpJSON();
    };
}
