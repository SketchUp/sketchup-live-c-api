#include "image.hpp"

#include <algorithm>
#include <vector>

#include "slapi/color.hpp"
#include "slapi/utilities.hpp"

namespace example {

bool CreateGreyScaleCopy(const SUImageRepRef input, SUImageRepRef output)
{
  if (SUIsInvalid(input) || SUIsInvalid(output)) {
    return false;
  }

  size_t data_size = 0, bits_per_pixel = 0;
  SU(SUImageRepGetDataSize(input, &data_size, &bits_per_pixel));

  size_t width = 0, height = 0;
  SU(SUImageRepGetPixelDimensions(input, &width, &height));

  size_t num_pixels = width * height;
  std::vector<SUColor> colors(num_pixels);
  SU(SUImageRepGetDataAsColors(input, colors.data()));

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
  SU(SUImageRepSetData(output, width, height, 32, 0, buffer));

  return true;
}

} // namespace example
