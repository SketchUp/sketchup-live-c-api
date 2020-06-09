#include "example_config.h"
#include "example_export.h"

#include <array>
#include <algorithm>
#include <cassert>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include <SketchUpAPI/sketchup.h>

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


VALUE grey_scale(VALUE self, VALUE ruby_face_pid, VALUE ruby_amount, VALUE ruby_temp_path)
{
  SUModelRef model = SU_INVALID;
  SU(SUApplicationGetActiveModel(&model));
  if (SUIsInvalid(model)) {
    rb_raise(rb_eTypeError, "invalid model");
  }

  // Face PID parameter
  SUInstancePathRef face_path = GetRubyInstancePath(model, ruby_face_pid);

  SUEntityRef entity = SU_INVALID;
  SUInstancePathGetLeafAsEntity(face_path, &entity);

  SU(SUInstancePathRelease(&face_path));

  if (SUEntityGetType(entity) != SURefType_Face) {
    rb_raise(rb_eTypeError, "invalid face references");
  }

  //Amount parameter
  double amount = NUM2DBL(ruby_amount);

  // Output path parameter
  std::string output_path(RSTRING_PTR(ruby_temp_path));

  // Validate face parameter
  SUFaceRef face = SUFaceFromEntity(entity);

  SUMaterialRef material = SU_INVALID;
  SUFaceGetFrontMaterial(face, &material);

  SUMaterialType type = SUMaterialType_Colored;
  SU(SUMaterialGetType(material, &type));
  if (type == SUMaterialType_Colored) {
    rb_raise(rb_eArgError, "face has no texture");
    return Qnil;
  }

  // Blend texture towards greyscale.
  SUTextureRef texture = SU_INVALID;
  SU(SUMaterialGetTexture(material, &texture));

  SUImageRepRef image_rep = SU_INVALID;
  SU(SUImageRepCreate(&image_rep));
  SU(SUTextureGetColorizedImageRep(texture, &image_rep));

  auto success = GreyScaleCopy(image_rep, image_rep, amount);
  if (!success) { return Qfalse; }

  SUStringRef filename_ref = SU_INVALID;
  SU(SUStringCreate(&filename_ref));
  SU(SUTextureGetFileName(texture, &filename_ref));
  std::string texture_filename = GetString(filename_ref);
  SU(SUStringRelease(&filename_ref));

  SU(SUImageRepSaveToFile(image_rep, output_path.c_str()));

  SU(SUImageRepRelease(&image_rep));

  return Qtrue;
}


VALUE active_entities_num_faces(VALUE self) {
  auto model = GetActiveModel();
  SUEntitiesRef entities = SU_INVALID;
  SU(SUModelGetActiveEntities(model, &entities));
  size_t num_faces = 0;
  SU(SUEntitiesGetNumFaces(entities, &num_faces));
  return SIZET2NUM(num_faces);
}

VALUE active_path_pids(VALUE self) {
  auto model = GetActiveModel();

  SUInstancePathRef path = SU_INVALID;
  auto result = SUModelGetActivePath(model, &path);
  if (result == SU_ERROR_NO_DATA) {
    return Qnil;
  } else if (result != SU_ERROR_NONE) {
    rb_raise(rb_eRuntimeError, "unable to get active path");
  }

  size_t num_items = 0;
  SU(SUInstancePathGetPathDepth(path, &num_items));
  VALUE pids = rb_ary_new_capa(static_cast<long>(num_items));

  for (size_t i = 0; i < num_items; ++i) {
    SUComponentInstanceRef instance = SU_INVALID;
    SU(SUInstancePathGetInstanceAtDepth(path, i, &instance));
    auto entity = SUComponentInstanceToEntity(instance);

    int64_t pid = 0;
    SU(SUEntityGetPersistentID(entity, &pid));

    rb_ary_push(pids, LL2NUM(pid));
  }

  return pids;
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

  rb_define_module_function(mLiveCAPI, "grey_scale",
      VALUEFUNC(grey_scale), 3);

  rb_define_module_function(mLiveCAPI, "active_entities_num_faces",
      VALUEFUNC(active_entities_num_faces), 0);
  rb_define_module_function(mLiveCAPI, "active_path_pids",
      VALUEFUNC(active_path_pids), 0);
}

} // extern "C"
