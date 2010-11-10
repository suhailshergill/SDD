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

using namespace clang;

namespace
{
  typedef std::map<Decl *, std::string> VarMap;
  typedef std::map<Stmt *, std::string> StmtMap;
  typedef std::pair<Decl *, std::string> VarPair;
  typedef std::set<std::string> SymbolSet;

  inline void _debug(std::string s)
  {
    //std::cerr << s;
  }

  unsigned int GUID_COUNT = 0;
  std::string getNewGUID()
  {
    std::ostringstream oss;
    oss << "sym" << GUID_COUNT;
    std::string newGUID = oss.str();

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
       stmtSymbols(),
       scopeSymbols(),
       INCEPTION_POINT(0)
    {
    }

    unsigned int INCEPTION_POINT;

    std::string AddStmt(Stmt * S)
    {
      std::string symbol = getNewGUID();
      stmtMap[S] = symbol;
      return symbol;
    }

    void AddDecl(Decl * D)
    {
      std::string symbol = getNewGUID();
      scopeMap[D] = symbol;

      VarPair P(D, symbol);

      scopeMap.insert(P);

      scopeSymbols.insert(symbol);
      stmtSymbols.insert(symbol);
    }

    void DumpSymbols(Stmt * S, std::string sym, SymbolSet set)
    {
      os << "# " << S->getStmtClassName() << "\n";

      os << "isStatement(" << sym << ").\n";

      SourceLocation sStart = S->getLocStart();
      SourceLocation sEnd   = S->getLocEnd();

      os << "sourceRange(" << sym << ","
         << SM->getFileOffset(sStart) << ","
         << SM->getFileOffset(sEnd) << ","
	 << SM->getBufferName(sStart) << ").\n";

      for (SymbolSet::iterator it = set.begin(); it != set.end(); it++)
	{
	  if (!(it->empty()))
	    os << "dependsOn(" << sym << "," << (*it) << ").\n";
	}

      os << "\n";

      os.flush();
    }

    void VisitStmt(Stmt * S)
    {
      // OffsetRanges oRange = getRealSourceRange(*SM, S);
      // os << "sourceRange(" << "IFCOND" << ","
      //    << oRange[0].getBegin() << ","
      //    << oRange[0].getEnd() << ","
      //    << oRange[0].getFileName() <<").\n";
      // os.flush();

		  

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
      scopeSymbols.clear();

      for (CompoundStmt::body_iterator CS = S->body_begin(), CSEnd = S->body_end(); CS != CSEnd; ++CS)
	{
	  stmtSymbols.clear();
				
	  Visit(*CS);
				
	  DumpSymbols(*CS, AddStmt(*CS), stmtSymbols);
	}

      DumpSymbols(S, AddStmt(S), scopeSymbols);

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
      std::string sym = it->second;

      scopeSymbols.insert(sym);
      stmtSymbols.insert(sym);

      _debug("OUT\tVisitDeclRefExpr\n");
    }

  private:
    VarMap scopeMap;
    StmtMap stmtMap;
    llvm::raw_fd_ostream & os;
    SourceManager * SM;
    SymbolSet scopeSymbols, stmtSymbols;
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

      OffsetRanges oRanges = getRealSourceRange(*SM, D);
      RangeKindToGUIDMap varNames;
      varNames[DECL] = var;
      printSymbol(varNames);
      printSourceRanges(varNames, oRanges);

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

      OffsetRanges oRanges = getRealSourceRange(*SM, D);
      RangeKindToGUIDMap varNames;
      varNames[DECL] = var;
      printSymbol(varNames);
      printSourceRanges(varNames, oRanges);
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

      OffsetRanges oRanges = getRealSourceRange(*SM, D);
      RangeKindToGUIDMap varNames;
      varNames[DECL] = var;
      printSymbol(varNames);
      printSourceRanges(varNames, oRanges);

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
    //   std::cerr << "VisitTemplateDecl\n";
    // }

    void VisitClassTemplateDecl(ClassTemplateDecl *D)
    {
      std::cerr << "VisitClassTemplateDecl\n";
    }
    
    void VisitFunctionTemplateDecl(FunctionTemplateDecl *D)
    {
      std::cerr << "VisitFunctionTemplateDecl\n";
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

      OffsetRanges oRanges = getRealSourceRange(*SM, D);
      RangeKindToGUIDMap varNames;
      varNames[DECL] = var;
      printSymbol(varNames);
      printSourceRanges(varNames, oRanges);

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
      if(FT) // if FunctionDecl is in fact a FT
	Visit(D->getDescribedFunctionTemplate());
      FT = D->getPrimaryTemplate();
      if(FT) // if this is a FT instantiation or specialization
	std::cerr << "create dependency to orig template\n";
      if(D->isMain())
	std::cerr << "Visiting MAIN\n";

      // else create symbol/SR for function
      // then descend into body, creating dependencies from contained stmts to 
      // function containing them.

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
      std::string newGUID = getNewGUID();
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

    void printSourceRanges(RangeKindToGUIDMap varNames, OffsetRanges & oRanges)
    {
      OffsetRanges::iterator it;
      for(it=oRanges.begin(); it != oRanges.end(); it++)
	{
	  os << "sourceRange(" << varNames[it->getRangeType()] << ","
	     << it->getBegin() << ","
	     << it->getEnd() << ","
	     << "'" << it->getFileName() <<"').\n";
	}
      os.flush();
    }

    void printDeclKindAndName(NamedDecl *D, const char* kindName="")
    {
      std::string nameOfKind(D->getDeclKindName());
      nameOfKind[0] = tolower(nameOfKind[0]);
      os << "# " << ((strlen(kindName) == 0) ? nameOfKind : kindName) << " " << D->getQualifiedNameAsString() << "\n";
    }    

    void printSymbol(RangeKindToGUIDMap varNames)
    {
      RangeKindToGUIDMap::iterator it;
      for(it=varNames.begin(); it != varNames.end(); it++)
	{
	  std::string predName;
	  switch((*it).first)
	    {
	    case DECL:
	      predName = "isDeclaration";
	      break;
	    case IFCONDITION:
	      predName = "isCondition";
	      break;
	    case STMT:
	      predName = "isStatement";
	      break;
	    case COMPOUNDSTMT:
	      predName = "isCompoundStatement";
	      break;
	    case INITIALIZER:
	      predName = "isInitializer";
	      break;
	    default:
	      assert(0 && "No symbol found for RangeQualifier");
	    }
	  os << predName << "(" << (*it).second << ").\n";
	}
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

