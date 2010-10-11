#ifndef __REAL__SOURCE__RANGES__HPP
#define __REAL__SOURCE__RANGES__HPP

#include <utility>

namespace clang
{
  class SourceManager;
  class Decl;
}

typedef std::pair<size_t, size_t> OffsetRange;

OffsetRange getRealSourceRange(clang::SourceManager & SM, clang::Decl *D);

#endif // __REAL__SOURCE__RANGES__HPP
