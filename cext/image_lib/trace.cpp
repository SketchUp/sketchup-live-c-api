#include "trace.h"

namespace example {

#ifdef _WIN32

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

#else // #ifndef _WIN32

  bool _trace(...)
  {
    return false;
  }

#endif

} // namespace example
