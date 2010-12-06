#include <stdint.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <assert.h>

#include <clang/AST/AST.h>
#include <clang/AST/DeclVisitor.h>
#include <clang/AST/TypeLoc.h>
#include <clang/Basic/SourceManager.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>

#include "BaseException.hpp"
#include "RealSourceRanges.hpp"

using namespace clang;

namespace
{
  DeclToSymMap declToSymbolMap;
  StmtToSymMap stmtToSymbolMap;
  
  struct TokenScanException : public BaseException
  {
    enum ScanDirection
    {
      SCAN_FORWARD,
      SCAN_BACKWARD
    };
    
    TokenScanException(const std::string & keyword, size_t startingIndex, ScanDirection sd)
    {
      std::string msg;
      llvm::raw_string_ostream os(msg);
      std::string dir;
      
      switch(sd)
      {
        case SCAN_FORWARD:
          dir = "forward";
          break;
          
        case SCAN_BACKWARD:
          dir = "backward";
          break;
      }
      
      os << "TokenScanException: Could not find '" << keyword
         << "' scanning  " << dir << " from offset " << startingIndex;
      
      setMessage(msg);
    }
  };
  
  struct DeclRangeCalculatorNotImplementedException : public BaseException
  {
    DeclRangeCalculatorNotImplementedException(Decl* D)
    {
      std::string msg;
      llvm::raw_string_ostream os(msg);
      
      os << "NotImplemented: Decl range calculation for " << D
         << " (" << D->getDeclKindName() << ")";
      
      setMessage(msg);
    }
  };
  
  size_t scanBackTo(const std::string & keyword, FullSourceLoc sl, bool isInclusive=true)
  {
    assert(!keyword.empty());
    
    const llvm::MemoryBuffer *buf = sl.getBuffer();
    
    const char * bufferStart = buf->getBufferStart();
    const char * tokenStart = sl.getCharacterData();
    
    size_t startingIndex = tokenStart - bufferStart;
    size_t currentIndex = startingIndex + 1;
    
    while(--currentIndex >= 0)
    {
      while(bufferStart[currentIndex] != keyword[0])
      {
        --currentIndex;
      }
      
      std::string maybeKeyword(bufferStart + currentIndex, keyword.size());
      
      if(maybeKeyword == keyword)
      {
        return (isInclusive ? currentIndex : currentIndex + keyword.length());
      }
    }
    
    throw TokenScanException(keyword, startingIndex, TokenScanException::SCAN_BACKWARD);
  }
  
  size_t scanForwardTo(const std::string & keyword, FullSourceLoc sl, bool isInclusive=true)
  {
    assert(!keyword.empty());
    
    const llvm::MemoryBuffer *buf = sl.getBuffer();
    
    const char * bufferStart = buf->getBufferStart();
    const char * tokenStart = sl.getCharacterData();
    
    size_t startingIndex = tokenStart - bufferStart;
    size_t currentIndex = startingIndex;
    
    // Jump to before the token
    while(currentIndex >= 0)
    {
      while(bufferStart[currentIndex] != keyword[0])
      {
        ++currentIndex;
      }
      
      std::string maybeKeyword(bufferStart + currentIndex, keyword.size());
      
      if(maybeKeyword == keyword)
      {
        return (isInclusive ? currentIndex + keyword.length() : currentIndex);
      }
    }
    
    throw TokenScanException(keyword, startingIndex, TokenScanException::SCAN_FORWARD);
  }
  
  class DeclSourceRangeVisitor : public DeclVisitor<DeclSourceRangeVisitor, OffsetRanges>
  {
  public:
    DeclSourceRangeVisitor(SourceManager & SMan)
      :SM(SMan)
    {
    }
    
    OffsetRanges VisitTypedefDecl(TypedefDecl * D)
    {
      OffsetRanges oRanges;
      
      FullSourceLoc definedTypeBegin(D->getLocation(), SM);
      size_t typedefBegin = scanBackTo("typedef", definedTypeBegin);
      size_t typedefEnd = scanForwardTo(";", definedTypeBegin);
      
      oRanges.insert(oRanges.begin(),
                     OffsetRange(declToSymbolMap[D],
                                 typedefBegin,
                                 typedefEnd,
                                 definedTypeBegin.getBuffer()->getBufferIdentifier(),
                                 DECL));
      _debug("TypedefDecl::DECL          - ");
      _debug(declToSymbolMap[D]);
      _debug("\n");
      
      return oRanges;
    }

    OffsetRanges VisitEnumDecl(EnumDecl * D)
    {
      OffsetRanges oRanges;
      
      FullSourceLoc definedTypeBegin(D->getLocation(), SM);
      size_t typedefBegin = scanBackTo("enum", definedTypeBegin);
      size_t typedefEnd = scanForwardTo(";", definedTypeBegin);
      oRanges.insert(oRanges.begin(),
                     OffsetRange(declToSymbolMap[D],
                                 typedefBegin,
                                 typedefEnd,
                                 definedTypeBegin.getBuffer()->getBufferIdentifier(),
                                 DECL));
      _debug("EnumDecl::DECL             - ");
      _debug(declToSymbolMap[D]);
      _debug("\n");
      
      return oRanges;
    }

    OffsetRanges VisitRecordDecl(RecordDecl * D)
    {
      OffsetRanges oRanges;
      
      FullSourceLoc definedTypeBegin(D->getLocation(), SM);
      FullSourceLoc definedTypeEnd(D->getRBraceLoc(), SM);
      const char* kindName;
      if (D->isStruct())
        kindName = "struct";
      if (D->isUnion())
        kindName = "union";
      if (D->isClass())
        kindName = "class";
      size_t typedefBegin = scanBackTo(kindName, definedTypeBegin);
      size_t typedefEnd = scanForwardTo(";", definedTypeEnd);
      oRanges.insert(oRanges.begin(),
                     OffsetRange(declToSymbolMap[D],
                                 typedefBegin,
                                 typedefEnd,
                                 definedTypeBegin.getBuffer()->getBufferIdentifier(),
                                 DECL));
      _debug("RecordDecl::DECL           - ");
      _debug(declToSymbolMap[D]);
      _debug("\n");
      
      return oRanges;
    }
    
    OffsetRanges VisitVarDecl(VarDecl * D)
    {
      OffsetRanges oRanges;
      
      SourceRange sr = D->getSourceRange();
      SourceLocation sBegin = SM.getSpellingLoc(sr.getBegin());
      SourceLocation sEnd = SM.getSpellingLoc(sr.getEnd());
      // SourceRange qr = D->DeclaratorDecl::getQualifierRange();
      SourceLocation qBegin = SM.getSpellingLoc(D->getTypeSpecStartLoc());//qr.getBegin());
      oRanges.insert(oRanges.begin(),
                     OffsetRange(declToSymbolMap[D],
                                 SM.getFileOffset(qBegin),
                                 SM.getFileOffset(sEnd),
                                 SM.getBufferName(sBegin),
                                 DECL));
      _debug("VarDecl::DECL              - ");
      _debug(declToSymbolMap[D]);
      _debug("\n");
      
      return oRanges;
    }
    
    OffsetRanges VisitDecl(Decl * D)
    {
      throw DeclRangeCalculatorNotImplementedException(D);
    }
    
    DeclToSymMap declToSymbolMap;
    
  private:
    SourceManager & SM;
  };
  
  class StmtSourceRangeVisitor : public StmtVisitor<StmtSourceRangeVisitor, OffsetRanges>
  {
  public:
    StmtSourceRangeVisitor(SourceManager & SMan)
      :SM(SMan)
    {
    }
    
    OffsetRanges VisitIfStmt(IfStmt * S)
    {
      OffsetRanges oRanges;
      
      Expr* C = S->getCond();
      FullSourceLoc ifConditionB(C->getLocStart(), SM);
      FullSourceLoc ifConditionE(C->getLocEnd(), SM);
      size_t conditionBegin = scanBackTo("(", ifConditionB, false);
      size_t conditionEnd = scanForwardTo(")", ifConditionE, false);
      oRanges.insert(oRanges.begin(),
                     OffsetRange(stmtToSymbolMap[C],
                                 conditionBegin,
                                 conditionEnd,
                                 ifConditionB.getBuffer()->getBufferIdentifier(),
                                 IFCONDITION));
      _debug("IfStmt::IFCONDITION        - ");
      _debug(stmtToSymbolMap[C]);
      _debug("\n");
      
      Stmt* T = S->getThen();
      Stmt* E = (S->getElse() != NULL) ? S->getElse() : S->getThen();
      FullSourceLoc ifBlockB(T->getLocStart(), SM);
      FullSourceLoc ifBlockE(E->getLocEnd(), SM);
      size_t ifBegin = scanBackTo("if", ifBlockB, true);
      size_t ifEnd = scanForwardTo("}", ifBlockE, true);
      oRanges.insert(oRanges.begin(),
                     OffsetRange(stmtToSymbolMap[T],
                                 ifBegin,
                                 ifEnd,
                                 ifBlockB.getBuffer()->getBufferIdentifier(),
                                 STMT));
      _debug("IfStmt::STMT               - ");
      _debug(stmtToSymbolMap[T]);
      _debug("\n");
      
      return oRanges;
    }
    
    OffsetRanges VisitCompoundStmt(CompoundStmt * S)
    {
      OffsetRanges oRanges;
      
      FullSourceLoc stmtBegin(S->getLocStart(), SM);
      FullSourceLoc stmtEnd(S->getLocEnd(), SM);
      size_t posBegin = scanBackTo("{", stmtBegin, true);
      size_t posEnd = scanForwardTo("}", stmtEnd, true);
      oRanges.insert(oRanges.begin(),
                     OffsetRange(stmtToSymbolMap[S],
                                 posBegin,
                                 posEnd,
                                 stmtBegin.getBuffer()->getBufferIdentifier(),
                                 COMPOUNDSTMT));
      _debug("CompoundStmt::COMPOUNDSTMT - ");
      _debug(stmtToSymbolMap[S]);
      _debug("\n");
      
      return oRanges;
    }
    
    OffsetRanges VisitExpr(Expr * E)
    {
      OffsetRanges oRanges;
      
      SourceRange sr = E->getSourceRange();
      FullSourceLoc exprBegin(sr.getBegin(), SM);
      FullSourceLoc exprEnd(sr.getEnd(), SM);
      size_t posBegin = exprBegin.getCharacterData()
                      - exprBegin.getBuffer()->getBufferStart();
      size_t posEnd = exprEnd.getCharacterData()
                    - exprEnd.getBuffer()->getBufferStart();
      oRanges.insert(oRanges.begin(),
                     OffsetRange(stmtToSymbolMap[E],
                                 posBegin,
                                 posEnd,
                                 exprBegin.getBuffer()->getBufferIdentifier(),
                                 EXPR));
      _debug("Expr::EXPR                 - ");
      _debug(stmtToSymbolMap[E]);
      _debug("\n");
      
      return oRanges;
    }
    
    OffsetRanges VisitStmt(Stmt * S)
    {
      OffsetRanges oRanges;
    	
      FullSourceLoc stmtBegin(S->getLocStart(), SM);
      FullSourceLoc stmtEnd(S->getLocEnd(), SM);
      size_t posBegin = stmtBegin.getCharacterData()
                      - stmtBegin.getBuffer()->getBufferStart();
      size_t posEnd = scanForwardTo(";", stmtEnd, true);
      oRanges.insert(oRanges.begin(),
                     OffsetRange(stmtToSymbolMap[S],
                                 posBegin,
                                 posEnd,
                                 stmtBegin.getBuffer()->getBufferIdentifier(),
                                 STMT));
      _debug("Stmt::STMT                 - ");
      _debug(stmtToSymbolMap[S]);
      _debug("\n");
      
      return oRanges;
    }
    
    StmtToSymMap stmtToSymbolMap;
    
  private:
    SourceManager & SM;
  };
}

OffsetRanges getRealSourceRange(SourceManager & SM, Decl *D, DeclToSymMap map)
{
  DeclSourceRangeVisitor dsrv(SM);
  dsrv.declToSymbolMap = map;
  return dsrv.Visit(D);
}

OffsetRanges getRealSourceRange(SourceManager & SM, Stmt *S, StmtToSymMap map)
{
  StmtSourceRangeVisitor ssrv(SM);
  ssrv.stmtToSymbolMap = map;
  return ssrv.Visit(S);
}
