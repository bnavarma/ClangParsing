// ClangParsing.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <memory>

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Parse/ParseAST.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/TextDiagnostic.h>
#include <clang/Frontend/TextDiagnosticBuffer.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <llvm/Support/TargetSelect.h>

#include "UndefSymbols.h"
#include "PreprocessorMacros.h"

using namespace clang;
using namespace llvm;

class FindNamedClassVisitor : public RecursiveASTVisitor < FindNamedClassVisitor > {
public:
	explicit FindNamedClassVisitor(ASTContext *Context) :Context(Context){
		std::unique_ptr<UndefinedSymbols> undef = make_unique<UndefinedSymbols>();
		undefSymbols = undef->getUndefinedSymbols();
	}

	bool VisitCXXRecordDecl(CXXRecordDecl *Declaration){
		Declaration->dump();
		return true;
	}

	bool VisitVarDecl(VarDecl *decl){
		for (auto i : undefSymbols)
		{
			if (decl->getName().str() == i && !decl->isFirstDecl())
			{
				FullSourceLoc fullLocation = Context->getFullLoc(decl->getLocStart());
				if (fullLocation.isValid())
				{
					llvm::errs() << "Found Decl at"
						<< fullLocation.getSpellingLineNumber() << ":"
						<< fullLocation.getSpellingColumnNumber() << "\t";
				}
				decl->print(errs());
			}
		}
		return true;
	}

	bool VisitFunctionDecl(FunctionDecl *decl){
		for (auto i : undefSymbols)
		{
			if (decl->getName().str() == i && decl->isFirstDecl())
			{
				FullSourceLoc fullLocation = Context->getFullLoc(decl->getLocStart());
				if (fullLocation.isValid())
				{
					llvm::errs() << "Found Decl at"
						<< fullLocation.getSpellingLineNumber() << ":"
						<< fullLocation.getSpellingColumnNumber() << "\t";
				}
				decl->print(errs());
			}
		}
		return true;
	}

private:
	ASTContext *Context;
	std::vector<std::string> undefSymbols;
};

class FindNamedClassConsumer : public clang::ASTConsumer{
public:
	explicit FindNamedClassConsumer(ASTContext *Context) : Visitor(Context){}
	virtual void HandleTranslationUnit(clang::ASTContext &Context)
	{
		//Visitor.TraverseDecl(Context.getTranslationUnitDecl());
		auto temp = Context.getTranslationUnitDecl();
		VarDecl *varDecl;
		FunctionDecl *funcDecl;
		for (auto i : temp->decls())
		{
			auto kind = i->getKind();
			switch (kind)
			{
			case Decl::Var:
				varDecl = (VarDecl*) i;
				std::cerr << varDecl->getName().str() << std::endl;
				varDecl->print(errs());
				break;
			case Decl::Function:
				funcDecl = (FunctionDecl*) i;
				std::cerr << funcDecl->getName().str() << std::endl;
				funcDecl->print(errs());
				break;
			default:
				break;
			}
		}
	}
	
	//virtual bool HandleTopLevelDecl(DeclGroupRef D){
	//	for (auto b = D.begin(), e = D.end(); b != e; ++b)
	//	{
	//		//Visitor.TraverseDecl(*b);
	//		*b.dumpColor();
	//		return true;
	//	}
	//}
private:
	FindNamedClassVisitor Visitor;
};

class FindNamedClassAction : public clang::ASTFrontendAction{
public:
	virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
		clang::CompilerInstance &Compiler, llvm::StringRef InFile){
		return std::unique_ptr<clang::ASTConsumer>(
			new FindNamedClassConsumer(&Compiler.getASTContext()));
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	std::vector<std::string> args;
	args.emplace_back("-DAVI");

	/*std::ifstream t("C:/Test.c");
	std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	clang::tooling::runToolOnCodeWithArgs(new FindNamedClassAction, str, args);*/

	CompilerInstance Clang;
	Clang.createDiagnostics(nullptr, false);

	Clang.getLangOpts().CPlusPlus = 1;

	std::shared_ptr<TargetOptions> to = std::make_shared<TargetOptions>();
	to->Triple = llvm::sys::getDefaultTargetTriple();
	TargetInfo *ti = TargetInfo::CreateTargetInfo(Clang.getDiagnostics(), to);
	Clang.setTarget(ti);

	Clang.createFileManager();
	FileManager &fm = Clang.getFileManager();

	Clang.createSourceManager(fm);
	SourceManager &sm = Clang.getSourceManager();

	PreprocessorOptions &ppOptions = Clang.getPreprocessorOpts();
	PreprocessorMacros ppMacros;
	std::vector<std::string> macros = ppMacros.getMacros();
	for (auto i : macros)
	{
		ppOptions.addMacroDef(i);
	}
	
	Clang.createPreprocessor(clang::TU_Prefix);

	Clang.createASTContext();

	const FileEntry* file = fm.getFile("C:/Test.c");
	sm.setMainFileID(sm.createFileID(file,SourceLocation(),clang::SrcMgr::C_User));

	Clang.getDiagnosticClient().BeginSourceFile(Clang.getLangOpts(), &Clang.getPreprocessor());

	FindNamedClassConsumer consumer(&Clang.getASTContext());

	ParseAST(Clang.getPreprocessor(), &consumer, Clang.getASTContext());

	return 0;
}

