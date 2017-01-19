// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

//For AST matching
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

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

constexpr auto endl = '\n';
constexpr auto tab = '\t';

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

        static StringRef GetTypeName(types param) {
            switch (param) {
            case Void:
                return "Void";

            case Floating:
                return "Floating";

            case Integral:
                return "Integral";

            case Boolean:
                return "Boolean";

            case Pointer:
                return "Pointer";

            case FunctionPointer:
                return "FunctionPointer";

            case Array:
                return "Array";

            case Struct:
                return "Struct";

            case String:
                return "String";

            default:
                //TODO(vim): Error
                return "";
            }
        }
    };

    struct Method {
        std::string Name;
        Type ReturnType;
        std::vector<Type> Params;
    };

    std::tuple<bool, std::string, Eegeo::Type::types> CheckBuiltinType(QualType CanonicalType, ASTContext& Context) {
        std::string keyword;
        Eegeo::Type::types returnTypeName = Eegeo::Type::Void;

        bool typeFound = false;

        if (!CanonicalType->isBuiltinType()) {
            return { typeFound, keyword, returnTypeName };
        }

        typeFound = true;

        keyword = CanonicalType.getAsString();

        auto BT = dyn_cast<BuiltinType>(CanonicalType);

        if (keyword == "_Bool") {
            keyword = "bool";
            returnTypeName = Eegeo::Type::Boolean;
        }
        else if (CanonicalType->isVoidType()) {
            returnTypeName = Eegeo::Type::Void;
        }
        else if (CanonicalType->isIntegralType(Context)) {
            returnTypeName = Eegeo::Type::Integral;
        }
        else if (CanonicalType->isFloatingType()) {
            auto kind = BT->getKind();

            //For future use case
            if (kind == BuiltinType::Kind::Double || kind == BuiltinType::Kind::LongDouble)
                returnTypeName = Eegeo::Type::Floating;
            else
                returnTypeName = Eegeo::Type::Floating;
        }
        else {
            //TODO(vim): Error
            llvm::errs() << "Unsupported Builtin Type Found!!" << endl;
            typeFound = false;
        }

        return { typeFound, keyword, returnTypeName };
    }

    Type ProcessReturnType(QualType type, ASTContext& context) {
        auto& OS = llvm::errs();

        auto canonType = type.getDesugaredType(context).getCanonicalType();
        canonType.removeLocalCVRQualifiers(Qualifiers::CVRMask);

        auto unqual = type.getTypePtr();

        std::string keyword;

        Eegeo::Type::types returnTypeName = Eegeo::Type::Void;

        if (unqual->isBuiltinType()) {

            keyword = canonType.getAsString();

            auto BT = dyn_cast<BuiltinType>(canonType);

            if (keyword == "_Bool") {
                keyword = "bool";
                returnTypeName = Eegeo::Type::Boolean;
            }
            else if (unqual->isVoidType()) {
                returnTypeName = Eegeo::Type::Void;
            }
            else if (unqual->isIntegralType(context)) {
                returnTypeName = Eegeo::Type::Integral;
            }
            else if (unqual->isFloatingType()) {
                auto kind = BT->getKind();

                //For future use case
                if (kind == BuiltinType::Kind::Double || kind == BuiltinType::Kind::LongDouble)
                    returnTypeName = Eegeo::Type::Floating;
                else
                    returnTypeName = Eegeo::Type::Floating;
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

            returnTypeName = Eegeo::Type::Pointer;

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

            returnTypeName = Eegeo::Type::Struct;

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

        return { std::move(keyword), Eegeo::Type::GetTypeName(returnTypeName), "" };
    }

    Type ProcessParamType(QualType type, ASTContext& context) {
        auto& OS = llvm::errs();

        auto canonType = type.getDesugaredType(context).getCanonicalType();
        canonType.removeLocalCVRQualifiers(Qualifiers::CVRMask);

        auto unqual = type.getTypePtr();

        std::string keyword;
        std::string name;

        Eegeo::Type::types returnTypeName = Eegeo::Type::Void;

        if (unqual->isBuiltinType()) {

            keyword = canonType.getAsString();

            auto BT = dyn_cast<BuiltinType>(canonType);

            if (keyword == "_Bool") {
                keyword = "bool";
                returnTypeName = Eegeo::Type::Boolean;
            }
            else if (unqual->isVoidType()) {
                returnTypeName = Eegeo::Type::Void;
                //TODO(vim): Error
            }
            else if (unqual->isIntegralType(context)) {
                returnTypeName = Eegeo::Type::Integral;
            }
            else if (unqual->isFloatingType()) {
                auto kind = BT->getKind();

                //For future use case
                if (kind == BuiltinType::Kind::Double || kind == BuiltinType::Kind::LongDouble)
                    returnTypeName = Eegeo::Type::Floating;
                else
                    returnTypeName = Eegeo::Type::Floating;
            }
            else {
                //TODO(vim): Error
                OS << "Unsupported Builtin Type Found!!" << endl;
            }
        }
        else if (unqual->isFunctionPointerType()) {
            returnTypeName = Eegeo::Type::FunctionPointer;
            keyword = canonType.getAsString();
        }
        else if (unqual->isPointerType()) {

            if (const auto DT = dyn_cast<DecayedType>(unqual)) {
                auto original = DT->getOriginalType();

                if (const auto AT = dyn_cast<ArrayType>(original.getTypePtr())) {
                    if (AT->getSizeModifier() == ArrayType::ArraySizeModifier::Normal) {
                        const auto CAT = dyn_cast<ConstantArrayType>(AT);
                        returnTypeName = Eegeo::Type::Array;
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

                returnTypeName = Eegeo::Type::Pointer;

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

            returnTypeName = Eegeo::Type::Struct;

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
                returnTypeName = Eegeo::Type::String;
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

        return { std::move(keyword), Eegeo::Type::GetTypeName(returnTypeName), std::move(name) };
    }
}

class MatchProcessor : public MatchFinder::MatchCallback {
    raw_ostream& OS{ llvm::errs() };
    raw_ostream& Dump{ llvm::outs() };
    SmallVector<SmallVector<SmallString<20>, 10>, 10> MemberFunctions;
    StringRef SourceFile;

    SmallString<20> ClassName;
    SmallVector<Eegeo::Method, 10> Methods;
public:
    MatchProcessor(StringRef ref)
        : SourceFile(std::move(ref)) {}

    void run(const MatchFinder::MatchResult &Result) override {

        const auto *classTree = Result.Nodes.getNodeAs<clang::CXXRecordDecl>(classBindName);

        const auto getFileName = [](auto* ptr) {
            std::string str = ptr->getASTContext().getSourceManager().getFilename(ptr->getLocation());
            return RemovePath(str);
        };

        if (classTree && ClassName.empty()) {

            if (SourceFile != getFileName(classTree))
                return;

            ClassName = classTree->getNameAsString();
        }

        if (const auto *methodTree = Result.Nodes.getNodeAs<clang::CXXMethodDecl>(methodBindName)) {

            if (SourceFile != getFileName(methodTree))
                return;

            if (isa<clang::CXXConstructorDecl>(methodTree)) { return; }

            if (methodTree->getAccess() != AS_public) { return; }

            auto returnType = methodTree->getReturnType();

            auto retType = Eegeo::ProcessReturnType(returnType, methodTree->getASTContext());

            //OS << methodTree->getDeclName() << " ";

            //***********************

            std::vector<Eegeo::Type> params;

            auto numParams = methodTree->getNumParams();

            for (auto i = 0; i < numParams; ++i) {
                auto paramType = Eegeo::ProcessParamType(methodTree->getParamDecl(i)->getType(), methodTree->getASTContext());
                paramType.Name = methodTree->getParamDecl(i)->getDeclName().getAsString();
                //OS << "\t" << paramType.Keyword << " [" << paramType.TypeName << "] " << methodTree->getParamDecl(i)->getDeclName() << endl;// paramType.Name << endl;

                params.emplace_back(std::move(paramType));
            }

            Methods.emplace_back(Eegeo::Method{ std::move(methodTree->getDeclName().getAsString()) , std::move(retType), std::move(params) });

            //OS << endl << endl;
        }

    }

    void Print() {
        auto NumTabs = 0;

        const auto Tabify = [&]() {
            for (auto i = 0; i < NumTabs; ++i) {
                Dump << tab;
            }
        };

        const auto OldLine = [&]() {
            Dump << endl;
            --NumTabs;
            Tabify();
        };

        const auto NewLine = [&]() {
            Dump << endl;
            ++NumTabs;
            Tabify();
        };

        const auto NextLine = [&]() {
            Dump << endl;
            Tabify();
        };

        const auto PrintField = [&](const auto& field) {
            Dump << "\"" << field << "\" : ";
        };

        const auto PrintString = [&](const auto& str) {
            Dump << "\"" << str << "\"";
        };

        const auto PrintType = [&](const Eegeo::Type& type) {
            Dump << "{ ";

            if (!type.Name.empty()) {
                PrintField("name");
                PrintString(type.Name);
                Dump << ", ";
            }

            PrintField("keyword");
            PrintString(type.Keyword);
            Dump << ", ";

            PrintField("type");
            PrintString(type.TypeName);

            Dump << "}";
        };
        
        Dump << "{";
        NewLine();

        {
            // Dump << "\"modules\" : [";
            PrintField("modules");
            Dump << "[";
            NewLine();
            {
                Dump << "{";
                NewLine();

                {
                    //Dump << "\"name\" : \"" << ClassName << "\",";
                    PrintField("name");
                    PrintString(ClassName);
                    Dump << ",";
                    NextLine();

                    //Dump << "\"methods\" : [";
                    PrintField("methods");
                    Dump << "[";
                    NewLine();
                    {
                        for (auto m = 0; m < Methods.size(); ++m) {
                        //for (auto& i : Methods) {
                            auto& i = Methods[m];
                            Dump << "{";
                            NewLine();

                            PrintField("name");
                            PrintString(i.Name);
                            Dump << ",";
                            NextLine();

                            PrintField("return-type");
                            PrintType(i.ReturnType);
                            
                            Dump << ",";
                            NextLine();
                            
                            PrintField("params");
                            Dump << "[";
                            NewLine();
                            
                            {
                                for (auto p = 0; p < i.Params.size(); ++p) {
                                    PrintType(i.Params[p]);
                                    
                                    if (p != i.Params.size() - 1) {
                                        Dump << ",";
                                        NextLine();
                                    }
                                }
                            }
                            
                            OldLine();
                            Dump << "]";

                            OldLine();
                            Dump << "}";

                            if (m != Methods.size() - 1) {
                                Dump << ",";
                            }

                            NextLine();
                        }
                    }

                    OldLine();
                    Dump << "]";
                }

                OldLine();
                Dump << "}";
            }

            OldLine();
            Dump << "]";
        }

        OldLine();
        Dump << "}";
    }
};

int main(int argc, const char **argv) {
    CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
    ClangTool Tool(OptionsParser.getCompilations(),
        OptionsParser.getSourcePathList());

    MatchProcessor Printer(RemovePath(OptionsParser.getSourcePathList()[0]));
    MatchFinder Finder;

    Finder.addMatcher(ClassDeclMatcher, &Printer);
    Finder.addMatcher(MemberFunctionMatcher, &Printer);

    auto ret = Tool.run(newFrontendActionFactory(&Finder).get());

    Printer.Print();

    //system("pause");

    return ret;
}
