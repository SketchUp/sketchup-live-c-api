#include "trace.hpp"

namespace example {

// TODO: Change to detect Windows instead.
#ifdef __APPLE__


bool _trace(...)
{
  return false;
}


#else // #ifdef __APPLE__


#include <Strsafe.h>


#ifdef _DEBUG
bool _trace(LPTSTR format, ...)
{
  TCHAR buffer[1000];

  va_list argptr;
  va_start(argptr, format);
  StringCchVPrintf(buffer, 1000, format, argptr);
  va_end(argptr);

  OutputDebugString(buffer);

  return true;
}
#endif


#endif // #ifndef __APPLE__

} // namespace example
