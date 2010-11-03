#include <stdint.h>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/AST.h>
#include <clang/AST/DeclVisitor.h>
#include <clang/AST/StmtVisitor.h>
#include <clang/AST/TypeLoc.h>
#include <clang/AST/TypeVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Index/ASTLocation.h>
#include <llvm/Support/raw_ostream.h>

#include "RealSourceRanges.hpp"

using namespace clang;

namespace
{
  class DeclForTypeVisitor : public TypeVisitor<DeclForTypeVisitor, Decl*>
  {
  public:
    Decl* VisitTypedefType(TypedefType *T)
    {
      return T->getDecl();
    }

    Decl* VisitTagType(TagType *T)
    {
      return T->getDecl();
    }

    Decl* VisitType(Type *T)
    {
      return NULL;
    }
  };

  class ConstraintGenerator : public ASTConsumer,
                              public DeclVisitor<ConstraintGenerator>
  {
  public:
    ConstraintGenerator(llvm::raw_fd_ostream & stream)
    :os(stream),
     declGensym(0),
     typeGensym(0),
     stmtGensym(0),
     astContext(NULL)
    {
    }

    virtual ~ConstraintGenerator() { }

    virtual void Initialize(ASTContext & Context)
    {
      astContext = &Context;
      SM = &astContext->getSourceManager();
    }

    virtual void HandleTopLevelDecl(DeclGroupRef DG)
    {
			std::cerr << "IN\tHandleTopLevelDecl\n";

      for (DeclGroupRef::iterator i = DG.begin(), e = DG.end(); i != e; ++i)
      {
        Decl *D = *i;
        // Don't generate constraints for decls that are not in the main
        // file, since we can't remove those anyway.
        SourceRange sr = D->getSourceRange();
        if(!isInMainFile(sr)) return;

        if(declToLogicVarMap.find(D) == declToLogicVarMap.end())
          Visit(D);
      }

			os.flush();
			std::cerr << "OUT\tHandleTopLevelDecl\n";
    }

    virtual void HandleTagDeclDefinition(TagDecl *D)
    {
			std::cerr << "IN\tHandleTagDeclDefinition\n";

      // Don't generate constraints for decls that are not in the main
      // file, since we can't remove those anyway.
      SourceRange sr = D->getSourceRange();
      if(!isInMainFile(sr)) return;

      if(declToLogicVarMap.find(D) == declToLogicVarMap.end())
        Visit(D);

			os.flush();
			std::cerr << "OUT\tHandleTagDeclDefinition\n";
    }

    void VisitTypedefDecl(TypedefDecl *D)
   {
			std::cerr << "IN\tVisitTypedefDecl\n";

      std::string var = gensymDecl(D);
      os << "# ";
      D->print(os);
      os << "\n";

      printDeclaration(var);

      OffsetRange oRange = getRealSourceRange(*SM, D);
      printSourceRange(var, oRange);

      std::string dependsOnDecl = getDeclarationForType(D->getUnderlyingType());
      printDependency(var, dependsOnDecl);

      os << "\n";

			os.flush();
			std::cerr << "OUT\tVisitTypedefDecl\n";
    }

		// FORMAT:
		// E := c(e1, e2, ... eN)
		// E->calleeDecl = c
		// CallExpr::arg_iterator
		// E->arg_begin  = e1
		// E->arg_end    = eN
		void VisitCallExpr(CallExpr *E)
		{
			std::cerr << "IN\tVisitCallExpr\n";

			Decl * calleeDecl = E->getCalleeDecl();
			calleeDecl->print(os);
			// introduce dependency (E -> calleeDecl)

			for (CallExpr::arg_iterator Arg = E->arg_begin(), ArgEnd = E->arg_end(); Arg != ArgEnd; ++Arg)
			{
				//introduce dependency (E -> Arg)
				
			}

			os.flush();
			std::cerr << "OUT\tVisitCallExpr\n";
		}

		// FORMAT:
		// E := c(s)
		// E->getCastKind = c
		// E->getSubExpr 	= s
		void VisitCastExpr(CastExpr *E)
		{
			std::cerr << "IN\tVisitCastExpr\n";

			CastKind castKind = E->getCastKind();
			// introduce dependency (E -> castKind)

			Expr * subExpr = E->getSubExpr();
			// introduce dependency (E -> subExpr)

			os.flush();
			std::cerr << "OUT\tVisitCastExpr\n";
		}

    void VisitEnumDecl(EnumDecl *D)
    {
			std::cerr << "IN\tVisitEnumDecl\n";

      std::string var = gensymDecl(D);
      os << "# enum " << D->getQualifiedNameAsString() << "\n";

      printDeclaration(var);
      OffsetRange oRange = getRealSourceRange(*SM, D);
      printSourceRange(var, oRange);
      // No dependencies
      os << "\n";
			
			os.flush();
			std::cerr << "OUT\tVisitEnumDecl\n";
    }

    void VisitRecordDecl(RecordDecl *D)
    {
			std::cerr << "IN\tVisitRecordDecl\n";

			os.flush();
			std::cerr << "OUT\tVisitRecordDecl\n";
    }

    void VisitVarDecl(VarDecl *D)
    {
			std::cerr << "IN\tVisitVarDecl\n";

      // FIXME: Need to descend into the initializer to generate
      // dependencies on variables referenced there (the initializer
      // itself should depend on them, instead of the entire decl).
      // This may not be as much of an issue for global decls
      std::string var = gensymDecl(D);
      os << "# ";
      D->print(os);
      os << "\n";

      printDeclaration(var);
      OffsetRange oRange = getRealSourceRange(*SM, D);
      printSourceRange(var, oRange);

      QualType t = D->getType();
      std::string varType = getDeclarationForType(t);
      if(!varType.empty())
        printDependency(var, varType);

      os << "\n";

			os.flush();
			std::cerr << "OUT\tVisitVarDecl\n";
    }

    void VisitFunctionDecl(FunctionDecl *D)
    {
			std::cerr << "IN\tVisitFunctionDecl\n";

			os.flush();
			std::cerr << "OUT\tVisitFunctionDecl\n";
    }

  private:
    std::string gensymDecl(Decl *D)
    {
      std::ostringstream ss;
      ss << "d" << declGensym++;
      declToLogicVarMap[D] = ss.str();
      return ss.str();
    }

    std::string getDeclarationForType(QualType qt)
    {
      Type* T = qt.getTypePtr();
      DeclForTypeVisitor dftv;
      Decl* d = dftv.Visit(T);
      std::map<Decl*, std::string>::const_iterator it = declToLogicVarMap.find(d);
      if(it == declToLogicVarMap.end()) return "";

      return it->second;
    }

    void printSourceRange(const std::string & logVar, const OffsetRange & oRange)
    {
      os << "sourceRange(" << logVar << ","
         << oRange.first << ","
         << oRange.second << ").\n";
			os.flush();
    }

    void printDeclaration(const std::string & logVar)
    {
      os << "isDeclaration(" << logVar << ").\n";
			os.flush();
    }

    void printDependency(const std::string & logVar,
        const std::string & dependency)
    {
      if(!dependency.empty())
        os << "dependsOn(" << logVar << ","
           << dependency << ").\n";
			os.flush();
    }

    bool isInMainFile(SourceRange sr)
    {
      return true; //SM->isFromMainFile(sr.getBegin());
    }

  private:
    std::map<Decl*, std::string> declToLogicVarMap;
    llvm::raw_fd_ostream & os;
    uint64_t declGensym;
    uint64_t typeGensym;
    uint64_t stmtGensym;
    ASTContext * astContext;
    SourceManager * SM;
  };

  class GenerateConstraintsAction : public PluginASTAction
  {
  public:
    GenerateConstraintsAction()
    :os(NULL)
    {
    }

    virtual ~GenerateConstraintsAction()
    {
    }

  protected:
    ASTConsumer* CreateASTConsumer(CompilerInstance &CI, llvm::StringRef sref)
    {
      return new ConstraintGenerator(*os);
    }

    bool ParseArgs(const CompilerInstance& CI,
        const std::vector<std::string> & args)
    {
      for(size_t i = 0; i < args.size(); ++i)
      {
        std::cout << "Arg " << i << " = " << args[i] << std::endl;
      }

      os = new llvm::raw_fd_ostream("out.txt", streamErrors);
      if(args.size() > 0 && args[0] == "help")
        PrintHelp(llvm::errs());
      return true;
    }

    void PrintHelp(llvm::raw_ostream& os)
    {
      os << "GenerateConstraints help\n";
    }

  private:
    llvm::raw_fd_ostream * os;
    std::string streamErrors;
  };
}

static FrontendPluginRegistry::Add<GenerateConstraintsAction>
X("gen-constraints", "Generate constraints for delta debugging");

