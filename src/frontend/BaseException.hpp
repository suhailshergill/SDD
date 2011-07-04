#ifndef __BASE__EXCEPTION__HPP
#define __BASE__EXCEPTION__HPP

#include <stdlib.h>
#include <string.h>
#include <exception>
#include <string>

struct BaseException : public std::exception
{
  BaseException()
  :msg(NULL)
  {
  }

  BaseException(const std::string & str)
  :msg(strdup(str.c_str()))
  {
  }

  virtual ~BaseException() throw()
  {
    free(msg);
  }

  virtual const char * what() throw()
  {
    return msg;
  }

protected:
  void setMessage(const char * str)
  {
    if(msg) free(msg);
    msg = strdup(str);
  }

  void setMessage(const std::string & str)
  {
    if(msg) free(msg);
    msg = strdup(str.c_str());
  }

private:
  char * msg;
};

#endif // __BASE__EXCEPTION__HPP
