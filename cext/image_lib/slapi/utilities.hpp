#pragma once

#include <SketchUpAPI/common.h>


#define SU(api_function_call) {\
  [[maybe_unused]] SUResult su_api_result = api_function_call;\
  assert(SU_ERROR_NONE == su_api_result);\
}
