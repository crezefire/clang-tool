#include "EegeoASTMatcher.h"

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

constexpr auto tab = "\t";
constexpr auto endl = "\n";

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

        const auto *ClassTree = Result.Nodes.getNodeAs<clang::CXXRecordDecl>(classBindName);

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

        if (const auto *MethodTree = Result.Nodes.getNodeAs<clang::CXXMethodDecl>(methodBindName)) {

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

            CurrentModule.Methods.emplace_back(Eegeo::InterfaceMethod{ std::move(MethodTree->getDeclName().getAsString()) , std::move(RetType), std::move(Params) });
        }
    }

    const auto printField = [&](const auto& field) {
        llvm::outs() << "\"" << field << "\" : ";
    };

    const auto printString = [&](const auto& str) {
        llvm::outs() << "\"" << str << "\"";
    };

    const auto printType = [&](const Eegeo::RestrictedSimplifiedType& type) {
        llvm::outs() << "{ ";

        if (!type.Name.empty()) {
            printField("name");
            printString(type.Name);
            llvm::outs() << ", ";
        }

        printField("keyword");
        printString(type.Keyword);
        llvm::outs() << ", ";

        printField("type");
        printString(type.TypeName);

        llvm::outs() << "}";
    };

    void EegeoASTMatcher::DumpJSON() {
        auto NumTabs = 0;

        const auto tabify = [&]() {
            for (auto i = 0; i < NumTabs; ++i) {
                OStream << tab;
            }
        };

        const auto oldLine = [&]() {
            OStream << endl;
            --NumTabs;
            tabify();
        };

        const auto newLine = [&]() {
            OStream << endl;
            ++NumTabs;
            tabify();
        };

        const auto nextLine = [&]() {
            OStream << endl;
            tabify();
        };

        const auto enterScope = [&](auto Enter, auto Exit) {
            OStream << Enter;
            newLine();

            struct Pop {
                ~Pop() {
                    oldLine();
                    OStream << Exit;
                }
            };

            return Pop;
        };

        OStream << "{";
        newLine();

        {
            printField("modules");
            OStream << "[";
            newLine();
            {
                OStream << "{";
                newLine();

                {
                    printField("name");
                    printString(Modules[0].ClassName);
                    OStream << ",";
                    nextLine();

                    printField("file-name");
                    printString(SourceFile);
                    OStream << ",";
                    nextLine();

                    printField("namespace");
                    printString(Modules[0].Namespace);
                    OStream << ",";
                    nextLine();

                    printField("methods");
                    OStream << "[";
                    newLine();

                    {
                        for (auto m = 0; m < Modules[0].Methods.size(); ++m) {
                            auto& i = Modules[0].Methods[m];
                            OStream << "{";
                            newLine();

                            printField("name");
                            printString(i.Name);
                            OStream << ",";
                            nextLine();

                            printField("return-type");
                            printType(i.ReturnType);

                            OStream << ",";
                            nextLine();

                            printField("params");
                            OStream << "[";
                            newLine();

                            {
                                for (auto p = 0; p < i.Params.size(); ++p) {
                                    printType(i.Params[p]);

                                    if (p != i.Params.size() - 1) {
                                        OStream << ",";
                                        nextLine();
                                    }
                                }
                            }

                            oldLine();
                            OStream << "]";

                            oldLine();
                            OStream << "}";

                            if (m != Modules[0].Methods.size() - 1) {
                                OStream << ",";
                            }

                            nextLine();
                        }
                    }

                    oldLine();
                    OStream << "]";
                }

                oldLine();
                OStream << "}";
            }

            oldLine();
            OStream << "]";
        }

        oldLine();
        OStream << "}";
    }
}