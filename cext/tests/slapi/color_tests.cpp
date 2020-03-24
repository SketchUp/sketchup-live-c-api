#include "gtest/gtest.h"

#include "slapi/color.h"

namespace example {

TEST(ColorTest, ConstructorDefault) {
  Color color;
  EXPECT_EQ(0, color.red);
  EXPECT_EQ(0, color.green);
  EXPECT_EQ(0, color.blue);
  EXPECT_EQ(0, color.alpha);
}

TEST(ColorTest, ConstructorSUColor) {
  SUColor su_color{ 12, 34, 56, 78 };
  Color color(su_color);
  EXPECT_EQ(12, color.red);
  EXPECT_EQ(34, color.green);
  EXPECT_EQ(56, color.blue);
  EXPECT_EQ(78, color.alpha);
}

TEST(ColorTest, ConstructorSUByte) {
  Color color(12, 34, 56, 78);
  EXPECT_EQ(12, color.red);
  EXPECT_EQ(34, color.green);
  EXPECT_EQ(56, color.blue);
  EXPECT_EQ(78, color.alpha);
}


TEST(ColorTest, OperatorEqual) {
  Color color1(12, 34, 56, 78);
  Color color2(12, 34, 56, 78);
  EXPECT_EQ(color1, color2);
}

TEST(ColorTest, OperatorNotEqual) {
  Color color1(12, 34, 56, 78);
  Color color2(12, 34, 56, 00);
  EXPECT_NE(color1, color2);
}


TEST(ColorTest, DataOrder) {
  #ifdef __APPLE__
    SUByte expected[4]{ 12, 34, 56, 78 };
  #else
    SUByte expected[4]{ 56, 34, 12, 78 };
  #endif
  Color color(12, 34, 56, 78);
  SUByte* color_data = reinterpret_cast<SUByte*>(&color);
  EXPECT_EQ(expected[0], color_data[0]);
  EXPECT_EQ(expected[1], color_data[1]);
  EXPECT_EQ(expected[2], color_data[2]);
  EXPECT_EQ(expected[3], color_data[3]);
}


TEST(ColorTest, LuminanceBlack) {
  EXPECT_EQ(0, Luminance({ 0, 0, 0, 255 }));
}

TEST(ColorTest, LuminanceWhite) {
  EXPECT_EQ(255, Luminance({ 255, 255, 255, 255 }));
}

TEST(ColorTest, LuminanceRed) {
  EXPECT_EQ(76, Luminance({ 255, 0, 0, 255 }));
}

TEST(ColorTest, LuminanceGreen) {
  EXPECT_EQ(149, Luminance({ 0, 255, 0, 255 }));
}

TEST(ColorTest, LuminanceBlue) {
  EXPECT_EQ(29, Luminance({ 0, 0, 255, 255 }));
}

TEST(ColorTest, LuminanceSolid) {
  EXPECT_EQ(29, Luminance({ 12, 34, 56, 255 }));
}

TEST(ColorTest, LuminanceTransparent) {
  EXPECT_EQ(29, Luminance({ 12, 34, 56, 78 }));
}

} // namespace example
