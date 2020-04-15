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


////////////////////////////////////////////////////////////////////////////////

SUModelRef GetActiveModel() {
  SUModelRef model = SU_INVALID;
  SU(SUApplicationGetActiveModel(&model));
  if (SUIsInvalid(model)) {
    rb_raise(rb_eTypeError, "invalid model");
  }
  return model;
}

SUSelectionRef GetSelection() {
  SUModelRef model = GetActiveModel();
  SUSelectionRef selection = SU_INVALID;
  SUModelGetSelection(model, &selection);
  return selection;
}

std::vector<SUDrawingElementRef> GetRubyDrawingElements(VALUE elements) {
  SUModelRef model = GetActiveModel();

  Check_Type(elements, T_ARRAY);
  auto num_elements = RARRAY_LEN(elements);

  std::vector<int64_t> pids;
  pids.reserve(num_elements);

  static ID id_pid = rb_intern("persistent_id");
  auto elements_ptr = RARRAY_PTR(elements);
  for (long i = 0; i < num_elements; ++i) {
    VALUE element = elements_ptr[i];
    VALUE ruby_pid = rb_funcall2(element, id_pid, 0, nullptr);
    int64_t pid = NUM2LL(ruby_pid);
    pids.push_back(pid);
  }

  std::vector<SUEntityRef> entities(num_elements, SU_INVALID);
  SUModelGetEntitiesByPersistentIDs(model, num_elements, pids.data(), entities.data());

  std::vector<SUDrawingElementRef> list(num_elements, SU_INVALID);
  std::transform(entities.begin(), entities.end(), list.begin(),
    [](const SUEntityRef& entity) {
      return SUDrawingElementFromEntity(entity);
    });

  return list;
}

VALUE selection_add(VALUE self, VALUE elements) {
  auto ref_elements = GetRubyDrawingElements(elements);
  auto selection = GetSelection();
  SUSelectionAdd(selection, ref_elements.size(), ref_elements.data());
  return Qnil;
}

VALUE selection_remove(VALUE self, VALUE elements) {
  auto ref_elements = GetRubyDrawingElements(elements);
  auto selection = GetSelection();
  SUSelectionRemove(selection, ref_elements.size(), ref_elements.data());
  return Qnil;
}

VALUE selection_toggle(VALUE self, VALUE elements) {
  auto ref_elements = GetRubyDrawingElements(elements);
  auto selection = GetSelection();
  SUSelectionToggle(selection, ref_elements.size(), ref_elements.data());
  return Qnil;
}

VALUE selection_clear(VALUE self) {
  auto selection = GetSelection();
  SUSelectionClear(selection);
  return Qnil;
}

VALUE selection_invert(VALUE self) {
  auto selection = GetSelection();
  SUSelectionInvert(selection);
  return Qnil;
}

VALUE selection_is_curve(VALUE self) {
  auto selection = GetSelection();
  bool is_curve = false;
  SUSelectionIsCurve(selection, &is_curve);
  return GetVALUE(is_curve);
}

VALUE selection_is_surface(VALUE self) {
  auto selection = GetSelection();
  bool is_surface = false;
  SUSelectionIsSurface(selection, &is_surface);
  return GetVALUE(is_surface);
}

VALUE selection_is_object(VALUE self) {
  auto selection = GetSelection();
  bool is_object = false;
  SUSelectionIsSingleObject(selection, &is_object);
  return GetVALUE(is_object);
}

VALUE selection_size(VALUE self) {
  auto selection = GetSelection();
  size_t size = false;
  SUSelectionGetNumElements(selection, &size);
  return SIZET2NUM(size);
}

VALUE selection_to_a(VALUE self) {
  auto selection = GetSelection();

  SUEntityListRef list = SU_INVALID;
  SUEntityListCreate(&list);
  SUSelectionGetEntityList(selection, list);

  size_t len = 0;
  SUEntityListSize(list, &len);

  VALUE pids = rb_ary_new_capa(static_cast<long>(len));

  SUEntityListIteratorRef it = SU_INVALID;
  SUEntityListIteratorCreate(&it);

  SUEntityRef entity = SU_INVALID;
  SUEntityListBegin(list, &it);

  while(SUEntityListIteratorGetEntity(it, &entity) == SU_ERROR_NONE) {
    int64_t pid = 0;
    SUEntityGetPersistentID(entity, &pid);
    VALUE ruby_pid = LL2NUM(pid);
    rb_ary_push(pids, ruby_pid);

    SUEntityListIteratorNext(it);
  }

  SUEntityListIteratorRelease(&it);
  SUEntityListRelease(&list);

  VALUE mSketchup = rb_const_get(rb_cObject, rb_intern("Sketchup"));
  VALUE active_model = rb_funcall(mSketchup, rb_intern("active_model"), 0, NULL);
  VALUE ruby_entities = rb_funcall(active_model, rb_intern("find_entity_by_persistent_id"), 1, pids);

  return ruby_entities;
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


  VALUE mSelection = rb_define_module_under(mLiveCAPI, "Selection");
  rb_define_module_function(mSelection, "add", VALUEFUNC(selection_add), 1);
  rb_define_module_function(mSelection, "remove", VALUEFUNC(selection_remove), 1);
  rb_define_module_function(mSelection, "toggle", VALUEFUNC(selection_toggle), 1);
  rb_define_module_function(mSelection, "clear", VALUEFUNC(selection_clear), 0);
  rb_define_module_function(mSelection, "invert", VALUEFUNC(selection_invert), 0);
  rb_define_module_function(mSelection, "curve?", VALUEFUNC(selection_is_curve), 0);
  rb_define_module_function(mSelection, "surface?", VALUEFUNC(selection_is_surface), 0);
  rb_define_module_function(mSelection, "single_object?", VALUEFUNC(selection_is_object), 0);
  rb_define_module_function(mSelection, "size", VALUEFUNC(selection_size), 0);
  rb_define_module_function(mSelection, "to_a", VALUEFUNC(selection_to_a), 0);
}

} // extern "C"
