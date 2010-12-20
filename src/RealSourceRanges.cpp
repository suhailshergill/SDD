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
#include <llvm/Support/raw_os_ostream.h>

#include "BaseException.hpp"
#include "RealSourceRanges.hpp"

using namespace clang;

namespace
{
  DeclToSymMap declToSymbolMap;
  StmtToSymMap stmtToSymbolMap;
  
  enum ScanDirection
  {
    SCAN_FORWARD,
    SCAN_BACKWARD
  };
  
  struct TokenScanException : public BaseException
  {
    
    TokenScanException(const std::string & keyword,
                       size_t startingIndex,
                       ScanDirection sd)
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
  
  size_t scan(ScanDirection direction,
              const std::string &keyword,
              FullSourceLoc location,
              bool isInclusive = true,
              bool orParen = false,
              bool orBrace = false)
  {
    const llvm::MemoryBuffer *memBuffer = location.getBuffer();
    
    const char * buffer = memBuffer->getBufferStart();
    const char * token  = location.getCharacterData();
    
    size_t current = token - buffer;
    
    const char * beginning = memBuffer->getBufferStart();
    const char * end = memBuffer->getBufferEnd();
    
    do {
      if (orParen) {
        if ((direction == SCAN_FORWARD) && (buffer[current] == ')'))
          return current - 1;
        
        if ((direction == SCAN_BACKWARD) && (buffer[current] == '('))
          return current + 1;
      }
      
      if (orBrace) {
        if ((direction == SCAN_FORWARD) && (buffer[current] == '}'))
          return current - 1;
        
        if ((direction == SCAN_BACKWARD) && (buffer[current] == '{'))
          return current + 1;
      }
      
      if (buffer[current] == keyword[0]) {
        if ((buffer + current + keyword.size()) <= end) {
          std::string word(buffer + current, keyword.size());
        
          if (word == keyword) {
            if (direction == SCAN_FORWARD)
              return (isInclusive ? current + keyword.length() : current);
            
            if (direction == SCAN_BACKWARD)
              return (isInclusive ? current : current + keyword.length());
          }
        }
      }
      
      if (direction == SCAN_FORWARD)
        current++;
      
      if (direction == SCAN_BACKWARD)
        current --;
    } while ((buffer + current >= beginning) && (buffer + current <= end));
    
    throw TokenScanException(keyword, token - buffer, direction);
  }
  
  /*
  size_t scanBackTo(const std::string & keyword,
                    FullSourceLoc sl,
                    bool isInclusive=true,
                    bool orParen=false,
                    bool orBrace=false)
  {
    assert(!keyword.empty());
    
    const llvm::MemoryBuffer *buf = sl.getBuffer();
    
    const char * bufferStart = buf->getBufferStart();
    const char * tokenStart = sl.getCharacterData();
    
    size_t startingIndex = tokenStart - bufferStart;
    size_t currentIndex = startingIndex + 1;
    
    while (currentIndex > 0)
    {
      if ((orParen && (bufferStart[currentIndex] == '(')) ||
          (orBrace && (bufferStart[currentIndex] == '{')))
      {
        return currentIndex + 1;
      }
      
      while ((bufferStart[currentIndex] != keyword[0]) && (currentIndex > 0))
      {
        --currentIndex;
        
        if ((orParen && (bufferStart[currentIndex] == '(')) ||
            (orBrace && (bufferStart[currentIndex] == '{')))
        {
          return currentIndex + 1;
        }
      }
      
      if (currentIndex >= 0)
      {
        std::string maybeKeyword(bufferStart + currentIndex, keyword.size());
        
        if (maybeKeyword == keyword)
        {
          return (isInclusive ? currentIndex : currentIndex + keyword.length());
        }
        
        if (currentIndex > 0)
        {
          --currentIndex;
        }
      }
    }
    
    if (orParen || orBrace)
    {
      return currentIndex;
    }
    
    throw TokenScanException(keyword, startingIndex, SCAN_BACKWARD);
  }
  
  size_t scanForwardTo(const std::string & keyword,
                       FullSourceLoc sl,
                       bool isInclusive=true,
                       bool orParen=false,
                       bool orBrace=false)
  {
    assert(!keyword.empty());
    
    const llvm::MemoryBuffer *buf = sl.getBuffer();
    
    const char * bufferStart = buf->getBufferStart();
    const char * tokenStart = sl.getCharacterData();
    
    size_t startingIndex = tokenStart - bufferStart;
    size_t currentIndex = startingIndex;
    
    // Jump to before the token
    while ((currentIndex + keyword.size()) <= SIZE_MAX)
    {
      if ((orParen && (bufferStart[currentIndex] == ')')) ||
          (orBrace && (bufferStart[currentIndex] == '}')))
      {
        return currentIndex - 1;
      }
      
      while ((bufferStart[currentIndex] != keyword[0]) && ((currentIndex + keyword.size()) <= SIZE_MAX))
      {
        ++currentIndex;
        
        if ((orParen && (bufferStart[currentIndex] == ')')) ||
            (orBrace && (bufferStart[currentIndex] == '}')))
        {
          return currentIndex - 1;
        }
      }
      
      if ((currentIndex + keyword.size()) <= SIZE_MAX)
      {
        std::string maybeKeyword(bufferStart + currentIndex, keyword.size());
        
        if(maybeKeyword == keyword)
        {
          return (isInclusive ? currentIndex + keyword.length() : currentIndex);
        }
        
        ++currentIndex;
      }
    }
    
    if (orParen || orBrace)
    {
      return currentIndex;
    }
    
    throw TokenScanException(keyword, startingIndex, SCAN_FORWARD);
  }
  */
  
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
      size_t typedefBegin = scan(SCAN_BACKWARD, "typedef", definedTypeBegin);
      size_t typedefEnd = scan(SCAN_FORWARD, ";", definedTypeBegin);
      
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
      size_t typedefBegin = scan(SCAN_BACKWARD, "enum", definedTypeBegin);
      size_t typedefEnd = scan(SCAN_FORWARD, ";", definedTypeBegin);
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
      size_t typedefBegin = scan(SCAN_BACKWARD, kindName, definedTypeBegin);
      size_t typedefEnd = scan(SCAN_FORWARD, ";", definedTypeEnd);
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
                                 SM.getFileOffset(sEnd) + 1, // TODO: check (SSS)
                                 SM.getBufferName(sBegin),
                                 DECL));
      _debug("VarDecl::DECL              - ");
      _debug(declToSymbolMap[D]);
      _debug("\n");
      
      return oRanges;
    }
    
    OffsetRanges VisitParmVarDecl(ParmVarDecl * D)
    {
      OffsetRanges oRanges;
      
      FullSourceLoc parmB(D->getLocStart(), SM);
      FullSourceLoc parmE(D->getLocEnd(), SM);
      size_t beginLoc = scan(SCAN_BACKWARD, ",", parmB, false, true);
      size_t endLoc = scan(SCAN_FORWARD, ",", parmE, false, true);
      oRanges.insert(oRanges.begin(),
                     OffsetRange(declToSymbolMap[D],
                                 beginLoc,
                                 endLoc, // TODO: check (SSS)
                                 parmB.getBuffer()->getBufferIdentifier(),
                                 DECL));
      _debug("ParmVarDecl::DECL          - ");
      _debug(declToSymbolMap[D]);
      _debug("\n");
      
      return oRanges;
    }
    
    OffsetRanges VisitFieldDecl(FieldDecl * D)
    {
      OffsetRanges oRanges;
      
      FullSourceLoc fieldB(D->getLocStart(), SM);
      FullSourceLoc fieldE(D->getLocEnd(), SM);
      /*
  NOTE: computing sourcerange this way is WRONG unless the file is
  preprocessed (SSS)
  FIXME: the scanBackTo (perhaps even the scanForwardTo) function seems to
  be buggy. (SSS)
      */
      size_t beginLoc = scan(SCAN_BACKWARD, ";", fieldB, false, false, true);
      size_t endLoc = scan(SCAN_FORWARD, ";", fieldE, false, false, true);
      oRanges.insert(oRanges.begin(),
                     OffsetRange(declToSymbolMap[D],
                                 beginLoc,
                                 endLoc,
                                 fieldB.getBuffer()->getBufferIdentifier(),
                                 DECL));
      _debug("FieldDecl::DECL            - ");
      _debug(declToSymbolMap[D]);
      _debug("\n");
      
      return oRanges;
    }
    
    OffsetRanges VisitFunctionDecl(FunctionDecl * D)
    {
      OffsetRanges oRanges;

      std::string typeName = D->getType().getAsString();
      size_t firstParen = typeName.find("(");
      /* 
   NOTE: the '-1' below is to remove the whitespace. This is *extremely*
   brittle and should be replaced with a proper 'trim' function later
      */
      std::string returnTypeName = typeName.substr(0, firstParen -1);
      
      FullSourceLoc funB(D->getLocStart(), SM);
      FullSourceLoc funE(D->getLocEnd(), SM);
      // size_t beginLoc = funB.getCharacterData()
      //                 - funB.getBuffer()->getBufferStart();
      size_t beginLoc = scan(SCAN_BACKWARD, returnTypeName, funB, true);
      size_t endLoc = scan(SCAN_FORWARD, "}", funE, true);
      oRanges.insert(oRanges.begin(),
                     OffsetRange(declToSymbolMap[D],
                                 beginLoc,
                                 endLoc,
                                 funB.getBuffer()->getBufferIdentifier(),
                                 DECL));
      _debug("FunctionDecl::DECL         - ");
      _debug(declToSymbolMap[D]);
      _debug("\n");
      
      return oRanges;
    }
    
    OffsetRanges VisitDecl(Decl * D)
    {
      llvm::raw_os_ostream fos(std::cerr);
      D->print(fos);
      fos.flush();
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
    
    OffsetRanges VisitDoStmt(DoStmt * S)
    {
      OffsetRanges oRanges;
      
      Expr* C = S->getCond();
      FullSourceLoc condB(C->getLocStart(), SM);
      FullSourceLoc condE(C->getLocEnd(), SM);
      size_t posCondB = scan(SCAN_BACKWARD, "(", condB, false);
      size_t posCondE = scan(SCAN_FORWARD, ")", condE, false);
      oRanges.insert(oRanges.begin(),
                     OffsetRange(stmtToSymbolMap[C],
                                 posCondB,
                                 posCondE,
                                 condB.getBuffer()->getBufferIdentifier(),
                                 IFCONDITION));
      _debug("DoStmt::IFCONDITION        - ");
      _debug(stmtToSymbolMap[C]);
      _debug("\n");
      
      Stmt* B = S->getBody();
      FullSourceLoc bodyB(B->getLocStart(), SM);
      size_t posBodyB = scan(SCAN_BACKWARD, "do", bodyB, true);
      size_t posBodyE = scan(SCAN_FORWARD, ";", condE, true);
      oRanges.insert(oRanges.begin(),
                     OffsetRange(stmtToSymbolMap[S],
                                 posBodyB,
                                 posBodyE,
                                 bodyB.getBuffer()->getBufferIdentifier(),
                                 STMT));
      _debug("DoStmt::STMT               - ");
      _debug(stmtToSymbolMap[S]);
      _debug("\n");
      
      return oRanges;
    }
    
    OffsetRanges VisitWhileStmt(WhileStmt * S)
    {
      OffsetRanges oRanges;
      
      Expr* C = S->getCond();
      FullSourceLoc condB(C->getLocStart(), SM);
      FullSourceLoc condE(C->getLocEnd(), SM);
      size_t posCondB = scan(SCAN_BACKWARD, "(", condB, false);
      size_t posCondE = scan(SCAN_FORWARD, ")", condE, false);
      oRanges.insert(oRanges.begin(),
                     OffsetRange(stmtToSymbolMap[C],
                                 posCondB,
                                 posCondE,
                                 condB.getBuffer()->getBufferIdentifier(),
                                 IFCONDITION));
      _debug("WhileStmt::IFCONDITION     - ");
      _debug(stmtToSymbolMap[C]);
      _debug("\n");
      
      Stmt* B = S->getBody();
      FullSourceLoc bodyB(B->getLocStart(), SM);
      FullSourceLoc bodyE(B->getLocEnd(), SM);
      size_t posBodyB = scan(SCAN_BACKWARD, "while", bodyB, true);
      size_t posBodyE = scan(SCAN_FORWARD, "}", bodyE, true);
      oRanges.insert(oRanges.begin(),
                     OffsetRange(stmtToSymbolMap[S],
                                 posBodyB,
                                 posBodyE,
                                 bodyB.getBuffer()->getBufferIdentifier(),
                                 STMT));
      _debug("WhileStmt::STMT            - ");
      _debug(stmtToSymbolMap[S]);
      _debug("\n");
      
      return oRanges;
    }
    
    OffsetRanges VisitForStmt(ForStmt * S)
    {
      OffsetRanges oRanges;
      
      Stmt* init = S->getInit();
      FullSourceLoc initB(init->getLocStart(), SM);
      FullSourceLoc initE(init->getLocEnd(), SM);
      size_t posInitB = scan(SCAN_BACKWARD, "(", initB, false);
      size_t posInitE = scan(SCAN_FORWARD, ";", initE, false);
      oRanges.insert(oRanges.begin(),
                     OffsetRange(stmtToSymbolMap[init],
                                 posInitB,
                                 posInitE,
                                 initB.getBuffer()->getBufferIdentifier(),
                                 INITIALIZER));
      _debug("ForStmt::INITIALIZER       - ");
      _debug(stmtToSymbolMap[init]);
      _debug("\n");
      
      Expr* C = S->getCond();
      FullSourceLoc ifConditionB(C->getLocStart(), SM);
      FullSourceLoc ifConditionE(C->getLocEnd(), SM);
      size_t conditionBegin = scan(SCAN_BACKWARD, ";", ifConditionB, false);
      size_t conditionEnd = scan(SCAN_FORWARD, ";", ifConditionE, false);
      oRanges.insert(oRanges.begin(),
                     OffsetRange(stmtToSymbolMap[C],
                                 conditionBegin,
                                 conditionEnd,
                                 ifConditionB.getBuffer()->getBufferIdentifier(),
                                 IFCONDITION));
      _debug("ForStmt::IFCONDITION       - ");
      _debug(stmtToSymbolMap[C]);
      _debug("\n");
      
      Expr* inc = S->getInc();
      FullSourceLoc incB(inc->getLocStart(), SM);
      FullSourceLoc incE(inc->getLocEnd(), SM);
      size_t posIncB = scan(SCAN_BACKWARD, ";", incB, false);
      size_t posIncE = scan(SCAN_FORWARD, ")", incE, false);
      oRanges.insert(oRanges.begin(),
                     OffsetRange(stmtToSymbolMap[inc],
                                 posIncB,
                                 posIncE,
                                 incB.getBuffer()->getBufferIdentifier(),
                                 EXPR));
      _debug("ForStmt::EXPR              - ");
      _debug(stmtToSymbolMap[inc]);
      _debug("\n");
      
      Stmt* B = S->getBody();
      FullSourceLoc bodyB(B->getLocStart(), SM);
      FullSourceLoc bodyE(B->getLocEnd(), SM);
      size_t posBodyB = scan(SCAN_BACKWARD, "for", bodyB, true);
      size_t posBodyE = scan(SCAN_FORWARD, "}", bodyE, true);
      oRanges.insert(oRanges.begin(),
                     OffsetRange(stmtToSymbolMap[S],
                                 posBodyB,
                                 posBodyE,
                                 bodyB.getBuffer()->getBufferIdentifier(),
                                 STMT));
      _debug("ForStmt::STMT              - ");
      _debug(stmtToSymbolMap[S]);
      _debug("\n");
      
      return oRanges;
    }
    
    OffsetRanges VisitIfStmt(IfStmt * S)
    {
      OffsetRanges oRanges;
      
      Expr* C = S->getCond();
      FullSourceLoc ifConditionB(C->getLocStart(), SM);
      FullSourceLoc ifConditionE(C->getLocEnd(), SM);
      size_t conditionBegin = scan(SCAN_BACKWARD, "(", ifConditionB, false);
      size_t conditionEnd = scan(SCAN_FORWARD, ")", ifConditionE, false);
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
      size_t ifBegin = scan(SCAN_BACKWARD, "if", ifBlockB, true);
      size_t ifEnd = scan(SCAN_FORWARD, "}", ifBlockE, true);
      oRanges.insert(oRanges.begin(),
                     OffsetRange(stmtToSymbolMap[S],
                                 ifBegin,
                                 ifEnd,
                                 ifBlockB.getBuffer()->getBufferIdentifier(),
                                 STMT));
      _debug("IfStmt::STMT               - ");
      _debug(stmtToSymbolMap[S]);
      _debug("\n");
      
      return oRanges;
    }
    
    OffsetRanges VisitCompoundStmt(CompoundStmt * S)
    {
      OffsetRanges oRanges;
      
      FullSourceLoc stmtBegin(S->getLocStart(), SM);
      FullSourceLoc stmtEnd(S->getLocEnd(), SM);
      size_t posBegin = scan(SCAN_BACKWARD, "{", stmtBegin, true);
      size_t posEnd = scan(SCAN_FORWARD, "}", stmtEnd, true);
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
                                 posEnd + 1, // TODO: check (SSS)
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
      size_t posEnd = scan(SCAN_FORWARD, ";", stmtEnd, true);
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
