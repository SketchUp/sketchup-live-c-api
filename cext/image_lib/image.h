#pragma once

#include <SketchUpAPI/model/image_rep.h>

namespace example {

/**
 * @brief Blends the \p input image to greyscale by the \p amount given.
 *
 * @param input[in] A valid image rep with image data.
 * @param amount[in] A value between `0.0` and `1.0` where `1.0` is full color
 *                   and `0.0` is full greyscale.
 * @param output[out] A valid image rep. Existing image data will be cleared.
 * @return true
 * @return false
 */
bool GreyScaleCopy(const SUImageRepRef input, double amount, SUImageRepRef output);

} // namespace example
