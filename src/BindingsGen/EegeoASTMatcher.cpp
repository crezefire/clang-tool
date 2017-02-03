#include "EegeoASTMatcher.h"

#include "EegeoTypeData.h"

constexpr auto Tab = "\t";
constexpr auto Endl = "\n";

using namespace clang;
using namespace llvm;


namespace Eegeo {
    EegeoASTMatcher::EegeoASTMatcher(llvm::StringRef Ref) :
        SourceFile(Ref)
    {}

    Optional<int> EegeoASTMatcher::getModuleIndexFromClassName(StringRef ClassName) {
        int Index = 0;

        for (auto& CurrModule : Modules) {
            if (CurrModule.ClassName == ClassName) {
                return Index;
            }

            ++Index;
        }

        return NoneType::None;
    }

    void EegeoASTMatcher::run(const clang::ast_matchers::MatchFinder::MatchResult &Result) {

        const auto *ClassTree = Result.Nodes.getNodeAs<clang::CXXRecordDecl>(ClassBindName);

        const auto getFileName = [](auto* Ptr) {
            return removePath(Ptr->getASTContext().getSourceManager().getFilename(Ptr->getLocation()));
        };

        if (ClassTree) {

            if (SourceFile != getFileName(ClassTree))
                return;

            auto QualifiedClassName = ClassTree->getQualifiedNameAsString();

            auto pos = QualifiedClassName.find_last_of("::");
            if (pos != std::string::npos)
                QualifiedClassName = QualifiedClassName.substr(0, pos + 1);

            auto ClassName = ClassTree->getNameAsString();

            Modules.push_back({ std::move(ClassName), std::move(QualifiedClassName), {} });
        }

        if (const auto *MethodTree = Result.Nodes.getNodeAs<clang::CXXMethodDecl>(MethodBindName)) {

            if (SourceFile != getFileName(MethodTree)) { return; }
            if (clang::isa<clang::CXXConstructorDecl>(MethodTree)) { return; }
            if (MethodTree->getAccess() != clang::AS_public) { return; }

            auto Result = getModuleIndexFromClassName(MethodTree->getParent()->getName());
            if (!Result) { return; }

            auto RT = processReturnType(MethodTree->getReturnType(), MethodTree->getLocation(), MethodTree->getASTContext());
            if (!RT.hasValue()) { return; }

            auto ModuleIndex = Result.getValue();
            auto& CurrentModule = Modules[ModuleIndex];

            auto& RetType = RT.getValue();

            std::vector<Eegeo::RestrictedSimplifiedType> Params;

            auto NumParams = MethodTree->getNumParams();

            for (auto i = 0; i < NumParams; ++i) {

                auto PD = MethodTree->getParamDecl(i);
                auto Result = Eegeo::processParamType(PD->getType(), PD->getDeclName().getAsString(), PD->getLocation(), MethodTree->getASTContext());

                if (!Result.hasValue()) { continue; }

                auto paramType = Result.getValue();

                Params.emplace_back(std::move(paramType));
            }

            

            CurrentModule.Methods.emplace_back(Eegeo::InterfaceMethod{ std::move(MethodTree->getDeclName().getAsString()), std::move(RetType), std::move(Params), MethodTree->isStatic() });
        }
    }

    const auto printField = [&](const auto& field) {
        llvm::outs() << "\"" << field << "\" : ";
    };

    const auto printString = [&](const auto& str) {
        llvm::outs() << "\"" << str << "\"";
    };

    const auto printType = [&](const Eegeo::RestrictedSimplifiedType& Type) {
        llvm::outs() << "{ ";

        if (!Type.Name.empty()) {
            printField("name");
            printString(Type.Name);
            llvm::outs() << ", ";
        }

        printField("qualifiers");
        printString(Type.Qualifiers);
        llvm::outs() << ", ";

        printField("keyword");
        printString(Type.Keyword);
        llvm::outs() << ", ";

        printField("type");
        printString(Type.TypeName);

        llvm::outs() << "}";
    };

    //TODO(vim): I know
    void EegeoASTMatcher::DumpJSON() {
        auto NumTabs = 0;

        const auto tabify = [&]() {
            for (auto i = 0; i < NumTabs; ++i) {
                OStream << Tab;
            }
        };

        const auto oldLine = [&]() {
            OStream << Endl;
            --NumTabs;
            tabify();
        };

        const auto newLine = [&]() {
            OStream << Endl;
            ++NumTabs;
            tabify();
        };

        const auto nextLine = [&]() {
            OStream << Endl;
            tabify();
        };

        const auto nextField = [&] {
            OStream << ",";
            nextLine();
        };

        const auto pushObject = [&]() {
            OStream << "{";
            newLine();
        };

        const auto popObject = [&]() {
            oldLine();
            OStream << "}";
        };

        const auto pushArray = [&]() {
            OStream << "[";
            newLine();
        };

        const auto popArray = [&]() {
            oldLine();
            OStream << "]";
        };

        pushObject();

        {
            printField("modules");
            pushArray();

            {
                for (int ModuleIndex = 0, End = Modules.size(); ModuleIndex < End; ++ModuleIndex) {
                    auto& CurrentModule = Modules[ModuleIndex];
                    pushObject();
                
                    printField("name");
                    printString(CurrentModule.ClassName);
                    nextField();

                    printField("file-name");
                    printString(SourceFile);
                    nextField();

                    printField("namespace");
                    printString(CurrentModule.Namespace);
                    nextField();

                    printField("methods");
                    pushArray();
                    {
                        for (auto m = 0; m < CurrentModule.Methods.size(); ++m) {
                            auto& CurrentMethod = CurrentModule.Methods[m];

                            pushObject();

                            printField("name");
                            printString(CurrentMethod.Name);
                            nextField();

                            printField("is-static");
                            printString(CurrentMethod.IsStatic ? "true" : "false");
                            nextField();

                            printField("return-type");
                            printType(CurrentMethod.ReturnType);
                            nextField();

                            printField("params");
                            pushArray();

                            {
                                for (auto p = 0; p < CurrentMethod.Params.size(); ++p) {
                                    printType(CurrentMethod.Params[p]);

                                    if (p != CurrentMethod.Params.size() - 1) {
                                        nextField();
                                    }
                                }
                            }

                            popArray();
                            popObject();

                            if (m != CurrentModule.Methods.size() - 1) {
                                OStream << ",";
                            }

                            nextLine();
                        }
                    }

                    popArray();
                    popObject();

                    if (ModuleIndex != End - 1) {
                        OStream << ",";
                    }

                    nextLine();
                }
            }

            popArray();
        }

        popObject();
    }
}