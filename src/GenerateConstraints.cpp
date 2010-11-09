#include <stdint.h>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <iostream>

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/AST.h>
#include <clang/AST/DeclVisitor.h>
#include <clang/AST/Decl.h>
#include <clang/AST/StmtVisitor.h>
#include <clang/AST/TypeLoc.h>
#include <clang/AST/TypeVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Index/ASTLocation.h>
#include <llvm/Support/raw_ostream.h>

#include "RealSourceRanges.hpp"

// #include <typeinfo>
// #include "/u/s/h/shergill/SW/llvm-root/llvm/tools/clang/test/CodeGenCXX/typeinfo"
using namespace clang;

namespace
{
	typedef std::string String;
	typedef std::map<Decl *, std::string> VarMap;
	typedef std::pair<Decl *, std::string> VarPair;
	typedef std::set<std::string> SymbolSet;

	inline void _debug(std::string s)
	{
		//std::cerr << s;
	}

	unsigned int GUID_COUNT = 0;
	String getNewGUID()
	{
		std::ostringstream oss;
		oss << "sym" << GUID_COUNT;
		String newGUID = oss.str();

		GUID_COUNT++;

		return newGUID;
	}

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

	class ConstraintVisitor : public StmtVisitor<ConstraintVisitor>
	{
	public:
		ConstraintVisitor(llvm::raw_fd_ostream & stream, VarMap globalMap, SourceManager * mgr)
		:os(stream),
		 scopeMap(globalMap),
		 SM(mgr),
		 lineSymbols(),
		 scopeSymbols(),
		 SYM_NUM(0),
		 INCEPTION_POINT(0)
		{
		}

		unsigned int SYM_NUM;
		unsigned int INCEPTION_POINT;

		void AddDecl(Decl * D)
		{
			String symbol = getNewGUID();

			VarPair P(D, symbol);

			scopeMap.insert(P);

			scopeSymbols.insert(symbol);
			lineSymbols.insert(symbol);
		}

		void DumpSymbols(SymbolSet set, String description)
		{
			for (unsigned int LVL = 0; LVL < INCEPTION_POINT; LVL++)
				std::cerr << "\t";

			std::cerr << description << "\tREFS\t{";

			for (SymbolSet::iterator it = set.begin(); it != set.end(); it++)
			{
				if (it != set.begin()) std::cerr << ", ";
				std::cerr << *it;
			}

			std::cerr << "}\n";
		}

		void VisitStmt(Stmt * S)
		{
			for (Stmt::child_iterator I = S->child_begin(), E = S->child_end(); I != E; ++I)
			{
				if (*I) Visit(*I);
			}
		}

		void VisitCompoundStmt(CompoundStmt * S)
		{
			_debug("IN\tVisitCompountStmt\n");

			INCEPTION_POINT++;
			SymbolSet prevScopeSymbols = scopeSymbols;

			for (CompoundStmt::body_iterator CS = S->body_begin(), CSEnd = S->body_end(); CS != CSEnd; ++CS)
			{
				lineSymbols.clear();
				
				Visit(*CS);
				
				DumpSymbols(lineSymbols, "STMT");
			}

			DumpSymbols(scopeSymbols, "SCOPE");

			INCEPTION_POINT--;
			scopeSymbols = prevScopeSymbols;

			_debug("OUT\tVisitCompountStmt\n");
		}

		void VisitDeclStmt(DeclStmt * S)
		{
			_debug("IN\tVisitDeclStmt\n");

			for (DeclStmt::decl_iterator I = S->decl_begin(), E = S->decl_end(); I != E; ++I)
				AddDecl(*I);

			_debug("OUT\tVisitDeclStmt\n");
		}

		void VisitDeclRefExpr(DeclRefExpr * E)
		{
			_debug("IN\tVisitDeclRefExpr\n");

			VarMap::const_iterator it = scopeMap.find(E->getDecl());
			String sym = it->second;

			scopeSymbols.insert(sym);
			lineSymbols.insert(sym);

			_debug("OUT\tVisitDeclRefExpr\n");
		}

	private:
    VarMap scopeMap;
    llvm::raw_fd_ostream & os;
    SourceManager * SM;
		SymbolSet scopeSymbols, lineSymbols;
	};

  class ConstraintGenerator : public ASTConsumer,
                              public DeclVisitor<ConstraintGenerator>
  {
  public:
    ConstraintGenerator(llvm::raw_fd_ostream & stream)
    :os(stream),
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
      _debug("IN\tHandleTopLevelDecl\n");
      std::cerr << "TOPLEVEL\n";
      
      for (DeclGroupRef::iterator i = DG.begin(), e = DG.end(); i != e; ++i)
      {
        Decl *D = *i;
        // Don't generate constraints for decls that are not in the main
        // file, since we can't remove those anyway.
        SourceRange sr = D->getSourceRange();
        if(!isInMainFile(sr)) return;

        if(declToLogicVarMap.find(D) == declToLogicVarMap.end())
	  {
	    std::cerr << "Visiting\n";
	    // std::cerr << typeid(*D).name() << std::endl;
	    if(FunctionTemplateDecl* D1 = dyn_cast< FunctionTemplateDecl >(D))
	      {
		VisitFunctionTemplateDecl(D1);
	      }
	    switch(D->getKind())
	      {
	      case Decl::FunctionTemplate:
		std::cerr << "FT!\n";
		break;
	      case Decl::ClassTemplate:
		std::cerr << "CT!\n";
		break;
	      case Decl::TemplateTemplateParm:
		std::cerr << "TTP\n";
		break;
	      case Decl::TemplateTypeParm:
		std::cerr << "TemplateTypeParm\n";
		break;
	      case Decl::Var:
		std::cerr << "Var\n";
		break;
	      case Decl::Function:
		std::cerr << "Function\n";
		break;
		
	      default:
		std::cerr << "shit " << D->getKind() << "\n";
	      }
	    
	    
	    Visit(D);
	    std::cerr << "DONE\n";
	    
	  }
	
      }

			os.flush();
			_debug("OUT\tHandleTopLevelDecl\n");
    }

    virtual void HandleTagDeclDefinition(TagDecl *D)
    {
      _debug("IN\tHandleTagDeclDefinition\n");
      std::cerr << "TAGDECL\n";
      
      // Don't generate constraints for decls that are not in the main
      // file, since we can't remove those anyway.
      SourceRange sr = D->getSourceRange();
      if(!isInMainFile(sr)) return;

      if(declToLogicVarMap.find(D) == declToLogicVarMap.end())
        Visit(D);

			os.flush();
			_debug("OUT\tHandleTagDeclDefinition\n");
    }

    void VisitTypedefDecl(TypedefDecl *D)
   {
			_debug("IN\tVisitTypedefDecl\n");

      std::string var = gensymDecl(D);
      // os << "# ";
      // D->print(os);
      // os << "\n";
      printDeclKindAndName(D);      

      printDeclaration(var);

      OffsetRange oRange = getRealSourceRange(*SM, D);
      printSourceRange(var, oRange);

      std::string dependsOnDecl = getDeclarationForType(D->getUnderlyingType());
      printDependency(var, dependsOnDecl);

      os << "\n";

			os.flush();
			_debug("OUT\tVisitTypedefDecl\n");
    }

    void VisitEnumDecl(EnumDecl *D)
    {
			_debug("IN\tVisitEnumDecl\n");

      std::string var = gensymDecl(D);
      printDeclKindAndName(D);      
      
      printDeclaration(var);
      OffsetRange oRange = getRealSourceRange(*SM, D);
      printSourceRange(var, oRange);
      // No dependencies
      os << "\n";
			
			os.flush();
			_debug("OUT\tVisitEnumDecl\n");
    }

    void VisitRecordDecl(RecordDecl *D)
    {
      _debug("IN\tVisitRecordDecl\n");
      printDeclKindAndName(D, D->getKindName());
      std::string var = gensymDecl(D);
      printDeclaration(var);
      OffsetRange oRange = getRealSourceRange(*SM, D);
      printSourceRange(var, oRange);

      RecordDecl::field_iterator fIt;      
      for(fIt = D->field_begin(); fIt != D->field_end(); fIt++)
	{
	  // print dependencies
	  if(fIt->isAnonymousStructOrUnion())
	    continue;
	  std::string dependsOnDecl = getDeclarationForType(fIt->getType());
	  printDependency(var, dependsOnDecl);	  
	}
      
      os << "\n";
      
      os.flush();
      _debug("OUT\tVisitRecordDecl\n");
    }

    void VisitCXXRecordDecl(CXXRecordDecl *D)
    {
      
      std::cerr << "HAHA\n";      
    }

    // void VisitTemplateDecl(TemplateDecl *D)
    // {
    //   std::cerr << "HOHO\n";
    // }

    // void VisitClassTemplateDecl(ClassTemplateDecl *D)
    // {
    //   std::cerr << "CT!\n";
    // }
    
    void VisitFunctionTemplateDecl(FunctionTemplateDecl *D)
    {
      std::cerr << "HEHE\n";
    }
    
    // void VisitTemplateTemplateParmDecl(TemplateTemplateParmDecl *D)
    // {
    //   std::cerr << "HMM\n";
    // }
    

    void VisitVarDecl(VarDecl *D)
    {
			_debug("IN\tVisitVarDecl\n");

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
			_debug("OUT\tVisitVarDecl\n");
    }

    void VisitFunctionDecl(FunctionDecl *D)
    {
      _debug("IN\tVisitFunctionDecl\n");
      FunctionTemplateDecl* FT;
      FT = D->getDescribedFunctionTemplate();
      if(FT)
	Visit(D->getDescribedFunctionTemplate());
      FT = D->getPrimaryTemplate();
      if(FT)
	std::cerr << "create dependency to orig template\n";
      						
			ConstraintVisitor c(os, declToLogicVarMap, SM);
			if (D->hasBody())
			{
				c.Visit(D->getBody());
			}

			os.flush();
			_debug("OUT\tVisitFunctionDecl\n");
    }

  private:
    std::string gensymDecl(Decl *D)
    {
			String newGUID = getNewGUID();
      declToLogicVarMap[D] = newGUID;
      return newGUID;
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

    void printSourceRange(const std::string & logVar, OffsetRange & oRange)
    {
      os << "sourceRange(" << logVar << ","
         << oRange.getBegin() << ","
         << oRange.getEnd() << ","
	 << oRange.getFileName() <<").\n";
			os.flush();
    }

    void printDeclKindAndName(NamedDecl *D, const char* kindName="")
    {
      std::string nameOfKind(D->getDeclKindName());
      nameOfKind[0] = tolower(nameOfKind[0]);
      os << "# " << ((strlen(kindName) == 0) ? nameOfKind : kindName) << " " << D->getQualifiedNameAsString() << "\n";
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
      FullSourceLoc sl(sr.getBegin(), *SM);
      return !sl.isInSystemHeader(); //SM->isFromMainFile(sr.getBegin());
    }

  private:
    std::map<Decl*, std::string> declToLogicVarMap;
    llvm::raw_fd_ostream & os;
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

