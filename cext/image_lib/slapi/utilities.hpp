#pragma once

#include <assert.h>
#include <vector>
#include <string>

#include <SketchUpAPI/common.h>


#define SU(api_function_call) {\
  [[maybe_unused]] SUResult su_api_result = api_function_call;\
  assert(SU_ERROR_NONE == su_api_result);\
}

namespace example {

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
