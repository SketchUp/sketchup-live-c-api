#include "example_config.hpp"
#include "example_export.h"

#include <algorithm>
#include <cassert>
#include <filesystem>
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

  return filename;
}

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

VALUE export_textures(VALUE self, VALUE ruby_path, VALUE ruby_greyscale)
{
  SUModelRef model = SU_INVALID;
  SU(SUApplicationGetActiveModel(&model));
  if (SUIsInvalid(model)) {
    rb_raise(rb_eTypeError, "invalid model");
    return Qnil;
  }

  std::string output_path(RSTRING_PTR(ruby_path));

  bool greyscale = RTEST(ruby_greyscale);

  size_t num_materials = 0;
  SU(SUModelGetNumMaterials(model, &num_materials));
  std::vector<SUMaterialRef> materials(num_materials, SU_INVALID);
  SU(SUModelGetMaterials(model, materials.size(), materials.data(), &num_materials));

  for (const auto& material : materials) {
    SUMaterialType type = SUMaterialType_Colored;
    SU(SUMaterialGetType(material, &type));
    if (type == SUMaterialType_Colored) {
      continue;
    }

    SUTextureRef texture = SU_INVALID;
    SU(SUMaterialGetTexture(material, &texture));

    SUImageRepRef image_rep = SU_INVALID;
    SU(SUTextureGetColorizedImageRep(texture, &image_rep));

    if (greyscale) {
      size_t data_size = 0, bits_per_pixel = 0;
      SU(SUImageRepGetDataSize(image_rep, &data_size, &bits_per_pixel));

      size_t width = 0, height = 0;
      SU(SUImageRepGetPixelDimensions(image_rep, &width, &height));

      size_t num_pixels = width * height;
      std::vector<SUColor> colors(num_pixels);
      SU(SUImageRepGetDataAsColors(image_rep, colors.data()));

      // Note: using `Color` here and not `SUColor` as it helps in serializing the
      // color data to a byte buffer compatible with the platform specific order.
      std::vector<Color> greyscale(num_pixels);
      std::transform(begin(colors), end(colors), begin(greyscale),
        [](const SUColor& color) -> Color {
          const auto luminance = Luminance(color);
          return { luminance, luminance, luminance, color.alpha };
        }
      );

      auto buffer = reinterpret_cast<SUByte*>(greyscale.data());
      SU(SUImageRepSetData(image_rep, width, height, 32, 0, buffer));
    }

    SUStringRef filename_ref = SU_INVALID;
    SU(SUTextureGetFileName(texture, &filename_ref));
    std::string texture_filename = GetString(filename_ref);
    SU(SUStringRelease(&filename_ref));

    std::string output_filepath = UniqueFilename(output_path, texture_filename);

    SU(SUImageRepSaveToFile(image_rep, output_filepath.c_str()));

    SU(SUImageRepRelease(&image_rep));
  }

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
  rb_define_module_function(mExample, "export_textures",
      VALUEFUNC(export_textures), 2);
}

} // extern "C"
