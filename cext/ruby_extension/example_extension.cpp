#include "example_config.h"
#include "example_export.h"

#include <assert.h>
#include <array>
#include <algorithm>
#include <cassert>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include <SketchUpAPI/sketchup.h>
#include <SketchUpAPI/application/ruby_api.h>

#include "ruby/framework.h"
#include "utilities.h"

#include "slapi/color.h"
#include "slapi/utilities.h"
#include "image.h"


namespace example {
namespace ruby {
namespace {

SUModelRef GetActiveModel() {
  SUModelRef model = SU_INVALID;
  SU(SUApplicationGetActiveModel(&model));
  if (SUIsInvalid(model)) {
    rb_raise(rb_eTypeError, "invalid model");
  }
  return model;
}

SUInstancePathRef GetRubyInstancePath(SUModelRef model, VALUE ruby_pid)
{
  Check_Type(ruby_pid, T_STRING);

  const char* pid = RSTRING_PTR(ruby_pid);

  SUStringRef pid_ref = SU_INVALID;
  SU(SUStringCreateFromUTF8(&pid_ref, pid));

  SUInstancePathRef instance_path = SU_INVALID;
  SU(SUInstancePathCreate(&instance_path));
  SU(SUModelGetInstancePathByPid(model, pid_ref, &instance_path));

  SU(SUStringRelease(&pid_ref));

  return instance_path;
}

} // namespace


// Disable the warnings for "unreferenced formal parameter" due to Ruby's
// required `self` argument which is often unused.
#pragma warning (push)
#pragma warning (disable : 4100)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"


VALUE grey_scale(VALUE self, VALUE ruby_image_rep, VALUE ruby_amount)
{
  SUModelRef model = SU_INVALID;
  SU(SUApplicationGetActiveModel(&model));
  if (SUIsInvalid(model)) {
    rb_raise(rb_eTypeError, "invalid model");
  }

  // ImageRep parameter
  SUImageRepRef image_rep = SU_INVALID;
  if (SU_ERROR_NONE != SUImageRepFromRuby(ruby_image_rep, &image_rep)) {
    rb_raise(rb_eTypeError, "invalid imagerep");
  }
  if (SUIsInvalid(image_rep)) {
    rb_raise(rb_eTypeError, "invalid imagerep");
  }

  // Amount parameter
  double amount = NUM2DBL(ruby_amount);
  if (amount < 0.0 || amount > 1.0) {
    rb_raise(rb_eTypeError, "amount must be between 0.0 and 1.0");
  }

  // Blend image towards greyscale.
  SUImageRepRef image_rep_out = SU_INVALID;
  SUImageRepCreate(&image_rep_out);
  auto success = GreyScaleCopy(image_rep, amount, image_rep_out);
  assert(success); // If this fails it would be a bug.
  if (!success) {
    SUImageRepRelease(&image_rep_out);
    rb_raise(rb_eRuntimeError, "unable to process image");
  }

  // Return imagerep to Ruby
  VALUE ruby_result = Qnil;
  SUImageRepToRuby(image_rep_out, &ruby_result);
  return ruby_result;
}


#pragma clang diagnostic pop
#pragma warning (pop)

} // namespace ruby
} // namespace example


extern "C" {

EXAMPLE_EXPORT void Init_example()
{
  using namespace example::ruby;

  VALUE mExamples = rb_define_module("Examples");
  VALUE mLiveCAPI = rb_define_module_under(mExamples, "LiveCAPI");

  rb_define_const(mLiveCAPI, "CEXT_VERSION", GetVALUE(EXAMPLE_VERSION));

  rb_define_module_function(mLiveCAPI, "grey_scale", VALUEFUNC(grey_scale), 2);
}

} // extern "C"
