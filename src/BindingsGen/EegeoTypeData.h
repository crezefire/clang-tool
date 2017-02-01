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
    struct Type {
        enum types {
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

        std::string Keyword;
        StringRef TypeName;
        std::string Name;

        static StringRef GetTypeName(types param);
    };

    struct Method {
        std::string Name;
        Type ReturnType;
        std::vector<Type> Params;
    };

    std::tuple<bool, std::string, Eegeo::Type::types> CheckBuiltinType(clang::QualType CanonicalType, clang::ASTContext& Context);

    Type ProcessReturnType(clang::QualType type, clang::ASTContext& context);

    Type ProcessParamType(clang::QualType type, clang::ASTContext& context);
}
