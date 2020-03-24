#pragma once

#include <SketchUpAPI/color.h>

namespace example {

// Utility Color structure to be used with SUImageRep. It's members are arranged
// according to the RGBA/BGRA layout the various platforms expects.
//
// It's main purpose is to be used in an array/vector which is reinterpreted
// into an SUByte sequence which `SUImageRepSetData` can use.
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

// https://en.cppreference.com/w/cpp/language/operators
inline bool operator==(const Color& lhs, const Color& rhs) {
  return lhs.red == rhs.red &&
      lhs.green == rhs.green &&
      lhs.blue == rhs.blue &&
      lhs.alpha == rhs.alpha;
}
inline bool operator!=(const Color& lhs, const Color& rhs) {
  return !(lhs == rhs);
}

// Colorimetric conversion to greyscale.
SUByte Luminance(const SUColor& color);

Color Blend(const Color& color1, const Color& color2, double amount);

} // namespace example
