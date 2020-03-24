#include "slapi/color.h"

#include <cassert>

namespace example {

SUByte Luminance(const SUColor& color) {
  // Original:
  // http://forums.sketchucation.com/viewtopic.php?t=12368#p88865
  // (red * 0.3) + (green * 0.59) + (blue * 0.11)
  // Current: https://stackoverflow.com/a/596243/486990
  //  => https://www.w3.org/TR/AERT/#color-contrast
  auto value = ((color.red * 299) + (color.green * 587) + (color.blue * 114)) / 1000;
  assert(value >= 0 && value <= 255);
  return static_cast<SUByte>(value);
}

double Blend(double value1, double w1, double value2, double w2)
{
  return (value1 * w1) + (value2 * w2);
}

SUByte Blend(SUByte value1, double w1, double value2, double w2)
{
  double value = Blend(
    static_cast<double>(value1), w1,
    static_cast<double>(value2), w2
  );
  assert(value >= 0 && value <= 255); // Or explicitly clamp?
  return static_cast<SUByte>(value);
}

Color Blend(const Color& color1, const Color& color2, double amount)
{
  const double w1 = amount;
  const double w2 = 1.0 - amount;
  SUByte red = Blend(color1.red, w1, color2.red, w2);
  SUByte green = Blend(color1.green, w1, color2.green, w2);
  SUByte blue = Blend(color1.blue, w1, color2.blue, w2);
  SUByte alpha = Blend(color1.alpha, w1, color2.alpha, w2);
  return { red, green, blue, alpha };
}

} // namespace example
