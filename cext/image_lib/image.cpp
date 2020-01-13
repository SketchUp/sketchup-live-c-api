#include "image.hpp"

namespace example {

SUImageRepRef CreateGreyScaleCopy(SUImageRepRef source_image)
{
  SUImageRepRef output_image = SU_INVALID;

  if (SUIsInvalid(source_image)) {
    return output_image;
  }

  return output_image;
}

} // namespace example
