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

constexpr auto endl = '\n';
constexpr auto tab = '\t';

namespace Eegeo {
    enum class RSTKind {
        Void,
        Floating,
        Integral,
        Boolean,
        Pointer,
        FunctionPointer,
        Array,
        Struct,
        String
    };
    
    struct RestrictedSimplifiedType {
        std::string Keyword;
        std::string Qualifiers;
        StringRef TypeName;
        std::string Name;

        static StringRef getTypeName(RSTKind param);
    };

    struct InterfaceMethod {
        std::string Name;
        RestrictedSimplifiedType ReturnType;
        std::vector<RestrictedSimplifiedType> Params;
    };

    llvm::Optional<RestrictedSimplifiedType> processReturnType(clang::QualType Type, clang::SourceLocation SourceLoc, clang::ASTContext& Context);

    llvm::Optional<RestrictedSimplifiedType> processParamType(clang::QualType Type, std::string Name, clang::SourceLocation SourceLoc, clang::ASTContext& Context);

    llvm::Optional<std::pair<RSTKind, std::string>> resolveTypeForBuiltin(clang::QualType CurrentType, clang::SourceLocation SourceLoc, clang::ASTContext& Context);
    
    llvm::Optional<std::pair<RSTKind, std::string>> getSimplifiedType(clang::QualType CurrentType, clang::SourceLocation SourceLoc, clang::ASTContext& Context);
}
