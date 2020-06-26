#pragma once

#include <SketchUpAPI/color.h>

namespace example {

/**
 * @brief Utility Color structure to be used with SUImageRep.
 *
 * It's members are arranged according to the RGBA/BGRA layout the various
 * platforms expects.
 *
 * It's main purpose is to be used in an array/vector which is reinterpreted
 * into an SUByte sequence which `SUImageRepSetData` can use.
 */
#ifdef __APPLE__
struct Color {
  Color() : red(0), green(0), blue(0), alpha(0) {}
  Color(SUByte r, SUByte g, SUByte b, SUByte a) :
      red(r), green(g), blue(b), alpha(a) {}
  Color(const SUColor& color) :
      red(color.red), green(color.green), blue(color.blue), alpha(color.alpha) {}
  SUByte red;
  SUByte green;
  SUByte blue;
  SUByte alpha;
};
#else
struct Color {
  Color() : blue(0), green(0), red(0), alpha(0) {}
  Color(SUByte r, SUByte g, SUByte b, SUByte a) :
      blue(b), green(g), red(r), alpha(a) {}
  Color(const SUColor& color) :
      blue(color.blue), green(color.green), red(color.red), alpha(color.alpha) {}

  SUByte blue;
  SUByte green;
  SUByte red;
  SUByte alpha;
};
#endif

/**
 * @brief Compares two Color structs for equality.
 *
 * @relates Color
 *
 * @param lhs
 * @param rhs
 * @return `true` when the two colors are equal, `false` otherwise.
 */
inline bool operator==(const Color& lhs, const Color& rhs) {
  // https://en.cppreference.com/w/cpp/language/operators
  return lhs.red == rhs.red &&
      lhs.green == rhs.green &&
      lhs.blue == rhs.blue &&
      lhs.alpha == rhs.alpha;
}
/**
 * @brief Compares two Color structs for inequality.
 *
 * @relates Color
 *
 * @param lhs
 * @param rhs
 * @return `true` when the two colors are not equal, `false` otherwise.
 */
inline bool operator!=(const Color& lhs, const Color& rhs) {
  return !(lhs == rhs);
}

/**
 * @brief Colorimetric conversion to greyscale.
 *
 * @param color
 * @return SUByte
 */
SUByte Luminance(const SUColor& color);

/**
 * @brief Blend two colors with given an `amount` between 0.0 and 1.0 where
 *        0.0 means 100% `color1` and 1.0 means 100% `color2`.
 *
 * @param color1
 * @param color2
 * @param amount A value between `0.0` and `1.0`.
 * @return Color
 */
Color Blend(const Color& color1, const Color& color2, double amount);

} // namespace example
