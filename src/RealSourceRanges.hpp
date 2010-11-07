#ifndef __REAL__SOURCE__RANGES__HPP
#define __REAL__SOURCE__RANGES__HPP

#include <utility>

namespace clang
{
  class SourceManager;
  class Decl;
}

// typedef std::pair<size_t, size_t> OffsetRange;

class OffsetRange
{
public:
  OffsetRange(size_t begin, size_t end, const char* fileName)
  {
    this->begin = begin;
    this->end = end;
    this->fileName = fileName;
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
    
private:
  size_t begin;
  size_t end;
  const char* fileName;
};  

OffsetRange getRealSourceRange(clang::SourceManager & SM, clang::Decl *D);

#endif // __REAL__SOURCE__RANGES__HPP
