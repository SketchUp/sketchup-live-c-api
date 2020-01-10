```cpp
#include <SketchUpAPI/sketchup.h>


namespace example {
namespace ruby {

VALUE ruby_num_faces(VALUE self) {
  SUModelRef model = SU_INVALID;
  SUApplicationGetActiveModel(&model);

  if (SUIsInvalid(model)) {
    return Qnil;
  }

  SUEntitiesRef entities = SU_INVALID;
  SUModelGetEntities(model, &entities);

  size_t num_faces = 0;
  SUEntitiesGetNumFaces(entities, &num_faces);

  return SIZET2NUM(num_faces);
}

} // namespace ruby
} // namespace example


extern "C" {

void Init_traceup()
{
  using namespace example::ruby;

  VALUE mExample = rb_define_module("Example");
  rb_define_module_function(mExample, "num_faces", VALUEFUNC(ruby_num_faces), 0);
}

} // extern "C"

```
