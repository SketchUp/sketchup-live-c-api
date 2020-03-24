#include "gtest/gtest.h"

#include <SketchUpAPI/model/image_rep.h>

#include "image.h"

namespace example {

TEST(ImageTest, InvalidInputBoth) {
  const SUImageRepRef input_image = SU_INVALID;
  SUImageRepRef output_image = SU_INVALID;
  auto result = GreyScaleCopy(input_image, output_image, 0.5);
  ASSERT_FALSE(result);
}

} // namespace example
