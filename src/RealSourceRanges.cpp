#include <stdint.h>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>

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

  size_t scanBackTo(const std::string & keyword, FullSourceLoc sl)
  {
    assert(!keyword.empty());

    const llvm::MemoryBuffer *buf = sl.getBuffer();
    const char * bufferStart = buf->getBufferStart();
    const char * tokenStart = sl.getCharacterData();
    size_t startingIndex = tokenStart - bufferStart;

    size_t currentIndex = startingIndex;

    while(--currentIndex >= 0)
    {
      while(bufferStart[currentIndex] != keyword[0])
        --currentIndex;
      std::string maybeKeyword(bufferStart + currentIndex, keyword.size());

      if(maybeKeyword == keyword)
        return currentIndex;
    }

    throw TokenScanException(keyword, startingIndex, TokenScanException::SCAN_BACKWARD);
  }

  size_t scanForwardTo(const std::string & keyword, FullSourceLoc sl)
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
        return currentIndex;
    }

    throw TokenScanException(keyword, startingIndex, TokenScanException::SCAN_FORWARD);
  }

  class DeclSourceRangeVisitor : public DeclVisitor<DeclSourceRangeVisitor, OffsetRange>
  {
  public:
    DeclSourceRangeVisitor(SourceManager & SMan)
    :SM(SMan)
    {
    }

    OffsetRange VisitTypedefDecl(TypedefDecl *D)
    {
			FullSourceLoc definedTypeBegin(D->getLocation(), SM);
			
			size_t typedefBegin = scanBackTo("typedef", definedTypeBegin);
			size_t typedefEnd = scanForwardTo(";", definedTypeBegin);

      return std::make_pair(typedefBegin, typedefEnd);
    }

    OffsetRange VisitEnumDecl(EnumDecl *D)
    {
      SourceRange sr = D->getSourceRange();
      SourceLocation sBegin = SM.getSpellingLoc(sr.getBegin());
      SourceLocation sEnd = SM.getSpellingLoc(sr.getEnd());

      return std::make_pair(SM.getFileOffset(sBegin), SM.getFileOffset(sEnd));
    }

    OffsetRange VisitVarDecl(VarDecl *D)
    {
      SourceRange sr = D->getSourceRange();
      SourceLocation sBegin = SM.getSpellingLoc(sr.getBegin());
      SourceLocation sEnd = SM.getSpellingLoc(sr.getEnd());

      // SourceRange qr = D->DeclaratorDecl::getQualifierRange();
      SourceLocation qBegin = SM.getSpellingLoc(D->getTypeSpecStartLoc());//qr.getBegin());

      return std::make_pair(SM.getFileOffset(qBegin), SM.getFileOffset(sEnd));
    }

    OffsetRange VisitDecl(Decl *D)
    {
      throw DeclRangeCalculatorNotImplementedException(D);
    }

  private:
    SourceManager & SM;
  };
}

OffsetRange getRealSourceRange(SourceManager & SM, Decl *D)
{
  DeclSourceRangeVisitor dsrv(SM);
  return dsrv.Visit(D);
}
