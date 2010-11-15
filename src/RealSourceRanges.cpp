#include <stdint.h>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
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
	  dir = "forward"; break;
	case SCAN_BACKWARD:
	  dir = "backward"; break;
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
	  --currentIndex;
	std::string maybeKeyword(bufferStart + currentIndex, keyword.size());

	if(maybeKeyword == keyword)
	  return (isInclusive ? currentIndex : currentIndex + keyword.length());
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
	  ++currentIndex;
	std::string maybeKeyword(bufferStart + currentIndex, keyword.size());
	if(maybeKeyword == keyword)
	  return (isInclusive ? currentIndex + keyword.length() : currentIndex);
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

    OffsetRanges VisitTypedefDecl(TypedefDecl *D)
    {
      FullSourceLoc definedTypeBegin(D->getLocation(), SM);
			
      size_t typedefBegin = scanBackTo("typedef", definedTypeBegin);
      size_t typedefEnd = scanForwardTo(";", definedTypeBegin);

      OffsetRanges oRanges;
      oRanges.insert(oRanges.begin(), OffsetRange(typedefBegin, typedefEnd, definedTypeBegin.getBuffer()->getBufferIdentifier(),DECL));
			
      return oRanges;
    }

    OffsetRanges VisitEnumDecl(EnumDecl *D)
    {
      FullSourceLoc definedTypeBegin(D->getLocation(), SM);
      
      size_t typedefBegin = scanBackTo("enum", definedTypeBegin);
      size_t typedefEnd = scanForwardTo(";", definedTypeBegin);
      
      OffsetRanges oRanges;
      oRanges.insert(oRanges.begin(), OffsetRange(typedefBegin, typedefEnd, definedTypeBegin.getBuffer()->getBufferIdentifier(), DECL));
      return oRanges;
      
    }

    OffsetRanges VisitRecordDecl(RecordDecl *D)
    {
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
      
      OffsetRanges oRanges;
      oRanges.insert(oRanges.begin(), OffsetRange(typedefBegin, typedefEnd, definedTypeBegin.getBuffer()->getBufferIdentifier(), DECL));
      return oRanges;
      
    }    

    OffsetRanges VisitVarDecl(VarDecl *D)
    {
      SourceRange sr = D->getSourceRange();
      SourceLocation sBegin = SM.getSpellingLoc(sr.getBegin());
      SourceLocation sEnd = SM.getSpellingLoc(sr.getEnd());

      // SourceRange qr = D->DeclaratorDecl::getQualifierRange();
      SourceLocation qBegin = SM.getSpellingLoc(D->getTypeSpecStartLoc());//qr.getBegin());

      OffsetRanges oRanges;
      oRanges.insert(oRanges.begin(), OffsetRange(SM.getFileOffset(qBegin), SM.getFileOffset(sEnd), SM.getBufferName(sBegin), DECL));
      return oRanges;
      
    }

    OffsetRanges VisitDecl(Decl *D)
    {
      throw DeclRangeCalculatorNotImplementedException(D);
    }   


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

    OffsetRanges VisitIfStmt(IfStmt *S)
    {
      OffsetRanges oRanges;

      const Expr* C = S->getCond();
      FullSourceLoc ifConditionB(C->getLocStart(), SM);
      FullSourceLoc ifConditionE(C->getLocEnd(), SM);
      size_t conditionBegin = scanBackTo("(", ifConditionB, false);
      size_t conditionEnd = scanForwardTo(")", ifConditionE, false);
      oRanges.insert(oRanges.begin(), OffsetRange(conditionBegin, conditionEnd, ifConditionB.getBuffer()->getBufferIdentifier(), IFCONDITION));
      
      FullSourceLoc thenLoc(S->getThen()->getLocStart(), SM);
      FullSourceLoc elseLoc((S->getElse()==NULL ? S->getThen() : S->getElse())->getLocEnd(), SM);
      size_t ifBegin = scanBackTo("if", thenLoc, true);
      size_t ifEnd = scanForwardTo("}", elseLoc, true);
      oRanges.insert(oRanges.begin(), OffsetRange(ifBegin, ifEnd, thenLoc.getBuffer()->getBufferIdentifier(), STMT));

      return oRanges;      
    }

    OffsetRange ExprVisitor(Expr *E, std::string leftBoundary, bool leftInclusive, std::string rightBoundary, bool rightInclusive)
    {
      OffsetRanges oRanges;
      
      SourceRange sr = E->getSourceRange();
      FullSourceLoc exprB(sr.getBegin(), SM);
      FullSourceLoc exprE(sr.getEnd(), SM);
      size_t exprBegin = scanBackTo(leftBoundary, exprB, leftInclusive);
      size_t exprEnd = scanBackTo(rightBoundary, exprE, rightInclusive);
      
      return OffsetRange(exprBegin, exprEnd, exprB.getBuffer()->getBufferIdentifier(), EXPR);
    }
    
    
  private:
    SourceManager & SM;
  };

}

OffsetRanges getRealSourceRange(SourceManager & SM, Decl *D)
{
  DeclSourceRangeVisitor dsrv(SM);
  return dsrv.Visit(D);
}

OffsetRanges getRealSourceRange(SourceManager & SM, Stmt *S)
{
  StmtSourceRangeVisitor ssrv(SM);
  return ssrv.Visit(S);
}
