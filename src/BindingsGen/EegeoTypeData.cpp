#include "EegeoTypeData.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"

using namespace clang;
using namespace llvm;

namespace Eegeo {
    StringRef RestrictedSimplifiedType::getTypeName(RSTKind Param) {
        switch (Param) {
        case RSTKind::Void:
            return "Void";

        case RSTKind::Floating:
            return "Floating";

        case RSTKind::Integral:
            return "Integral";

        case RSTKind::Boolean:
            return "Boolean";

        case RSTKind::Pointer:
            return "Pointer";

        case RSTKind::FunctionPointer:
            return "FunctionPointer";

        case RSTKind::Array:
            return "Array";

        case RSTKind::Struct:
            return "Struct";

        case RSTKind::String:
            return "String";
        }

        assert(false && "Unknown SimpleType of RSTKind");
    }

    void ReportDiagnostic(ASTContext& Context, SourceLocation SourceLoc, StringRef Message, DiagnosticIDs::Level Level) {
        auto& Engine = Context.getDiagnostics();
        const auto DiagID = Engine.getDiagnosticIDs()->getCustomDiagID(Level, Message);
        Engine.Report(SourceLoc, DiagID);
    }

    const auto ReportError = [&](ASTContext& Context, SourceLocation SourceLoc, StringRef Message) {
        ReportDiagnostic(Context, SourceLoc, Message, DiagnosticIDs::Level::Error);
    };

    const auto ReportNote = [&](ASTContext& Context, SourceLocation SourceLoc, StringRef Message) {
        ReportDiagnostic(Context, SourceLoc, Message, DiagnosticIDs::Level::Note);
    };

    Optional<std::pair<RSTKind, std::string>> resolveTypeForBuiltin(QualType CurrentType, SourceLocation SourceLoc, ASTContext& Context) {
        const auto BT = dyn_cast<BuiltinType>(CurrentType);

        if (!BT) {
            return NoneType::None;
        }

        CurrentType.removeLocalCVRQualifiers(Qualifiers::CVRMask);

        auto Keyword = CurrentType.getAsString();
        const auto Pos = Keyword.find("_Bool");

        RSTKind SimpleType;

        if (Pos != std::string::npos) {
            Keyword.replace(Pos, 5, "bool");
            SimpleType = RSTKind::Boolean;
        }
        else if (CurrentType->isVoidType()) {
            SimpleType = RSTKind::Void;
        }
        else if (CurrentType->isIntegralType(Context)) {
            SimpleType = RSTKind::Integral;
        }
        else if (CurrentType->isFloatingType()) {
            auto Kind = BT->getKind();

            //For future use case
            if (Kind == BuiltinType::Kind::Double || Kind == BuiltinType::Kind::LongDouble)
                SimpleType = RSTKind::Floating;
            else
                SimpleType = RSTKind::Floating;
        }
        else {
            ReportError(Context, SourceLoc, "Unsupported builtin type found");
            return NoneType::None;
        }

        return std::make_pair(SimpleType, std::move(Keyword));
    }

    Optional<std::pair<RSTKind, std::string>> resolveTypeForDecayed(QualType CurrentType, SourceLocation SourceLoc, ASTContext& Context) {
        const auto DT = dyn_cast<DecayedType>(CurrentType);

        if (!DT) {
            return NoneType::None;
        }

        RSTKind SimpleType;
        std::string Keyword;

        auto original = DT->getOriginalType();

        if (const auto AT = dyn_cast<ArrayType>(original.getTypePtr())) {

            if (AT->getSizeModifier() != ArrayType::ArraySizeModifier::Normal) {
                ReportError(Context, SourceLoc, "Unbounded array types are unsupported");
                return NoneType::None;
            }

            const auto CAT = dyn_cast<ConstantArrayType>(AT);
            SimpleType = RSTKind::Array;
            Keyword = original.getAsString();
        }
        else {
            ReportError(Context, SourceLoc, "Unsupported decayed type");
            return NoneType::None;
        }

        return std::make_pair(SimpleType, std::move(Keyword));
    }

    Optional<std::pair<RSTKind, std::string>> resolveTypeForPointer(QualType CurrentType, SourceLocation SourceLoc, ASTContext& Context) {
        auto PT = dyn_cast<PointerType>(CurrentType);

        if (!PT) {
            return NoneType::None;
        }

        RSTKind SimpleType = RSTKind::Pointer;
        std::string Keyword;

        auto Pointee = PT->getPointeeType();

        if (Pointee->isBuiltinType()) {
            Keyword = Pointee.getAsString();
        }
        else if (Pointee->isRecordType()) {
            auto record = Pointee->getAsCXXRecordDecl();

            /*if (!record->isPOD()) {
                ReportError(Context, SourceLoc, "Non POD pointer type found.");
                return NoneType::None;
            }*/

            Keyword = record->getQualifiedNameAsString();//getName();
        }
        else if (PT->isFunctionPointerType()) {
            SimpleType = RSTKind::FunctionPointer;
            Keyword = CurrentType.getAsString();
        }
        else {
            ReportError(Context, SourceLoc, "Unsupported pointer type found");
            return NoneType::None;
        }

        return std::make_pair(SimpleType, std::move(Keyword));
    }

    //TODO(vim): Decide how this shoudl be handled
    //TODO(vim): How and if the quals should be discarded
    Optional<std::pair<RSTKind, std::string>> resolveTypeForTypedef(QualType CurrentType, SourceLocation SourceLoc, ASTContext& Context) {
        
        auto Ret = getSimplifiedType(CurrentType.getDesugaredType(Context), SourceLoc, Context);
        
        if (Ret.hasValue()) {
            auto  ResolvedType = Ret.getValue();
            ResolvedType.second = CurrentType.getAsString();

            return ResolvedType;
        }

        return Ret;
    }

    Optional<std::pair<RSTKind, std::string>> resolveTypeForRecord(QualType CurrentType, SourceLocation SourceLoc, ASTContext& Context) {
        auto RT = CurrentType.getTypePtr()->getAsCXXRecordDecl();

        if (!RT) { return NoneType::None; }

        RSTKind SimpleType = RSTKind::Struct;
        std::string Keyword;

        if (RT->isPOD())
            Keyword = RT->getQualifiedNameAsString();// getName();
        else {
            ReportError(Context, SourceLoc, "Unsupported Value return type found");
            return NoneType::None;
        }

        return std::make_pair(SimpleType, std::move(Keyword));
    }

    Optional<std::pair<RSTKind, std::string>> resolveTypeForReference(QualType CurrentType, SourceLocation SourceLoc, ASTContext& Context) {
        auto RT = dyn_cast<ReferenceType>(CurrentType);

        if (!RT) { return NoneType::None; }

        RSTKind SimpleType;
        std::string Keyword;

        auto Pointee = RT->getPointeeType();

        if (Pointee->isRecordType() &&
            Pointee->getAsCXXRecordDecl()->getDeclName().getAsString() == "basic_string" &&
            Pointee->getAsCXXRecordDecl()->getEnclosingNamespaceContext()->isStdNamespace()) {
            Keyword = "char";
            SimpleType = RSTKind::String;
        }
        else {
            ReportError(Context, SourceLoc, "Unsupported reference type. Use a pointer instead");
            return NoneType::None;
        }

        return std::make_pair(SimpleType, Keyword);
    }

    Optional<std::pair<RSTKind, std::string>> getSimplifiedType(QualType CurrentType, SourceLocation SourceLoc, ASTContext& Context) {

        switch (CurrentType.getTypePtr()->getTypeClass())
        {
        case Type::TypeClass::Builtin:
            return resolveTypeForBuiltin(CurrentType, SourceLoc, Context);

        case Type::TypeClass::Decayed:
            return resolveTypeForDecayed(CurrentType, SourceLoc, Context);

        case Type::TypeClass::Pointer:
            return resolveTypeForPointer(CurrentType, SourceLoc, Context);

        case Type::TypeClass::Typedef:
            return resolveTypeForTypedef(CurrentType, SourceLoc, Context);

        case Type::TypeClass::Record:
            return resolveTypeForRecord(CurrentType, SourceLoc, Context);

        case Type::TypeClass::LValueReference:
            return resolveTypeForReference(CurrentType, SourceLoc, Context);

        default:
            ReportError(Context, SourceLoc, "Unsupported type found");
            break;
        }

        return llvm::NoneType::None;
    }

     Optional<RestrictedSimplifiedType>  processReturnType(QualType Type, clang::SourceLocation SourceLoc, ASTContext& Context) {

         auto Result = getSimplifiedType(Type, SourceLoc, Context);

         if (!Result.hasValue()) { return NoneType::None; }

         auto SimpleType = Result.getValue();

         if (SimpleType.first == RSTKind::Array || SimpleType.first == RSTKind::Pointer || SimpleType.first == RSTKind::String) {
             ReportNote(Context, SourceLoc, "This is a pointer type, which has several memory ownership implications");
         }

         if (SimpleType.first == RSTKind::Struct) {
             ReportNote(Context, SourceLoc, "Returning structs/classes may not be supported in other langauges");
         }

         return RestrictedSimplifiedType{ SimpleType.second, Type.getQualifiers().getAsString(), RestrictedSimplifiedType::getTypeName(SimpleType.first), "" };
     }

    Optional<RestrictedSimplifiedType> processParamType(QualType Type, std::string Name, SourceLocation SourceLoc, ASTContext& Context) {
        
        auto Result = getSimplifiedType(Type, SourceLoc, Context);

        if (!Result.hasValue()) { return NoneType::None; }

        auto SimpleType = Result.getValue();

        return RestrictedSimplifiedType{ SimpleType.second, Type.getQualifiers().getAsString(), RestrictedSimplifiedType::getTypeName(SimpleType.first), std::move(Name) };
    }
}
