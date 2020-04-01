/*
 *
 * Copyright (C) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "image/image.hpp"
#include "logging/logging.hpp"

#define png_infopp_NULL (png_infopp) NULL
#define int_p_NULL (int *)NULL

// Boost 1.68 introduced API-breaking changes to gil IO, handle this to be
// compatible with both pre-1.68 and newer versions of boost
#include <boost/version.hpp>
#define BOOST_VERSION_MAJOR (BOOST_VERSION / 100000)
#define BOOST_VERSION_MINOR (BOOST_VERSION / 100 % 1000)
#if (BOOST_VERSION_MAJOR == 1 && BOOST_VERSION_MINOR > 67)
#define BOOST_GIL_IO_PNG_168_API
#endif

#ifdef BOOST_GIL_IO_PNG_168_API
#include <boost/gil/extension/io/png.hpp>
#else
#include <boost/gil/extension/io/png_dynamic_io.hpp>
#endif

namespace gil = boost::gil;

#include "bmp.hpp"

namespace level_zero_tests {
template <typename T> ImagePNG<T>::ImagePNG() : width_(0), height_(0) {}

template <typename T> ImagePNG<T>::ImagePNG(const std::string &image_path) {
  read(image_path);
}

template <typename T>
ImagePNG<T>::ImagePNG(const int width, const int height)
    : width_(width), height_(height) {
  pixels_.resize(size());
}

template <typename T>
ImagePNG<T>::ImagePNG(const int width, const int height,
                      const std::vector<T> &data)
    : width_(width), height_(height), pixels_(data) {}

template <> bool ImagePNG<uint32_t>::read(const std::string &image_path) {
  gil::rgba8_image_t image;
#ifdef BOOST_GIL_IO_PNG_168_API
  gil::read_and_convert_image(image_path, image, gil::png_tag());
#else
  gil::png_read_and_convert_image(image_path, image);
#endif
  gil::rgba8_view_t view = gil::view(image);
  for (gil::rgba8_pixel_t pixel : view) {
    uint32_t raw_pixel =
        (pixel[0] << 24) + (pixel[1] << 16) + (pixel[2] << 8) + pixel[3];
    pixels_.push_back(raw_pixel);
  }
  width_ = static_cast<int>(view.width());
  height_ = static_cast<int>(view.height());
  return false;
}

template <> bool ImagePNG<uint32_t>::write(const std::string &image_path) {
  gil::rgba8_image_t image(width(), height());
  std::vector<gil::rgba8_pixel_t> channels;
  gil::rgba8_view_t view = gil::view(image);
  for (int id = 0; id < static_cast<int>(pixels_.size()); ++id) {
    uint32_t raw_pixel = pixels_[id];
    gil::rgba8_pixel_t pixel;
    pixel[3] = raw_pixel & 0xFF;
    pixel[2] = (raw_pixel >> 8) & 0xFF;
    pixel[1] = (raw_pixel >> 16) & 0xFF;
    pixel[0] = (raw_pixel >> 24) & 0xFF;
    view[id] = pixel;
  }
#ifdef BOOST_GIL_IO_PNG_168_API
  gil::write_view(image_path, view, gil::png_tag());
#else
  gil::png_write_view(image_path, view);
#endif
  return false;
}

template <typename T>
bool ImagePNG<T>::write(const std::string &image_path, const T *data) {
  copy_raw_data(data);
  return write(image_path);
}

template <typename T> int ImagePNG<T>::width() const { return width_; }

template <typename T> int ImagePNG<T>::height() const { return height_; }

template <typename T> int ImagePNG<T>::number_of_channels() const {
  return bits_per_pixel() / bits_per_channel();
}

template <typename T> int ImagePNG<T>::bits_per_channel() const { return 8; }

template <typename T> int ImagePNG<T>::bits_per_pixel() const {
  return std::numeric_limits<T>::digits;
}

template <typename T> int ImagePNG<T>::size() const {
  return width() * height();
}

template <typename T> int ImagePNG<T>::size_in_bytes() const {
  return static_cast<int>(pixels_.size() * sizeof(T));
}

template <typename T> T ImagePNG<T>::get_pixel(const int x, const int y) const {
  return pixels_[y * width() + x];
}

template <typename T>
void ImagePNG<T>::set_pixel(const int x, const int y, const T data) {
  pixels_[y * width() + x] = data;
}

template <typename T>
std::vector<T> level_zero_tests::ImagePNG<T>::get_pixels() const {
  return pixels_;
}

template <typename T> void ImagePNG<T>::copy_raw_data(const T *data) {
  std::copy(data, data + size(), std::begin(pixels_));
}

template <typename T> T *ImagePNG<T>::raw_data() { return pixels_.data(); }

template <typename T> const T *ImagePNG<T>::raw_data() const {
  return pixels_.data();
}

template <typename T>
bool ImagePNG<T>::operator==(const ImagePNG<T> &rhs) const {
  return pixels_ == rhs.pixels_;
}

template <typename T> void ImagePNG<T>::dump_image() const {
  for (int y = 0; y < height(); y++) {
    for (int x = 0; x < width(); x++) {
      T pixel = get_pixel(x, y);
      LOG_DEBUG << " x: " << x << " y: " << y << " pixel: 0x" << std::hex
                << pixel;
    }
  }
}

template class ImagePNG<uint32_t>;

template <typename T> ImageBMP<T>::ImageBMP() : width_(0), height_(0) {}

template <typename T> ImageBMP<T>::ImageBMP(const std::string &image_path) {
  read(image_path);
}

template <typename T>
ImageBMP<T>::ImageBMP(const int width, const int height)
    : width_(width), height_(height) {
  pixels_.resize(size());
}

template <typename T>
ImageBMP<T>::ImageBMP(const int width, const int height,
                      const std::vector<T> &data)
    : width_(width), height_(height), pixels_(data) {}

template <typename T> bool ImageBMP<T>::read(const std::string &image_path) {
  std::unique_ptr<uint8_t> data = nullptr;
  uint8_t *tmp = nullptr;
  int pitch = 0;
  uint16_t bits_per_pixel = 0;
  bool error = !BmpUtils::load_bmp_image(tmp, width_, height_, pitch,
                                         bits_per_pixel, image_path.c_str());
  data.reset(tmp);
  pixels_.resize(size(), 0);
  copy_raw_data(reinterpret_cast<T *>(data.get()));
  return error;
}

template <> bool ImageBMP<uint8_t>::read(const std::string &image_path) {
  std::unique_ptr<uint8_t> data = nullptr;
  uint8_t *tmp = nullptr;
  bool error =
      !BmpUtils::load_bmp_image_8u(tmp, width_, height_, image_path.c_str());
  data.reset(tmp);
  pixels_.resize(size(), 0);
  copy_raw_data(data.get());
  return error;
}

template <typename T> bool ImageBMP<T>::write(const std::string &image_path) {
  return !BmpUtils::save_image_as_bmp(pixels_.data(), width(), height(),
                                      image_path.c_str());
}

template <> bool ImageBMP<uint8_t>::write(const std::string &image_path) {
  return !BmpUtils::save_image_as_bmp_8u(pixels_.data(), width(), height(),
                                         image_path.c_str());
}

template <typename T>
bool ImageBMP<T>::write(const std::string &image_path, const T *data) {
  copy_raw_data(data);
  return write(image_path);
}

template <typename T> int ImageBMP<T>::width() const { return width_; }

template <typename T> int ImageBMP<T>::height() const { return height_; }

template <typename T> int ImageBMP<T>::number_of_channels() const {
  return bits_per_pixel() / bits_per_channel();
}

template <typename T> int ImageBMP<T>::bits_per_channel() const { return 8; }

template <typename T> int ImageBMP<T>::bits_per_pixel() const {
  return std::numeric_limits<T>::digits;
}

template <typename T> int ImageBMP<T>::size() const {
  return width() * height();
}

template <typename T> int ImageBMP<T>::size_in_bytes() const {
  return static_cast<int>(pixels_.size() * sizeof(T));
}

template <typename T> T ImageBMP<T>::get_pixel(const int x, const int y) const {
  return pixels_[y * width() + x];
}

template <typename T>
void ImageBMP<T>::set_pixel(const int x, const int y, const T data) {
  pixels_[y * width() + x] = data;
}

template <typename T> std::vector<T> ImageBMP<T>::get_pixels() const {
  return pixels_;
}

template <typename T> void ImageBMP<T>::copy_raw_data(const T *data) {
  std::copy(data, data + size(), std::begin(pixels_));
}

template <typename T> T *ImageBMP<T>::raw_data() { return pixels_.data(); }

template <typename T> const T *ImageBMP<T>::raw_data() const {
  return pixels_.data();
}

template <typename T> bool ImageBMP<T>::operator==(const ImageBMP &rhs) const {
  return pixels_ == rhs.pixels_;
}

template class ImageBMP<uint8_t>;
template class ImageBMP<uint32_t>;
} // namespace level_zero_tests
