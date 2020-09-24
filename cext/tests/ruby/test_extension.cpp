#include <assert.h>

#include <ruby.h>


namespace {


struct DataItem {
  int value;
};


#define VALUEFUNC(f) ((VALUE (*)(ANYARGS)) f)
#define VOIDFUNC(f)  ((RUBY_DATA_FUNC) f)


// Disable the warnings for "unreferenced formal parameter" due to Ruby's
// required `self` argument which is often unused.
#pragma warning (push)
#pragma warning (disable : 4100)

void wrap_free(void* data)
{
  delete reinterpret_cast<DataItem*>(data);
}

size_t wrap_type_size(const void* data)
{
  return sizeof(DataItem);
}

const rb_data_type_t data_type = {
  "ex_data_t", // wrap_struct_name
  { // function
    nullptr,        // dmark
    wrap_free,      // dfree
    wrap_type_size, // dsize
    // New member added between Ruby 2.5 and 2.7. Unclear how to use it.
    // To make compiling across versions easier, relying on default
    // initialization. Assuming nullptr for dcompact mean not supporting.
    // nullptr // dcompact (How to use?)
    // { nullptr, nullptr } // reserved (must be ZERO)
  },
  nullptr, // parent
  nullptr, // data
  RUBY_TYPED_FREE_IMMEDIATELY, // flags
};

VALUE wrap_alloc(VALUE self)
{
  auto data = new DataItem;
  return TypedData_Wrap_Struct(self, &data_type, data);
}

DataItem* get_instance(VALUE self)
{
  DataItem* data;
  TypedData_Get_Struct(self, DataItem, &data_type, data);
  assert(data);
  return data;
}

VALUE wrap_initialize(VALUE self, VALUE ruby_value)
{
  const int value = NUM2INT(ruby_value);

  auto data = get_instance(self);
  *data = DataItem{ value };
  return self;
}

VALUE wrap_value(VALUE self)
{
  return INT2NUM(get_instance(self)->value);
}

} // namespace


extern "C" {

__declspec(dllexport) void Init_compacter()
{
  VALUE mExample = rb_define_module("Example");
  VALUE mCompacting = rb_define_module_under(mExample, "Compacting");
  VALUE cData = rb_define_class_under(mCompacting, "Data", rb_cObject);

  rb_define_alloc_func(cData, wrap_alloc);
  rb_define_method(cData, "initialize", VALUEFUNC(wrap_initialize), 1);

  rb_define_method(cData, "value", VALUEFUNC(wrap_value), 0);
}

} // extern "C"
