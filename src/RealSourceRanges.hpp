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
    DECL, IFCONDITION, STMT, COMPOUNDSTMT, INITIALIZER,
  };
  
class OffsetRange
{
public:
  OffsetRange(size_t begin, size_t end, const char* fileName, RangeQualifier rangeType)
  {
    this->begin = begin;
    this->end = end;
    this->fileName = fileName;
    this->rangeType = rangeType;
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
  size_t begin;
  size_t end;
  const char* fileName;
  RangeQualifier rangeType;
};  

typedef std::vector<OffsetRange> OffsetRanges;
typedef std::map<RangeQualifier, std::string> RangeKindToGUIDMap;

OffsetRanges getRealSourceRange(clang::SourceManager & SM, clang::Decl *D);
OffsetRanges getRealSourceRange(clang::SourceManager & SM, clang::Stmt *D);

#endif // __REAL__SOURCE__RANGES__HPP
