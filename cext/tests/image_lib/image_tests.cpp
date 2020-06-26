#include "gtest/gtest.h"

#include <array>
#include <vector>
#include <SketchUpAPI/model/image_rep.h>
#include "image.h"

namespace example {

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

class ImageTest : public ::testing::Test {
protected:
  ImageTest() : image_data_(2, 2, 3),
      input_image_(SU_INVALID), output_image_(SU_INVALID) {

    SUImageRepCreate(&output_image_);
    assert(SUIsValid(output_image_));

    SUImageRepCreate(&input_image_);
    assert(SUIsValid(input_image_));
    std::vector<SUByte> pixels{
      255,  65,   0,
      32,    0,   0,
        0,  90,  64,
      255, 255, 255,
    };
    assert(pixels.size() == image_data_.size);

    [[maybe_unused]] auto result = SUImageRepSetData(input_image_,
        image_data_.width, image_data_.height,
        image_data_.bits_per_pixel, 0, pixels.data());
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

  return buffer;
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

  std::vector<SUByte> expected{
      255,  65,   0, 255,
       32,   0,   0, 255,
        0,  90,  64, 255,
      255, 255, 255, 255,
  };
  auto buffer = GetImageData(output_image_);
  ASSERT_EQ(expected, buffer);
}

TEST_F(ImageTest, GreyScaleCopy_GreyScale_50) {
  auto result = GreyScaleCopy(input_image_, 0.5, output_image_);
  ASSERT_TRUE(result);
  AssertExpectedImageInfo(image_data_, output_image_);

  std::vector<SUByte> expected{
      161,  66,  33, 255,
       17,   1,   1, 255,
       35,  80,  67, 255,
      255, 255, 255, 255,
  };
  auto buffer = GetImageData(output_image_);
  ASSERT_EQ(expected, buffer);
}

TEST_F(ImageTest, GreyScaleCopy_GreyScale_100) {
  auto result = GreyScaleCopy(input_image_, 0.0, output_image_);
  ASSERT_TRUE(result);
  AssertExpectedImageInfo(image_data_, output_image_);

  std::vector<SUByte> expected{
       67,  67,  67, 255,
        3,   3,   3, 255,
       71,  71,  71, 255,
      255, 255, 255, 255,
  };
  auto buffer = GetImageData(output_image_);
  ASSERT_EQ(expected, buffer);
}

} // namespace example
