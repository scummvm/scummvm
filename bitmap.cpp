// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include <cstdlib>
#include <cstring>
#include <SDL.h>
#include <SDL_opengl.h>
#include "bitmap.h"
#include "bits.h"
#include "debug.h"

static void decompress_codec3(const char *compressed, char *result);

Bitmap::Bitmap(const char *filename, const char *data, int len) :
  Resource(filename)
{
  if (len < 8 || memcmp(data, "BM  F\0\0\0", 8) != 0)
    error("Invalid magic loading bitmap\n");

  int codec = get_LE_uint32(data + 8);
  num_images_ = get_LE_uint32(data + 16);
  x_ = get_LE_uint32(data + 20);
  y_ = get_LE_uint32(data + 24);
  format_ = get_LE_uint32(data + 32);
  width_ = get_LE_uint32(data + 128);
  height_ = get_LE_uint32(data + 132);
  curr_image_ = 0;

  data_ = new char*[num_images_];
  int pos = 0x88;
  for (int i = 0; i < num_images_; i++) {
    data_[i] = new char[2 * width_ * height_];
    if (codec == 0) {
      memcpy(data_[i], data + pos, 2 * width_ * height_);
      pos += 2 * width_ * height_ + 8;
    }
    else if (codec == 3) {
      int compressed_len = get_LE_uint32(data + pos);
      decompress_codec3(data + pos + 4, data_[i]);
      pos += compressed_len + 12;
    }
  }
}

void Bitmap::prepareGL() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, 640, 480, 0, 0, 1);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glPixelZoom(1, -1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void Bitmap::draw() const {
  glRasterPos2i(x_, y_);

  // FIXME: glDrawPixels is SLOW, we should be splitting the image into precached textures
  // and mapping them onto rectangles, or something.
  if (format_ == 1)
    glDrawPixels(width_, height_, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, data_[curr_image_]);
  else if (format_ == 5) {
    printf("format2\n");
    glDrawPixels(width_, height_, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, data_[curr_image_]);
  }
}

Bitmap::~Bitmap() {
  for (int i = 0; i < num_images_; i++)
    delete[] data_[i];
  delete[] data_;
}

#define GET_BIT bit = bitstr_value & 1; \
  bitstr_len--; \
  bitstr_value >>= 1; \
  if (bitstr_len == 0) { \
    bitstr_value = get_LE_uint16(compressed); \
    bitstr_len = 16; \
    compressed += 2; \
  } \
  do {} while (0)

static void decompress_codec3(const char *compressed, char *result) {
  int bitstr_value = get_LE_uint16(compressed);
  int bitstr_len = 16;
  compressed += 2;
  bool bit;

  for (;;) {
    GET_BIT;
    if (bit == 1)
      *result++ = *compressed++;
    else {
      GET_BIT;
      int copy_len, copy_offset;
      if (bit == 0) {
	GET_BIT;
	copy_len = 2 * bit;
	GET_BIT;
	copy_len += bit + 3;
	copy_offset = get_uint8(compressed++) - 0x100;
      }
      else {
	copy_offset = (get_uint8(compressed) |
		       (get_uint8(compressed + 1) & 0xf0) << 4) - 0x1000;
	copy_len = (get_uint8(compressed + 1) & 0xf) + 3;
	compressed += 2;
	if (copy_len == 3) {
	  copy_len = get_uint8(compressed++) + 1;
	  if (copy_len == 1)
	    return;
	}
      }
      while (copy_len > 0) {
	*result = result[copy_offset];
	result++;
	copy_len--;
      }
    }
  }
}
