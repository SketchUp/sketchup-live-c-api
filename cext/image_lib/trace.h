#pragma once

namespace example {

#ifdef __APPLE__

// http://stackoverflow.com/a/1644898/486990
// http://stackoverflow.com/a/1306690/486990

bool _trace(...);
#define __noop do {} while (0)

#define TRACE(...) __noop

#else // #ifdef __APPLE__

// Must disable the min/max macros defined by windows.h to avoid conflict with
// std::max and std:min.
// http://stackoverflow.com/a/2789509/486990
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>


#ifdef _DEBUG

bool _trace(LPTSTR format, ...);
#define TRACE _trace

#else // #ifdef _DEBUG

#define TRACE __noop

#endif // #ifdef _DEBUG


#endif // #ifdef __APPLE__

} // namespace example
