#include "example_config.hpp"
#include "example_export.h"

#include <array>
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include <SketchUpAPI/sketchup.h>

#include "ruby/framework.h"
#include "ruby/utilities.h"

// #include "geom/polygon2d.hpp"
// #include "geom/point2d.hpp"
// #include "example.hpp"
#include "slapi/color.hpp"
#include "slapi/utilities.hpp"
#include "image.hpp"


namespace example {
namespace ruby {
namespace {

VALUE new_ruby_image_rep() {
  static VALUE mSketchup = rb_define_module("Sketchup");
  static VALUE cImageRep = rb_const_get(mSketchup, rb_intern("ImageRep"));
  VALUE image_rep = rb_class_new_instance(0, nullptr, cImageRep);
  return image_rep;
}

// VALUE RubyInterface(const Point2D& point) {
//   static VALUE mGeom = rb_define_module("Geom");
//   static VALUE cPoint3d = rb_const_get(mGeom, rb_intern("Point3d"));
//   VALUE args[2] { INT2NUM(point.x), INT2NUM(point.y) };
//   VALUE instance = rb_class_new_instance(2, args, cPoint3d);
//   return instance;
// }

std::string FileExtension(const std::string filename)
{
  const auto index = filename.rfind('.');
  if (index == std::string::npos) {
    return "";
  }
  return filename.substr(index);
}

std::string UniqueFilename(
  const std::string& output_path,
  const std::string& texture_filename)
{
  assert(texture_filename.size() > 0);
  std::string filename = output_path + "\\" + texture_filename;

  // TODO: Separate out basename and file extension. Then do this loop.
  // right-find . and get trailing part -> file extension.
  if (texture_filename.at(0) == '.') {
    size_t i = 0;
    while (std::filesystem::exists(filename)) {
      filename = output_path + "Untitled" + std::to_string(i) + texture_filename;
    }
  }

  assert(!std::filesystem::exists(filename));
  return filename;
}

enum class FilterType {
  GreyScale,
  ChromaticAberration
};

FilterType GetRubyFilterType(VALUE ruby_filter_type) {
  Check_Type(ruby_filter_type, T_SYMBOL);
  const ID filter_type = SYM2ID(ruby_filter_type);
  if (filter_type == rb_intern("grey_scale")) {
    return FilterType::GreyScale;
  } else if (filter_type == rb_intern("chromatic_aberration")) {
    return FilterType::ChromaticAberration;
  } else {
    rb_raise(rb_eArgError, "invalid filter type");
  }
}

struct Point2d {
  int x;
  int y;
};

struct GreyScaleOptions {
  float amount;
};

struct ChromaticAberrationOptions {
  Point2d offsets[4];
};

union FilterOptionsValue {
  GreyScaleOptions grey_scale;
  ChromaticAberrationOptions chromatic_aberration;
};

struct FilterOptions {
  FilterType type;
  FilterOptionsValue value;
};

} // namespace


// Disable the warnings for "unreferenced formal parameter" due to Ruby's
// required `self` argument which is often unused.
#pragma warning (push)
#pragma warning (disable : 4100)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"


// Simple example processing the active model.
VALUE num_faces(VALUE self)
{
  SUModelRef model = SU_INVALID;
  SU(SUApplicationGetActiveModel(&model));
  if (SUIsInvalid(model)) {
    return Qnil;
  }

  SUEntitiesRef entities = SU_INVALID;
  SU(SUModelGetEntities(model, &entities));

  size_t num_faces = 0;
  SUEntitiesGetNumFaces(entities, &num_faces);

  return SIZET2NUM(num_faces);
}

VALUE image_filter(VALUE self, VALUE ruby_face_pid, VALUE ruby_path, VALUE ruby_options)
{
  SUModelRef model = SU_INVALID;
  SU(SUApplicationGetActiveModel(&model));
  if (SUIsInvalid(model)) {
    rb_raise(rb_eTypeError, "invalid model");
    return Qnil;
  }


  SUInstancePathRef face_path = GetRubyInstancePath(ruby_face_pid);

  SUEntityRef entity = SU_INVALID;
  SUInstancePathGetLeafAsEntity(face_path, &entity);

  if (SUEntityGetType(entity) != SURefType_Face) {
    rb_raise(rb_eTypeError, "invalid face references");
    return Qnil;
  }


  std::string output_path(RSTRING_PTR(ruby_path));


  Check_Type(ruby_options, T_HASH);

  FilterOptions options;

  VALUE ruby_filter_type = rb_hash_aref(ruby_options, ID2SYM(rb_intern("filter_type")));
  options.type = GetRubyFilterType(ruby_filter_type);

  std::function<Color(const SUColor& color)> filter;
  switch (options.type) {
  case FilterType::GreyScale:
    filter = [](const SUColor& color) -> Color {
      const auto luminance = Luminance(color);
      return { luminance, luminance, luminance, color.alpha };
    };
    break;
  case FilterType::ChromaticAberration:
    VALUE ruby_offsets = rb_hash_aref(ruby_options, ID2SYM(rb_intern("filter_type")));
    Check_Type(ruby_offsets, T_ARRAY);
    if (RARRAY_LEN(ruby_offsets) != 3) {
      rb_raise(rb_eArgError, "unexpected offsets size");
    }
    VALUE* ruby_offsets_array = RARRAY_PTR(ruby_offsets);
    std::array<Point2d, 3> offsets;
    for (size_t i = i; i < 3; ++i) {
      offsets[i] = GetRubyPoint2d(ruby_offsets_array[i]);
    }
    options.value.chromatic_aberration.offsets = offsets.data();

    // TODO: get offsets per color channel
    filter = [&options](const SUColor& color) -> Color {
      const auto luminance = ChromaticAberration(color, options.value.chromatic_aberration);
      return { luminance, luminance, luminance, color.alpha };
    };
    break;
  }


  SUFaceRef face = SUFaceFromEntity(entity);

  SUMaterialRef material = SU_INVALID;
  SUFaceGetFrontMaterial(face, &material);

  SUMaterialType type = SUMaterialType_Colored;
  SU(SUMaterialGetType(material, &type));
  if (type == SUMaterialType_Colored) {
    rb_raise(rb_eArgError, "face has no texture");
    return Qnil;
  }


  SUTextureRef texture = SU_INVALID;
  SU(SUMaterialGetTexture(material, &texture));

  SUImageRepRef image_rep = SU_INVALID;
  SU(SUTextureGetColorizedImageRep(texture, &image_rep));

  size_t data_size = 0, bits_per_pixel = 0;
  SU(SUImageRepGetDataSize(image_rep, &data_size, &bits_per_pixel));

  size_t width = 0, height = 0;
  SU(SUImageRepGetPixelDimensions(image_rep, &width, &height));

  size_t num_pixels = width * height;
  std::vector<SUColor> colors(num_pixels);
  SU(SUImageRepGetDataAsColors(image_rep, colors.data()));

  // Note: using `Color` here and not `SUColor` as it helps in serializing the
  // color data to a byte buffer compatible with the platform specific order.
  std::vector<Color> result(num_pixels);
  std::transform(begin(colors), end(colors), begin(result), filter);

  auto buffer = reinterpret_cast<SUByte*>(result.data());
  SU(SUImageRepSetData(image_rep, width, height, 32, 0, buffer));

  SUStringRef filename_ref = SU_INVALID;
  SU(SUTextureGetFileName(texture, &filename_ref));
  std::string texture_filename = GetString(filename_ref);
  SU(SUStringRelease(&filename_ref));

  std::string output_filepath = UniqueFilename(output_path, texture_filename);

  SU(SUImageRepSaveToFile(image_rep, output_filepath.c_str()));

  SU(SUImageRepRelease(&image_rep));

  return Qtrue;
}

// Advanced example where data is exchanged between Ruby API objects and C API
// objects.
// NOTE: Exchanging data like is not officially part of the API contract. It's
//       implementation details that happen to work.
VALUE convert_to_greyscale(VALUE self, VALUE image_rep)
{
  // A bit nasty this, relying on knowledge of internal implementation details
  // of the SketchUp Ruby API and the SketchUp C API - passing void pointers
  // around.
  void* data = rb_data_object_get(image_rep);
  SUImageRepRef image_rep_input = *reinterpret_cast<SUImageRepRef*>(data);

  // Create a Ruby ImageRep, but use the C API to populate the data for
  // performance reasons. This is taking advantage of implementation details of
  // both APIs. The Ruby API and the C API both wraps the same data type.
  VALUE ruby_image_rep_output = new_ruby_image_rep();
  void* out_data = rb_data_object_get(ruby_image_rep_output);
  SUImageRepRef image_rep_output = *reinterpret_cast<SUImageRepRef*>(out_data);

  GreyScaleCopy(image_rep_input, image_rep_output);

  return ruby_image_rep_output;
}


#pragma clang diagnostic pop
#pragma warning (pop)

} // namespace ruby
} // namespace example


extern "C" {

EXAMPLE_EXPORT void Init_example()
{
  using namespace example::ruby;

  VALUE mExample = rb_define_module("Example");

  rb_define_const(mExample, "CEXT_VERSION", GetVALUE(EXAMPLE_VERSION));

  rb_define_module_function(mExample, "num_faces",
      VALUEFUNC(num_faces), 0);
  rb_define_module_function(mExample, "image_filter",
      VALUEFUNC(image_filter), 2);
}

} // extern "C"
