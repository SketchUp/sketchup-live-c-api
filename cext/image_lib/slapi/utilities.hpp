#pragma once

#include <assert.h>
#include <vector>
#include <string>

#include <SketchUpAPI/common.h>

#include "trace.hpp"


namespace example {

#define SU(api_function_call) {\
  [[maybe_unused]] SUResult su_api_result = api_function_call;\
  if (SU_ERROR_NONE != su_api_result) { TRACE(L"\nSU_RESULT: %i\n", su_api_result); }\
  assert(SU_ERROR_NONE == su_api_result);\
}


std::string GetString(const SUStringRef& string) {
  size_t length = 0;
  SU(SUStringGetUTF8Length(string, &length));
  std::vector<char> buffer(length + 1);
  size_t out_length = 0;
  SU(SUStringGetUTF8(string, length + 1, buffer.data(), &out_length));
  assert(out_length == length);
  return std::string(begin(buffer), end(buffer));
}

} // namespace example
