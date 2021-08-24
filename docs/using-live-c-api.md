# Using the SketchUp Live C API

## Initialization

When using the Live C API there is no need to call `SUInitialize()` and `SUTerminate()` - SketchUp will take care of this.

## Entity Life Spans

It's important to beware of the life-span of SketchUp's entities when using the Live C API. In contrast to the SketchUp Ruby API, the C API's entity references are not invalidated if the internal entities are deleted. Make sure not to hold on to the entity references for long.

## Example

Below are two small example snippets that roughly illustrates the interaction between the C API and the Ruby API. Refer to the project in this repository for a more complete and detailed example.

```cpp
// Minimal Ruby C Extension

#include <ruby.h>
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

void Init_live_c_api_example()
{
  using namespace example::ruby;

  VALUE mExample = rb_define_module("Example");
  rb_define_module_function(mExample, "num_faces", VALUEFUNC(ruby_num_faces), 0);
}

} // extern "C"

```

```ruby
require 'example/live_c_api_example' # Load the Ruby C Extension

module Example

  def self.report_num_faces
    message = "Number of faces: #{self.num_faces}"
    UI.messagebox(message)
  end

end
```

## See Also

* [SketchUp Ruby C Extension examples](https://github.com/SketchUp/ruby-c-extension-examples) for more information on building Ruby C extensions for SketchUp.
* [SketchUp Extension examples](https://github.com/SketchUp/sketchup-ruby-api-tutorials) for details on how to structure a SketchUp Ruby extension.
