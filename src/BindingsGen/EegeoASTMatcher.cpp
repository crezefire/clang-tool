#include "EegeoASTMatcher.h"

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

constexpr auto tab = "\t";
constexpr auto endl = "\n";


namespace Eegeo {
    void MatchProcessor::run(const clang::ast_matchers::MatchFinder::MatchResult &Result) {

        const auto *classTree = Result.Nodes.getNodeAs<clang::CXXRecordDecl>(classBindName);

        const auto getFileName = [](auto* ptr) {
            std::string str = ptr->getASTContext().getSourceManager().getFilename(ptr->getLocation());
            return RemovePath(str);
        };

        if (classTree && ClassName.empty()) {

            if (SourceFile != getFileName(classTree))
                return;

            QualifiedClassName = classTree->getQualifiedNameAsString();

            auto pos = QualifiedClassName.find_last_of("::");
            if (pos != std::string::npos)
                QualifiedClassName = QualifiedClassName.substr(0, pos + 1);

            ClassName = classTree->getNameAsString();
        }

        if (const auto *methodTree = Result.Nodes.getNodeAs<clang::CXXMethodDecl>(methodBindName)) {

            if (SourceFile != getFileName(methodTree))
                return;

            if (clang::isa<clang::CXXConstructorDecl>(methodTree)) { return; }

            if (methodTree->getAccess() != clang::AS_public) { return; }

            auto returnType = methodTree->getReturnType();

            auto RT = Eegeo::processReturnType(returnType, methodTree->getLocation(), methodTree->getASTContext());

            if (!RT.hasValue()) { return; }

            auto retType = RT.getValue();

            //OS << methodTree->getDeclName() << " ";

            //***********************

            std::vector<Eegeo::RestrictedSimplifiedType> params;

            auto numParams = methodTree->getNumParams();

            for (auto i = 0; i < numParams; ++i) {

                auto PD = methodTree->getParamDecl(i);
                auto Result = Eegeo::processParamType(PD->getType(), PD->getDeclName().getAsString(), PD->getLocation(), methodTree->getASTContext());

                if (!Result.hasValue()) { continue; }

                auto paramType = Result.getValue();

                params.emplace_back(std::move(paramType));
            }

            Methods.emplace_back(Eegeo::InterfaceMethod{ std::move(methodTree->getDeclName().getAsString()) , std::move(retType), std::move(params) });

            //OS << endl << endl;
        }
    }

        void MatchProcessor::Print() {
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

            const auto PrintType = [&](const Eegeo::RestrictedSimplifiedType& type) {
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
                PrintField("modules");
                Dump << "[";
                NewLine();
                {
                    Dump << "{";
                    NewLine();

                    {
                        PrintField("name");
                        PrintString(ClassName);
                        Dump << ",";
                        NextLine();

                        PrintField("file-name");
                        PrintString(SourceFile);
                        Dump << ",";
                        NextLine();

                        PrintField("namespace");
                        PrintString(QualifiedClassName);
                        Dump << ",";
                        NextLine();

                        PrintField("methods");
                        Dump << "[";
                        NewLine();

                        {
                            for (auto m = 0; m < Methods.size(); ++m) {
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
    }
