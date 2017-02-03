#include "EegeoTypeData.h"

// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;
using namespace llvm;

namespace Eegeo {
    StringRef RestrictedSimplifiedType::getTypeName(RSType Param) {
        switch (Param) {
        case RSType::Void:
            return "Void";

        case RSType::Floating:
            return "Floating";

        case RSType::Integral:
            return "Integral";

        case RSType::Boolean:
            return "Boolean";

        case RSType::Pointer:
            return "Pointer";

        case RSType::FunctionPointer:
            return "FunctionPointer";

        case RSType::Array:
            return "Array";

        case RSType::Struct:
            return "Struct";

        case RSType::String:
            return "String";

        default:
            //TODO(vim): Error
            return "";
        }
    }

    void ReportError(ASTContext& Context, SourceLocation SourceLoc, StringRef Message) {
        auto& Engine = Context.getDiagnostics();
        const auto ErrorID = Engine.getDiagnosticIDs()->getCustomDiagID(DiagnosticIDs::Error, Message);
        Engine.Report(SourceLoc, ErrorID);
    }

    Optional<std::pair<RSType, std::string>> resolveTypeForBuiltin(QualType CurrentType, SourceLocation SourceLoc, ASTContext& Context) {
        const auto BT = dyn_cast<BuiltinType>(CurrentType);

        if (!BT) {
            return NoneType::None;
        }

        CurrentType.removeLocalCVRQualifiers(Qualifiers::CVRMask);

        auto Keyword = CurrentType.getAsString();
        const auto Pos = Keyword.find("_Bool");

        RSType SimpleType;

        if (Pos != std::string::npos) {
            Keyword.replace(Pos, 5, "bool");
            SimpleType = RSType::Boolean;
        }
        else if (CurrentType->isVoidType()) {
            SimpleType = RSType::Void;
        }
        else if (CurrentType->isIntegralType(Context)) {
            SimpleType = RSType::Integral;
        }
        else if (CurrentType->isFloatingType()) {
            auto Kind = BT->getKind();

            //For future use case
            if (Kind == BuiltinType::Kind::Double || Kind == BuiltinType::Kind::LongDouble)
                SimpleType = RSType::Floating;
            else
                SimpleType = RSType::Floating;
        }
        else {
            ReportError(Context, SourceLoc, "Unsupported builtin type found");
            return NoneType::None;
        }

        return std::make_pair(SimpleType, std::move(Keyword));
    }

    Optional<std::pair<RSType, std::string>> resolveTypeForDecayed(QualType CurrentType, SourceLocation SourceLoc, ASTContext& Context) {
        const auto DT = dyn_cast<DecayedType>(CurrentType);

        if (!DT) {
            return NoneType::None;
        }

        RSType SimpleType;
        std::string Keyword;

        auto original = DT->getOriginalType();

        if (const auto AT = dyn_cast<ArrayType>(original.getTypePtr())) {

            if (AT->getSizeModifier() != ArrayType::ArraySizeModifier::Normal) {
                ReportError(Context, SourceLoc, "Unbounded array types are unsupported");
                return NoneType::None;
            }

            const auto CAT = dyn_cast<ConstantArrayType>(AT);
            SimpleType = RSType::Array;
            Keyword = original.getAsString();
        }
        else {
            ReportError(Context, SourceLoc, "Unsupported decayed type");
            return NoneType::None;
        }

        return std::make_pair(SimpleType, std::move(Keyword));
    }

    Optional<std::pair<RSType, std::string>> resolveTypeForPointer(QualType CurrentType, SourceLocation SourceLoc, ASTContext& Context) {
        auto PT = dyn_cast<PointerType>(CurrentType);

        if (!PT) {
            return NoneType::None;
        }

        RSType SimpleType = RSType::Pointer;
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
            SimpleType = RSType::FunctionPointer;
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
    Optional<std::pair<RSType, std::string>> resolveTypeForTypedef(QualType CurrentType, SourceLocation SourceLoc, ASTContext& Context) {
        
        auto Ret = getSimplifiedType(CurrentType.getDesugaredType(Context), SourceLoc, Context);
        
        if (Ret.hasValue()) {
            auto  ResolvedType = Ret.getValue();
            ResolvedType.second = CurrentType.getAsString();

            return ResolvedType;
        }

        return Ret;
    }

    Optional<std::pair<RSType, std::string>> resolveTypeForRecord(QualType CurrentType, SourceLocation SourceLoc, ASTContext& Context) {
        auto RT = CurrentType.getTypePtr()->getAsCXXRecordDecl();

        if (!RT) {
            return NoneType::None;
        }

        RSType SimpleType = RSType::Struct;
        std::string Keyword;

        if (RT->isPOD())
            Keyword = RT->getQualifiedNameAsString();// getName();
        else {
            ReportError(Context, SourceLoc, "Unsupported Value return type found");
            return NoneType::None;
        }

        return std::make_pair(SimpleType, std::move(Keyword));
    }

    Optional<std::pair<RSType, std::string>> resolveTypeForReference(QualType CurrentType, SourceLocation SourceLoc, ASTContext& Context) {
        auto RT = dyn_cast<ReferenceType>(CurrentType);

        if (!RT) {
            return NoneType::None;
        }

        RSType SimpleType;
        std::string Keyword;

        auto Pointee = RT->getPointeeType();

        if (Pointee->isRecordType() &&
            Pointee->getAsCXXRecordDecl()->getDeclName().getAsString() == "basic_string") {
            Keyword = "char";
            SimpleType = RSType::String;
        }
        else {
            ReportError(Context, SourceLoc, "Unsupported reference type. Use a pointer instead");
            return NoneType::None;
        }

        return std::make_pair(SimpleType, Keyword);
    }

    Optional<std::pair<RSType, std::string>> getSimplifiedType(QualType CurrentType, SourceLocation SourceLoc, ASTContext& Context) {

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

        return {llvm::NoneType::None};
    }

    RestrictedSimplifiedType ProcessReturnType(QualType type, ASTContext& context) {
        auto& OS = llvm::errs();

        auto canonType = type.getDesugaredType(context).getCanonicalType();
        canonType.removeLocalCVRQualifiers(Qualifiers::CVRMask);

        auto unqual = type.getTypePtr();

        std::string keyword;

        RSType returnTypeName = RSType::Void;

        OS << type.getTypePtr()->getTypeClass() << " " << unqual->getTypeClassName() << "\n";

        if (unqual->isBuiltinType()) {

            keyword = canonType.getAsString();

            auto BT = dyn_cast<BuiltinType>(canonType);

            if (keyword == "_Bool") {
                keyword = "bool";
                returnTypeName = RSType::Boolean;
            }
            else if (unqual->isVoidType()) {
                returnTypeName = RSType::Void;
            }
            else if (unqual->isIntegralType(context)) {
                returnTypeName = RSType::Integral;
            }
            else if (unqual->isFloatingType()) {
                auto kind = BT->getKind();

                //For future use case
                if (kind == BuiltinType::Kind::Double || kind == BuiltinType::Kind::LongDouble)
                    returnTypeName = RSType::Floating;
                else
                    returnTypeName = RSType::Floating;
            }
            else {
                //TODO(vim): Error
                OS << "Unsupported Builtin Type Found!!" << endl;
            }
        }
        else if (unqual->isFunctionPointerType()) {
            keyword = canonType.getAsString();
        }
        else if (unqual->isPointerType()) {
            auto PT = dyn_cast<PointerType>(canonType);

            returnTypeName = RSType::Pointer;

            auto pointee = PT->getPointeeType();

            if (pointee->isBuiltinType()) {
                keyword = pointee.getAsString();
            }
            else if (pointee->isRecordType()) {
                auto record = pointee->getAsCXXRecordDecl();

                //if (!record->isPOD()) {
                //    //TODO(vim): Error
                //    OS << "Non POD Pointer Type Found!!" << endl;
                //}

                keyword = record->getName();
            }
            else {
                //TODO(vim): Error
                OS << "Unsupported Pointer Type Found!!" << endl;
            }
        }
        else if (unqual->isRecordType()) {
            auto RT = unqual->getAsCXXRecordDecl();

            returnTypeName = RSType::Struct;

            if (RT->isPOD())
                keyword = RT->getName();
            else {
                //TODO(vim): Error
                OS << "Unsupported Value return type found!!" << endl;
            }
        }
        else {
            //TODO(vim): Error
            OS << "Unsupported Param Type Found!!" << endl;
        }

        return { std::move(keyword), Eegeo::RestrictedSimplifiedType::getTypeName(returnTypeName), "" };
    }

    RestrictedSimplifiedType ProcessParamType(QualType type, ASTContext& context) {
        auto& OS = llvm::errs();

        auto canonType = type.getDesugaredType(context).getCanonicalType();
        canonType.removeLocalCVRQualifiers(Qualifiers::CVRMask);

        auto unqual = type.getTypePtr();

        std::string keyword;
        std::string name;

        RSType returnTypeName = RSType::Void;

        OS << type.getTypePtr()->getTypeClass() << " " << unqual->getTypeClassName() << "\n";

        if (unqual->isBuiltinType()) {

            keyword = canonType.getAsString();

            auto BT = dyn_cast<BuiltinType>(canonType);

            if (keyword == "_Bool") {
                keyword = "bool";
                returnTypeName = RSType::Boolean;
            }
            else if (unqual->isVoidType()) {
                returnTypeName = RSType::Void;
                //TODO(vim): Error
            }
            else if (unqual->isIntegralType(context)) {
                returnTypeName = RSType::Integral;
            }
            else if (unqual->isFloatingType()) {
                auto kind = BT->getKind();

                //For future use case
                if (kind == BuiltinType::Kind::Double || kind == BuiltinType::Kind::LongDouble)
                    returnTypeName = RSType::Floating;
                else
                    returnTypeName = RSType::Floating;
            }
            else {
                //TODO(vim): Error
                OS << "Unsupported Builtin Type Found!!" << endl;
            }
        }
        else if (unqual->isFunctionPointerType()) {
            returnTypeName = RSType::FunctionPointer;
            keyword = canonType.getAsString();
        }
        else if (unqual->isPointerType()) {

            if (const auto DT = dyn_cast<DecayedType>(unqual)) {
                auto original = DT->getOriginalType();

                if (const auto AT = dyn_cast<ArrayType>(original.getTypePtr())) {
                    if (AT->getSizeModifier() == ArrayType::ArraySizeModifier::Normal) {
                        const auto CAT = dyn_cast<ConstantArrayType>(AT);
                        returnTypeName = RSType::Array;
                        keyword = original.getAsString();
                    }
                    else {
                        //TODO(vim): Error
                        OS << "Unbounded array types are not supported!!";
                    }
                }

            }
            else {

                auto PT = dyn_cast<PointerType>(canonType);

                returnTypeName = RSType::Pointer;

                auto pointee = PT->getPointeeType();

                if (pointee->isBuiltinType()) {
                    keyword = pointee.getAsString();
                }
                else if (pointee->isRecordType()) {
                    auto record = pointee->getAsCXXRecordDecl();

                    if (!record->isPOD()) {
                        //TODO(vim): Error
                        OS << "Non POD Pointer Type Found!!" << endl;
                    }

                    keyword = record->getName();
                }
                else {
                    //TODO(vim): Error
                    OS << "Unsupported Pointer Type Found!!" << endl;
                }
            }
        }
        else if (unqual->isRecordType()) {
            auto RT = unqual->getAsCXXRecordDecl();

            returnTypeName = RSType::Struct;

            if (RT->isPOD())
                keyword = RT->getName();
            else {
                //TODO(vim): Error Example: std::string*
                OS << "Unsupported Value return type found!!" << endl;
            }
        }
        else if (unqual->isReferenceType()) {
            auto RT = dyn_cast<ReferenceType>(unqual);
            auto pointee = RT->getPointeeType();

            if (pointee->isRecordType() && pointee->getAsCXXRecordDecl()->getDeclName().getAsString() == "basic_string") {
                keyword = "char";
                returnTypeName = RSType::String;
            }
            else {
                OS << "Unsupported reference type. Use a pointer instead!" << endl;
            }

        }
        else {
            //TODO(vim): Error
            OS << "Unsupported Return Type Found!!" << endl;
            type.dump();
        }

        return { std::move(keyword), Eegeo::RestrictedSimplifiedType::getTypeName(returnTypeName), std::move(name) };
    }
}
