#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/ASTConsumer.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {

class TypeInfoVisitor : public RecursiveASTVisitor<TypeInfoVisitor> {
public:
    explicit TypeInfoVisitor(ASTContext *Context) : Context(Context) {}

    bool VisitCXXRecordDecl(CXXRecordDecl *Decl) {
        if (!Decl->isThisDeclarationADefinition() || Decl->isImplicit())
            return true;

        // Базовые классы
        if (Decl->getNumBases() > 0) {
            llvm::outs() << Decl->getName() << " -> ";
            bool first = true;
            for (const auto &Base : Decl->bases()) {
                if (!first) llvm::outs() << ", ";
                const auto *BaseType = Base.getType()->getAsCXXRecordDecl();
                if (BaseType)
                    llvm::outs() << BaseType->getName();
                first = false;
            }
            llvm::outs() << "\n";
        } else {
            llvm::outs() << Decl->getName() << "\n";
        }

        // Поля
        llvm::outs() << "|_Fields\n";
        for (const FieldDecl *Field : Decl->fields()) {
            llvm::outs() << "| |_ " << Field->getName() << " (";
            Field->getType().print(llvm::outs(), Context->getPrintingPolicy());
            llvm::outs() << "|" << getAccessSpelling(Field->getAccess()) << ")\n";
        }

        // Методы
        llvm::outs() << "|\n|_Methods\n";
        for (const CXXMethodDecl *Method : Decl->methods()) {
            if (Method->isImplicit()) continue;

            llvm::outs() << "| |_ " << Method->getNameAsString() << " (";

            Method->getReturnType().print(llvm::outs(), Context->getPrintingPolicy());
            llvm::outs() << "()|" << getAccessSpelling(Method->getAccess());

            if (Method->isVirtual()) {
                llvm::outs() << "|virtual";
                if (Method->isPureVirtual())
                    llvm::outs() << "|pure";
            }

            if (Method->size_overridden_methods() > 0)
                llvm::outs() << "|override";

            llvm::outs() << ")\n";
        }

        llvm::outs() << "\n";
        return true;
    }

private:
    ASTContext *Context;

    static StringRef getAccessSpelling(AccessSpecifier AS) {
        switch (AS) {
            case AS_public: return "public";
            case AS_private: return "private";
            case AS_protected: return "protected";
            case AS_none: return "none";
        }
        return "none";
    }
};

class TypeInfoConsumer : public ASTConsumer {
public:
    explicit TypeInfoConsumer(ASTContext *Context) : Visitor(Context) {}
    void HandleTranslationUnit(ASTContext &Context) override {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }

private:
    TypeInfoVisitor Visitor;
};

class TypeInfoAction : public PluginASTAction {
protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) override {
        return std::make_unique<TypeInfoConsumer>(&CI.getASTContext());
    }

    bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &) override {
        return true;
    }
};

} // namespace

static FrontendPluginRegistry::Add<TypeInfoAction>
X("print-type-info", "Print fields, methods and bases of user-defined types");
