#include <ruby.h>

extern "C" {

void Init_compacter()
{
  VALUE mExample = rb_define_module("Example");
  VALUE mCompacting = rb_define_module_under(mExample, "Compacting");
  VALUE cData = rb_define_class_under(mCompacting, "Data", rb_cObject);

  // rb_define_module_function(mCompacting, "trace_map", VALUEFUNC(trace_map), 2);
}

} // extern "C"
