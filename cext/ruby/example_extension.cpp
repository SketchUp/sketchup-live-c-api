#include "example_config.hpp"
#include "example_export.h"

#include <cassert>
#include <vector>

#include <SketchUpAPI/sketchup.h>

#include "ruby/framework.h"
#include "ruby/utilities.h"

// #include "geom/polygon2d.hpp"
// #include "geom/point2d.hpp"
// #include "example.hpp"
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

} // namespace


// Disable the warnings for "unreferenced formal parameter" due to Ruby's
// required `self` argument which is often unused.
#pragma warning (push)
#pragma warning (disable : 4100)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"


VALUE num_faces(VALUE self)
{
  return Qnil;
}

VALUE convert_to_greyscale(VALUE self)
{
  return Qnil;
}

// VALUE trace_map(VALUE self, VALUE image_rep, VALUE threshold) {
//   size_t trace_threshold = NUM2SIZET(threshold);

//   // A bit nasty this, relying on knowledge of internal implementation details
//   // of the SketchUp Ruby API and the SketchUp C API - passing void pointers
//   // around.
//   void* data = rb_data_object_get(image_rep);
//   SUImageRepRef image_rep_ref = *reinterpret_cast<SUImageRepRef*>(data);

//   // Create a Ruby ImageRep, but use the C API to populate the data for
//   // performance reasons. This is taking advantage of implementation details of
//   // both APIs. The Ruby API and the C API both wraps the same data type.
//   VALUE ruby_out_image = new_ruby_image_rep();
//   void* out_data = rb_data_object_get(ruby_out_image);
//   SUImageRepRef out_image = *reinterpret_cast<SUImageRepRef*>(out_data);

//   GenerateTraceMap(image_rep_ref, trace_threshold, out_image);

//   return ruby_out_image;
// }

// VALUE trace_polygons(VALUE self, VALUE image_rep, VALUE threshold, VALUE simplify) {
//   size_t trace_threshold = NUM2SIZET(threshold);
//   bool polygon_simplify = RTEST(simplify);

//   void* data = rb_data_object_get(image_rep);
//   SUImageRepRef image_rep_ref = *reinterpret_cast<SUImageRepRef*>(data);

//   VALUE ruby_out_image = new_ruby_image_rep();
//   void* out_data = rb_data_object_get(ruby_out_image);
//   SUImageRepRef out_image = *reinterpret_cast<SUImageRepRef*>(out_data);

//   const auto polygons = TraceImageRepPolygons(image_rep_ref, trace_threshold,
//       polygon_simplify);

//   VALUE ruby_polygons = rb_ary_new_capa((long)polygons.size());
//   for (const auto& polygon : polygons) {
//     VALUE ruby_polygon = rb_ary_new_capa((long)polygon.Points().size());
//     for (const auto& point : polygon.Points()) {
//       VALUE ruby_point = RubyInterface(point);
//       rb_ary_push(ruby_polygon, ruby_point);
//     }
//     rb_ary_push(ruby_polygons, ruby_polygon);
//   }

//   return ruby_polygons;
// }

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
  rb_define_module_function(mExample, "convert_to_greyscale",
      VALUEFUNC(convert_to_greyscale), 2);
}

} // extern "C"
