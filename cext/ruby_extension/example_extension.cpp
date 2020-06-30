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
#include <unordered_set>

#include <SketchUpAPI/sketchup.h>
#include <SketchUpAPI/application/ruby_api.h>

#include "ruby/framework.h"
#include "utilities.h"

#include "slapi/color.h"
#include "slapi/utilities.h"
#include "image.h"


// Allow SUVertexRef to be hashable used with std::unordered_set:

bool operator==(const SUVertexRef& lhs, const SUVertexRef& rhs) {
  return lhs.ptr == rhs.ptr;
}

namespace std
{
  template<> struct hash<SUVertexRef>
  {
    std::size_t operator()(SUVertexRef const& vertex) const noexcept
    {
      return std::hash<void*>{}(vertex.ptr);
    }
  };
}


namespace example {
namespace ruby {
namespace {

void GetEdgesWithMaterial(SUEntityListRef list, VALUE edges) {
  SUEntityListIteratorRef it = SU_INVALID;
  SU(SUEntityListIteratorCreate(&it));

  SUEntityRef entity = SU_INVALID;
  SU(SUEntityListBegin(list, &it));
  while(SUEntityListIteratorGetEntity(it, &entity) == SU_ERROR_NONE) {
    if (SUEntityGetType(entity) == SURefType_Edge) {
      auto element = SUDrawingElementFromEntity(entity);
      SUMaterialRef material = SU_INVALID;
      auto result = SUDrawingElementGetMaterial(element, &material);
      assert(result == SU_ERROR_NONE || result == SU_ERROR_NO_DATA);
      if (result == SU_ERROR_NONE) {
        VALUE ruby_edge = Qnil;
        SU(SUEntityToRuby(entity, &ruby_edge));
        rb_ary_push(edges, ruby_edge);
      }
    }
    SUEntityListIteratorNext(it);
  }

  SU(SUEntityListIteratorRelease(&it));
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

VALUE find_edges_with_material(VALUE self)
{
  SUModelRef model = SU_INVALID;
  SU(SUApplicationGetActiveModel(&model));
  if (SUIsInvalid(model)) {
    rb_raise(rb_eTypeError, "invalid model");
  }

  VALUE edges = rb_ary_new();
  SUEntityListRef entity_list = SU_INVALID;
  SU(SUEntityListCreate(&entity_list));

  SUSelectionRef selection = SU_INVALID;
  SU(SUModelGetSelection(model, &selection));

  size_t num_selected = 0;
  SU(SUSelectionGetNumElements(selection, &num_selected));
  if (num_selected > 0) {
    SU(SUSelectionGetEntityList(selection, &entity_list));
  } else {
    SUEntitiesRef entities = SU_INVALID;
    SU(SUModelGetActiveEntities(model, &entities));
    SU(SUEntitiesEntityListFill(entities, SURefType_Edge, entity_list));
  }
  GetEdgesWithMaterial(entity_list, edges);

  SU(SUEntityListRelease(&entity_list));

  return edges;
}

VALUE get_vertices(VALUE self, VALUE ruby_entities)
{
  SUModelRef model = SU_INVALID;
  SU(SUApplicationGetActiveModel(&model));
  if (SUIsInvalid(model)) {
    rb_raise(rb_eTypeError, "invalid model");
  }

  Check_Type(ruby_entities, T_ARRAY);

  // Using a set to ensure we have only unique vertices.
  std::unordered_set<SUVertexRef> vertices;
  long num_entities = RARRAY_LEN(ruby_entities);
  for(long i = 0; i < num_entities; ++i) {
    VALUE item = rb_ary_entry(ruby_entities, i);

    SUEntityRef entity = SU_INVALID;
    if (SUEntityFromRuby(item, &entity) != SU_ERROR_NONE) {
      continue;
    }

    auto type = SUEntityGetType(entity);
    if (type != SURefType_Edge) {
      continue;
    }

    SUEdgeRef edge = SUEdgeFromEntity(entity);
    assert(SUIsValid(edge));

    SUVertexRef vertex = SU_INVALID;
    SU(SUEdgeGetStartVertex(edge, &vertex));
    vertices.insert(vertex);

    vertex = SU_INVALID;
    SU(SUEdgeGetEndVertex(edge, &vertex));
    vertices.insert(vertex);
  }

  VALUE ruby_vertices = rb_ary_new_capa(static_cast<long>(vertices.size()));
  for(const auto& vertex : vertices) {
    SUEntityRef entity = SUVertexToEntity(vertex);
    VALUE ruby_vertex = Qnil;
    SU(SUEntityToRuby(entity, &ruby_vertex));
    rb_ary_push(ruby_vertices, ruby_vertex);
  }
  return ruby_vertices;
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

  rb_define_module_function(mLiveCAPI, "find_edges_with_material",
      VALUEFUNC(find_edges_with_material), 0);

  rb_define_module_function(mLiveCAPI, "get_vertices",
      VALUEFUNC(get_vertices), 1);
}

} // extern "C"
