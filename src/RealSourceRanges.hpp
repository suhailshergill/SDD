#ifndef __REAL__SOURCE__RANGES__HPP
#define __REAL__SOURCE__RANGES__HPP

#include <utility>
#include <map>

namespace clang
{
  class SourceManager;
  class Decl;
}

enum RangeQualifier
{
  DECL, IFCONDITION, STMT, COMPOUNDSTMT, INITIALIZER, EXPR
};

class OffsetRange
{
public:
  OffsetRange(std::string symbol, size_t begin, size_t end, const char* fileName, RangeQualifier rangeType)
  {
    this->symbol = symbol;
    this->begin = begin;
    this->end = end;
    this->fileName = fileName;
    this->rangeType = rangeType;
  }
  
  std::string getSymbol()
  {
    return symbol;
  }
  size_t getBegin() 
  {
    return begin;
  }
  size_t getEnd()
  {
    return end;
  }
  const char* getFileName()
  {
    return fileName;
  }
  RangeQualifier getRangeType()
  {
    return rangeType;
  }
  
private:
  std::string symbol;
  size_t begin;
  size_t end;
  const char* fileName;
  RangeQualifier rangeType;
};

inline void _debug(std::string s) { std::cerr << s; std::cerr.flush(); }

typedef std::map<clang::Decl *, std::string> DeclToSymMap;
typedef std::map<clang::Stmt *, std::string> StmtToSymMap;
typedef std::vector<OffsetRange> OffsetRanges;
typedef std::map<RangeQualifier, std::string> RangeKindToGUIDMap;

OffsetRanges getRealSourceRange(clang::SourceManager & SM, clang::Decl *D, DeclToSymMap map);
OffsetRanges getRealSourceRange(clang::SourceManager & SM, clang::Stmt *D, StmtToSymMap map);

#endif // __REAL__SOURCE__RANGES__HPP
