#include <stdint.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <set>

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
#include <llvm/Support/raw_os_ostream.h>

#include "RealSourceRanges.hpp"

using namespace clang;

namespace
{
  typedef llvm::raw_fd_ostream RawOS;
  
  typedef std::string String;
  
  typedef std::map<Decl *, String> DeclToSymMap;
  typedef std::map<Stmt *, String> StmtToSymMap;
  typedef std::map<String, Decl *> SymToDeclMap;
  typedef std::map<String, Stmt *> SymToStmtMap;
  
  typedef std::pair<Decl *, String> VarPair;
  typedef std::set<String> SymbolSet;
  
  void printLine(RawOS & os, Expr * E, SymbolSet dependantSymbols);
  void printLine(RawOS & os, Stmt * S, SymbolSet dependantSymbols);
  void printSymbol(RawOS &os, RangeKindToGUIDMap varNames);
  void printSourceRanges(RawOS &os, OffsetRanges & oRanges);
  void printDependencies(RawOS &os, String logVar, SymbolSet dependantSymbols);
  void printDependency(RawOS &os, String logVar, String dependency);
  
  inline void _debug(String s)
  {
    //std::cerr << s;
    //std::cerr.flush();
  }
  
  unsigned int GUID_COUNT = 0;
  String getNewGUID()
  {
    std::ostringstream oss;
    oss << "sym" << (GUID_COUNT++);
    return oss.str();
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
    ConstraintVisitor(RawOS & stream,
                      DeclToSymMap globalMap,
                      SymToDeclMap invGlobalMap,
                      SourceManager * mgr)
      :declToSymbolMap(globalMap),
       symbolToDeclMap(invGlobalMap),
       os(stream),
       SM(mgr)
    {
    }
    
    void printDeclMap(SymToDeclMap map)
    {
      llvm::raw_os_ostream fos(std::cerr);
      
      for (SymToDeclMap::iterator it = map.begin(); it != map.end(); it++)
      {
        fos << it->first << "\n\t# ";
        (it->second)->print(fos);
        fos << "\n";
        fos.flush();
      }
    }
    
    String AddStmt(Stmt * S)
    {
      String symbol = getNewGUID(); // Fetch a new GUID
      
      stmtToSymbolMap[S] = symbol; // Create a mapping from the Stmt to the Symbol
      symbolToStmtMap[symbol] = S; // Create a mapping from the Symbol to the Stmt
      
      OffsetRanges oRanges = getRealSourceRange(*SM, S, stmtToSymbolMap);
      printSourceRanges(os, oRanges);
      os << "\n";
      os.flush();
      
      return symbol;
    }
    
    String AddDecl(Decl * D)
    {
      String symbol = getNewGUID(); // Fetch a new GUID
      
      declToSymbolMap[D] = symbol; // Create a mapping from the Decl to the Symbol
      symbolToDeclMap[symbol] = D; // Create a mapping from the Symbol to the Decl
      
      OffsetRanges oRanges = getRealSourceRange(*SM, D, declToSymbolMap);
      printSourceRanges(os, oRanges);
      os << "\n";
      os.flush();
      
      return symbol;
    }
    
    void VisitExpr(Expr * E)
    {
      _debug("IN\tVisitExpr\n");
      
      for (Stmt::child_iterator ChildE = E->child_begin();
           ChildE != E->child_end();
           ++ChildE)
      {
        if (*ChildE)
        {
          Visit(*ChildE);
          stmtSymbols.insert(AddStmt(*ChildE));
        }
      }
      
      _debug("OUT\tVisitExpr\n");
    }
    
    void VisitStmt(Stmt * S)
    {
      _debug("IN\tVisitStmt\n");
      
      SymbolSet outerStmtSymbols;
      
      for (Stmt::child_iterator ChildS = S->child_begin();
           ChildS != S->child_end();
           ++ChildS)
      {
        if (*ChildS)
        {
          stmtSymbols.clear();
          Visit(*ChildS);
          String sym = AddStmt(*ChildS);
          stmtSymbols.insert(sym);
          outerStmtSymbols.insert(sym);
          
          if (Expr::classof(*ChildS)) 
          {
            printLine(os, static_cast<Expr *>(*ChildS), stmtSymbols);
          }
          else
          {
            printLine(os, *ChildS, stmtSymbols);
          }
        }
      }
      
      stmtSymbols = outerStmtSymbols;
      
      _debug("OUT\tVisitStmt\n");
    }
    
    void VisitCompoundStmt(CompoundStmt * S)
    {
      _debug("IN\tVisitCompoundStmt\n");
      
      for (CompoundStmt::body_iterator BodyS = S->body_begin();
           BodyS != S->body_end();
           ++BodyS)
      {
        stmtSymbols.clear();
        Visit(*BodyS);
        stmtSymbols.insert(AddStmt(*BodyS));
        
        if (Expr::classof(*BodyS)) 
        {
          printLine(os, static_cast<Expr *>(*BodyS), stmtSymbols);
        }
        else
        {
          printLine(os, *BodyS, stmtSymbols);
        }
      }
      
      stmtSymbols.clear();
      
      _debug("OUT\tVisitCompoundStmt\n");
    }
    
    void VisitDeclStmt(DeclStmt * S)
    {
      _debug("IN\tVisitDeclStmt\n");
      
      for (DeclStmt::decl_iterator DeclS = S->decl_begin();
           DeclS != S->decl_end();
           ++DeclS)
      {
        String var = AddDecl(*DeclS);
        
        if (ValueDecl::classof(*DeclS))
        {
          ValueDecl *V = static_cast<ValueDecl *>(*DeclS);
          
          QualType qt = V->getType();
          Type* T = qt.getTypePtr();
          
          DeclForTypeVisitor dftv;
          Decl* D = dftv.Visit(T);
          
          String varType = declToSymbolMap[D];
          
          if(!varType.empty())
          {
            printDependency(os, var, varType);
          }
        }
        else
        {
          assert(0 && "Not a ValueDecl?!?\n");
        }
        
        stmtSymbols.insert(var);
      }
      
      _debug("OUT\tVisitDeclStmt\n");
    }
    
    void VisitDeclRefExpr(DeclRefExpr * E)
    {
      _debug("IN\tVisitDeclRefExpr\n");
      
      // Look up the symbol from the associated declaration
      // and add it to the stmt-level scope
      stmtSymbols.insert(declToSymbolMap[E->getDecl()]);
      
      _debug("OUT\tVisitDeclRefExpr\n");
    }
    
    void VisitBlockDeclRefExpr(BlockDeclRefExpr * E)
    {
      _debug("IN\tVisitBlockDeclRefExpr\n");
      
      // Look up the symbol from the associated declaration
      // and add it to the stmt-level scope
      stmtSymbols.insert(declToSymbolMap[E->getDecl()]);
      
      _debug("OUT\tVisitBlockDeclRefExpr\n");
    }
    
  private:
    void printStmtKind(Stmt *S)
    {
      os << "%% " << S->getStmtClassName() << "\n";
    }
    
    void printLine(RawOS & os,
                   Expr * E,
                   SymbolSet dependantSymbols)
    {
      String symbol = stmtToSymbolMap[E];
      
      RangeKindToGUIDMap varNames;
      varNames[EXPR] = symbol;
      
      printSymbol(os, varNames);
      printDependencies(os, symbol, dependantSymbols);
      
      os << "\n";
      os.flush();
    }
    
    void printLine(RawOS & os,
                   Stmt * S,
                   SymbolSet dependantSymbols)
    {
      String symbol = stmtToSymbolMap[S];
      
      RangeKindToGUIDMap varNames;
      varNames[STMT] = symbol;
      
      printSymbol(os, varNames);
      printDependencies(os, symbol, dependantSymbols);
      
      os << "\n";
      os.flush();
    }
    
  private:
    DeclToSymMap declToSymbolMap;
    SymToDeclMap symbolToDeclMap;
    StmtToSymMap stmtToSymbolMap;
    SymToStmtMap symbolToStmtMap;
    RawOS & os;
    SourceManager * SM;
    SymbolSet stmtSymbols;
  };
  
  class ConstraintGenerator : public ASTConsumer,
                              public DeclVisitor<ConstraintGenerator>
  {
  public:
    ConstraintGenerator(RawOS & stream)
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
      
      for (DeclGroupRef::iterator i = DG.begin(), e = DG.end(); i != e; ++i)
      {
        Decl *D = *i;
        
        // Don't generate constraints for decls that are not in the main
        // file, since we can't remove those anyway.
        SourceRange sr = D->getSourceRange();
        if(isInMainFile(sr))
        {
          if(declToSymbolMap.find(D) == declToSymbolMap.end())
          {
            Visit(D);
          }
        }
      }
      
      os.flush();
      
      _debug("OUT\tHandleTopLevelDecl\n");
    }

    virtual void HandleTagDeclDefinition(TagDecl *D)
    {
      _debug("IN\tHandleTagDeclDefinition\n");
      
      // Don't generate constraints for decls that are not in the main
      // file, since we can't remove those anyway.
      SourceRange sr = D->getSourceRange();
      if(!isInMainFile(sr))
      {
        return;
      }
      
      if(declToSymbolMap.find(D) == declToSymbolMap.end())
      {
        Visit(D);
      }
      
      os.flush();
      _debug("OUT\tHandleTagDeclDefinition\n");
    }

    void VisitTypedefDecl(TypedefDecl *D)
    {
      _debug("IN\tVisitTypedefDecl\n");
      
      String var = gensymDecl(D);
      printDeclKindAndName(D);
      
      OffsetRanges oRanges = getRealSourceRange(*SM, D, declToSymbolMap);
      
      RangeKindToGUIDMap varNames;
      varNames[DECL] = var;
      
      printSymbol(os, varNames);
      printSourceRanges(os, oRanges);
      
      String dependsOnDecl = getDeclarationForType(D->getUnderlyingType());
      printDependency(os, var, dependsOnDecl);
      
      os << "\n";
      os.flush();
      
      _debug("OUT\tVisitTypedefDecl\n");
    }

    void VisitEnumDecl(EnumDecl *D)
    {
      _debug("IN\tVisitEnumDecl\n");
      
      String var = gensymDecl(D);
      printDeclKindAndName(D);
      
      OffsetRanges oRanges = getRealSourceRange(*SM, D, declToSymbolMap);
      
      RangeKindToGUIDMap varNames;
      varNames[DECL] = var;
      
      printSymbol(os, varNames);
      printSourceRanges(os, oRanges);
      
      // No dependencies
      os << "\n";
      
      os.flush();
      _debug("OUT\tVisitEnumDecl\n");
    }

    void VisitRecordDecl(RecordDecl *D)
    {
      _debug("IN\tVisitRecordDecl\n");
      
      String var = gensymDecl(D);
      printDeclKindAndName(D, D->getKindName());
      
      OffsetRanges oRanges = getRealSourceRange(*SM, D, declToSymbolMap);
      
      RangeKindToGUIDMap varNames;
      varNames[DECL] = var;
      
      printSymbol(os, varNames);
      printSourceRanges(os, oRanges);
      
      for(RecordDecl::field_iterator field = D->field_begin();
          field != D->field_end();
          ++field)
      {
        // print dependencies
        if(field->isAnonymousStructOrUnion())
        {
          continue;
        }
        
        String dependsOnDecl = getDeclarationForType(field->getType());
        printDependency(os, var, dependsOnDecl);
      }
      
      os << "\n";
      os.flush();
      
      _debug("OUT\tVisitRecordDecl\n");
    }
    
    void VisitCXXRecordDecl(CXXRecordDecl *D)
    {
      _debug("HAHA\n");
    }
    
    // void VisitTemplateDecl(TemplateDecl *D)
    // {
    //   std::cerr << "VisitTemplateDecl\n";
    // }

    void VisitClassTemplateDecl(ClassTemplateDecl *D)
    {
      _debug("VisitClassTemplateDecl\n");
    }
    
    void VisitFunctionTemplateDecl(FunctionTemplateDecl *D)
    {
      _debug("VisitFunctionTemplateDecl\n");
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
      
      String var = gensymDecl(D);
      // os << "# ";
      // D->print(os);
      // os << "\n";
      printDeclKindAndName(D);
      
      OffsetRanges oRanges = getRealSourceRange(*SM, D, declToSymbolMap);
      
      RangeKindToGUIDMap varNames;
      varNames[DECL] = var;
      
      printSymbol(os, varNames);
      printSourceRanges(os, oRanges);
      
      QualType t = D->getType();
      String varType = getDeclarationForType(t);
      
      if(!varType.empty())
      {
        printDependency(os, var, varType);
      }
      
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
      {
        Visit(D->getDescribedFunctionTemplate());
      }
      
      FT = D->getPrimaryTemplate();
      if(FT) // if this is a FT instantiation or specialization
      {
        std::cerr << "create dependency to orig template\n";
      }
      
      if(D->isMain())
      {
        std::cerr << "Visiting MAIN\n";
      }
      
      // else create symbol/SR for function
      // then descend into body, creating dependencies from contained stmts to 
      // function containing them.
      
      ConstraintVisitor c(os, declToSymbolMap, symbolToDeclMap, SM);
      
      if (D->hasBody())
      {
        c.Visit(D->getBody());
      }
      
      os.flush();
      
      _debug("OUT\tVisitFunctionDecl\n");
    }
    
  private:
    String gensymDecl(Decl *D)
    {
      String symbol = getNewGUID();
      
      declToSymbolMap[D] = symbol;
      symbolToDeclMap[symbol] = D;
      
      return symbol;
    }
    
    String getDeclarationForType(QualType qt)
    {
      Type* T = qt.getTypePtr();
      
      DeclForTypeVisitor dftv;
      Decl* D = dftv.Visit(T);
      
      return declToSymbolMap[D];
    }
    
    void printDeclKindAndName(NamedDecl *D, const char* kindName="")
    {
      String nameOfKind(D->getDeclKindName());
      
      nameOfKind[0] = tolower(nameOfKind[0]);
      
      os << "%% " << ((strlen(kindName) == 0) ? nameOfKind : kindName)
         << " " << D->getQualifiedNameAsString() << "\n";
    }
    
    bool isInMainFile(SourceRange sr)
    {
      FullSourceLoc sl(sr.getBegin(), *SM);
      return !sl.isInSystemHeader(); //SM->isFromMainFile(sr.getBegin());
    }
    
  private:
    DeclToSymMap declToSymbolMap;
    SymToDeclMap symbolToDeclMap;
    RawOS & os;
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
                   const std::vector<String> & args)
    {
      for(size_t i = 0; i < args.size(); ++i)
      {
        std::cout << "Arg " << i << " = " << args[i] << std::endl;
      }
      
      os = new RawOS("out.txt", streamErrors);
      
      if(args.size() > 0 && args[0] == "help")
      {
        PrintHelp(llvm::errs());
      }
      
      return true;
    }
    
    void PrintHelp(llvm::raw_ostream& os)
    {
      os << "GenerateConstraints help\n";
    }
    
  private:
    RawOS * os;
    String streamErrors;
  };
  
  // Utility functions
  void printSymbol(RawOS &os, RangeKindToGUIDMap varNames)
  {
    for(RangeKindToGUIDMap::iterator it = varNames.begin();
        it != varNames.end();
        it++)
    {
      String predName;
      
      switch ((*it).first)
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
          
        case EXPR:
          predName = "isExpr";
          break;
          
        default:
          assert(0 && "No symbol found for RangeQualifier");
      }
      
      os << predName << "(" << (*it).second << ").\n";
    }
    
    os.flush();
  }
  
  void printSourceRanges(RawOS & os, OffsetRanges & oRanges)
  {
    for(OffsetRanges::iterator it = oRanges.begin();
        it != oRanges.end();
        it++)
    {
      RangeKindToGUIDMap varNames;
      varNames[it->getRangeType()] = it->getSymbol();
      printSymbol(os, varNames);
      
      os << "sourceRange(" << it->getSymbol() << ","
         << it->getBegin() << ","
         << it->getEnd() << ","
         << "'" << it->getFileName() <<"').\n";
      os.flush();
    }
  }
  
  void printDependencies(RawOS & os, String symbol, SymbolSet dependantSymbols)
  {
    for (SymbolSet::iterator it = dependantSymbols.begin();
         it != dependantSymbols.end();
         it++)
    {
      printDependency(os, symbol, (*it));
    }
  }
  
  void printDependency(RawOS &os, String symbol, String dependantSymbol)
  {
    if (!dependantSymbol.empty())
    {
      if (dependantSymbol.compare(symbol) != 0)
      {
        os << "dependsOn(" << symbol << ","
           << dependantSymbol << ").\n";
        os.flush();
      }
    }
  }
}

static FrontendPluginRegistry::Add<GenerateConstraintsAction>
X("gen-constraints", "Generate constraints for delta debugging");

