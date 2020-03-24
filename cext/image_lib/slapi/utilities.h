#pragma once

#include <assert.h>
#include <vector>
#include <string>

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>

#include "trace.h"


namespace example {

#define SU(api_function_call) {\
  [[maybe_unused]] SUResult su_api_result = api_function_call;\
  if (SU_ERROR_NONE != su_api_result) { TRACE(L"\nSU_RESULT: %i\n", su_api_result); }\
  assert(SU_ERROR_NONE == su_api_result);\
}


std::string GetString(const SUStringRef& string);

} // namespace example
