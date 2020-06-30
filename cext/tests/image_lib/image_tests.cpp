#include "gtest/gtest.h"

#include <array>
#include <vector>
#include <SketchUpAPI/model/image_rep.h>
#include "image.h"


bool operator==(const SUColor& lhs, const SUColor& rhs) {
  return lhs.red == rhs.red &&
         lhs.green == rhs.green &&
         lhs.green == rhs.green &&
         lhs.alpha == rhs.alpha;
}

// Make GoogleTest print SUColor in a more readable manner.
void PrintTo(const SUColor& value, ::std::ostream* os) {
  *os << "SUColor("
      << static_cast<int>(value.red) << ", "
      << static_cast<int>(value.green) << ", "
      << static_cast<int>(value.blue) << ", "
      << static_cast<int>(value.alpha) << ")";
}

bool operator==(const SUColorOrder& lhs, const SUColorOrder& rhs) {
  return lhs.red_index == rhs.red_index &&
         lhs.green_index == rhs.green_index &&
         lhs.green_index == rhs.green_index &&
         lhs.alpha_index == rhs.alpha_index;
}


namespace example {

// Helper struct used to keep track of expected image data.
struct ImageInfo {
  ImageInfo(size_t width, size_t height, size_t num_channels) :
      width(width), height(height),
      num_channels(num_channels),
      num_pixels(width * height),
      bits_per_pixel(8 * num_channels),
      size(num_pixels * num_channels) {}
  size_t width = 0;
  size_t height = 0;
  size_t num_channels = 0;
  size_t num_pixels = 0;
  size_t bits_per_pixel = 0;
  size_t size = 0;
};

// Helper struct that let us work with ImageRepRef data regardless of how the
// RGBA data is stored per platform.
struct Pixel24bit {
  #ifdef _WIN32
  Pixel24bit(SUByte red, SUByte green, SUByte blue) :
      b(blue), g(green), r(red) {}
  SUByte b;
  SUByte g;
  SUByte r;
  #else
  Pixel24bit(SUByte red, SUByte green, SUByte blue) :
      r(red), g(green), b(blue) {}
  SUByte r;
  SUByte g;
  SUByte b;
  #endif
};

class ImageTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    // SUGetColorOrder isn't available at compile time so Pixel24bit et al must
    // be defined by making some assumptions.
    // Ensure the assumption of the RGBA order is correct. If implementation
    // details should change that should be caught here.
    #ifdef _WIN32
    SUColorOrder expected{ 2, 1, 0, 3 };
    #else
    [[maybe_unused]] SUColorOrder expected{ 0, 1, 2, 3 };
    #endif
    [[maybe_unused]] SUColorOrder actual = SUGetColorOrder();
    assert(actual == expected);
  }

  ImageTest() : image_data_(2, 2, 3),
      input_image_(SU_INVALID), output_image_(SU_INVALID) {

    SUImageRepCreate(&output_image_);
    assert(SUIsValid(output_image_));

    SUImageRepCreate(&input_image_);
    assert(SUIsValid(input_image_));
    std::vector<Pixel24bit> pixels{
      { 255,  65,   0 },
      { 32,    0,   0 },
      {   0,  90,  64 },
      { 255, 255, 255 },
    };
    assert(pixels.size() == image_data_.num_pixels);
    assert(pixels.size() * sizeof(Pixel24bit) == image_data_.size);

    SUByte* data = reinterpret_cast<SUByte*>(pixels.data());
    [[maybe_unused]] auto result = SUImageRepSetData(input_image_,
        image_data_.width, image_data_.height,
        image_data_.bits_per_pixel, 0, data);
    assert(result == SU_ERROR_NONE);
  }

  ~ImageTest() {
    if (SUIsValid(input_image_)) {
      SUImageRepRelease(&input_image_);
    }
    if (SUIsValid(output_image_)) {
      SUImageRepRelease(&output_image_);
    }
  }

  ImageInfo image_data_;

  SUImageRepRef input_image_;
  SUImageRepRef output_image_;
};

void AssertExpectedImageInfo(const ImageInfo& image_data, SUImageRepRef image) {
  size_t width = 0, height = 0;
  ASSERT_EQ(SU_ERROR_NONE, SUImageRepGetPixelDimensions(image, &width, &height));
  ASSERT_EQ(image_data.width, width);
  ASSERT_EQ(image_data.height, height);

  size_t data_size = 0, bpp = 0;
  ASSERT_EQ(SU_ERROR_NONE, SUImageRepGetDataSize(image, &data_size, &bpp));
  // GreyScaleCopy currently creates a 32bit image regardless of the input
  // being 24bit.
  // Might want to consider outputting 24bit for 24bit input.
  // ASSERT_EQ(image_data.size, data_size);
  // ASSERT_EQ(image_data.bits_per_pixel, bpp);
  ASSERT_EQ(image_data.width * image_data.height * 4, data_size);
  ASSERT_EQ(32, bpp);
}

std::vector<SUByte> GetImageData(SUImageRepRef image) {
  size_t data_size = 0, bpp = 0;
  SUImageRepGetDataSize(image, &data_size, &bpp);

  std::vector<SUByte> buffer(data_size, 0);
  SUImageRepGetData(image, data_size, buffer.data());

  size_t width = 0, height = 0;
  SUImageRepGetPixelDimensions(image, &width, &height);

  std::vector<SUColor> colors(width * height, {0, 0, 0, 0});
  SUImageRepGetDataAsColors(image, colors.data());

  return buffer;
}

std::vector<SUColor> GetImagePixels(SUImageRepRef image) {
  size_t width = 0, height = 0;
  SUImageRepGetPixelDimensions(image, &width, &height);

  std::vector<SUColor> colors(width * height, {0, 0, 0, 0});
  SUImageRepGetDataAsColors(image, colors.data());

  return colors;
}


TEST_F(ImageTest, GreyScaleCopy_InvalidInputBoth) {
  const SUImageRepRef input_image = SU_INVALID;
  SUImageRepRef output_image = SU_INVALID;
  auto result = GreyScaleCopy(input_image, 0.5, output_image);
  ASSERT_FALSE(result);
}

TEST_F(ImageTest, GreyScaleCopy_GreyScale_0) {
  auto result = GreyScaleCopy(input_image_, 1.0, output_image_);
  ASSERT_TRUE(result);
  AssertExpectedImageInfo(image_data_, output_image_);

  std::vector<SUColor> expected{
      { 255,  65,   0, 255 },
      {  32,   0,   0, 255 },
      {   0,  90,  64, 255 },
      { 255, 255, 255, 255 },
  };
  auto buffer = GetImagePixels(output_image_);
  ASSERT_EQ(expected, buffer);
}

TEST_F(ImageTest, GreyScaleCopy_GreyScale_50) {
  auto result = GreyScaleCopy(input_image_, 0.5, output_image_);
  ASSERT_TRUE(result);
  AssertExpectedImageInfo(image_data_, output_image_);

  std::vector<SUColor> expected{
      { 184,  89,  57, 255 },
      {  20,   4,   4, 255 },
      {  30,  75,  62, 255 },
      { 255, 255, 255, 255 },
  };
  auto buffer = GetImagePixels(output_image_);
  ASSERT_EQ(expected, buffer);
}

TEST_F(ImageTest, GreyScaleCopy_GreyScale_100) {
  auto result = GreyScaleCopy(input_image_, 0.0, output_image_);
  ASSERT_TRUE(result);
  AssertExpectedImageInfo(image_data_, output_image_);

  std::vector<SUColor> expected{
      { 114, 114, 114, 255 },
      {   9,   9,   9, 255 },
      {  60,  60,  60, 255 },
      { 255, 255, 255, 255 },
  };
  auto buffer = GetImagePixels(output_image_);
  ASSERT_EQ(expected, buffer);
}

} // namespace example
