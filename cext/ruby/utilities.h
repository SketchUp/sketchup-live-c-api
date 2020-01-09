#pragma once

#include <string>

#include "ruby/framework.h"


/*
 * Need to be very careful about how these macros are defined, especially
 * when compiling C++ code or C code with an ANSI C compiler.
 *
 * VALUEFUNC(f) is a macro used to typecast a C function that implements
 * a Ruby method so that it can be passed as an argument to API functions
 * like rb_define_method() and rb_define_singleton_method().
 *
 * VOIDFUNC(f) is a macro used to typecast a C function that implements
 * either the "mark" or "free" stuff for a Ruby Data object, so that it
 * can be passed as an argument to API functions like Data_Wrap_Struct()
 * and Data_Make_Struct().
 */

#define VALUEFUNC(f) ((VALUE (*)(ANYARGS)) f)
#define VOIDFUNC(f)  ((RUBY_DATA_FUNC) f)


namespace traceup {
namespace ruby {


VALUE GetVALUE(bool boolean);
VALUE GetVALUE(const char* string);
VALUE GetVALUE(const std::string& string);
VALUE GetVALUE(int number);
VALUE GetVALUE(unsigned int number);
VALUE GetVALUE(long number);
VALUE GetVALUE(long long number);
VALUE GetVALUE(unsigned long long number);
VALUE GetVALUE(double number);

template <typename Iter>
VALUE GetArrayVALUE(Iter it, Iter end) {
  //long num_items = static_cast<long>(it->size());
  //VALUE ruby_array = rb_ary_new2(num_items); // rb_ary_new_capa
  VALUE ruby_array = rb_ary_new();
  for (; it != end; ++it)
  {
    rb_ary_push(ruby_array, GetVALUE(*it));
  }
  return ruby_array;
}

#ifdef __clang__
VALUE GetVALUE(const size_t number);
#endif

VALUE GetBooleanVALUE(VALUE value);

bool IsBoolean(VALUE object);


} // namespace ruby
} // namespace traceup
