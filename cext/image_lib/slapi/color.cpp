#include "color.hpp"

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

} // namespace example
