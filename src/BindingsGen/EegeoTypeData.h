#pragma once

#include "clang/AST/Type.h"
#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/StringRef.h"

class clang::ASTContext;

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
        llvm::StringRef TypeName;
        std::string Name;

        static llvm::StringRef getTypeName(RSTKind param);
    };

    struct InterfaceMethod {
        std::string Name;
        RestrictedSimplifiedType ReturnType;
        std::vector<RestrictedSimplifiedType> Params;
        bool IsStatic{ false };
    };

    llvm::Optional<RestrictedSimplifiedType> processReturnType(clang::QualType Type, clang::SourceLocation SourceLoc, clang::ASTContext& Context);

    llvm::Optional<RestrictedSimplifiedType> processParamType(clang::QualType Type, std::string Name, clang::SourceLocation SourceLoc, clang::ASTContext& Context);

    llvm::Optional<std::pair<RSTKind, std::string>> getSimplifiedType(clang::QualType CurrentType, clang::SourceLocation SourceLoc, clang::ASTContext& Context);
}
