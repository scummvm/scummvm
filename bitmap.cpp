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

#include "stdafx.h"
#include <cstdlib>
#include <cstring>
#include <SDL.h>
#include <SDL_opengl.h>
#include "bitmap.h"
#include "bits.h"
#include "debug.h"

#define BITMAP_TEXTURE_SIZE 256

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

  if (format_ == 1) {
    num_tex_ = ((width_ + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE) *
      ((height_ + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE);
    tex_ids_ = new GLuint[num_tex_];
    glGenTextures(num_tex_, tex_ids_);
    for (int i = 0; i < num_tex_; i++) {
      glBindTexture(GL_TEXTURE_2D, tex_ids_[i]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
		   BITMAP_TEXTURE_SIZE, BITMAP_TEXTURE_SIZE, 0,
		   GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
    }
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, width_);

    int cur_tex_idx = 0;
    for (int y = 0; y < height_; y += BITMAP_TEXTURE_SIZE) {
      for (int x = 0; x < width_; x += BITMAP_TEXTURE_SIZE) {
	int width  = (x + BITMAP_TEXTURE_SIZE >= width_)  ? (width_  - x) : BITMAP_TEXTURE_SIZE;
	int height = (y + BITMAP_TEXTURE_SIZE >= height_) ? (height_ - y) : BITMAP_TEXTURE_SIZE;
	glBindTexture(GL_TEXTURE_2D, tex_ids_[cur_tex_idx]);
	glTexSubImage2D(GL_TEXTURE_2D,
			0,
			0, 0,
			width, height,
			GL_RGB,
			GL_UNSIGNED_SHORT_5_6_5,
			data_[curr_image_] + (y * 2 * width_) + (2 * x));
	cur_tex_idx++;
      }
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  } else {
    for (int i = 0; i < (width_ * height_); i++) {
      uint16_t val = get_LE_uint16(data_[curr_image_] + 2 * i);
      ((uint16_t *) data_[curr_image_])[i] =
	0xffff - ((uint32_t) val) * 0x10000 / 100 / (0x10000 - val);
    }
    tex_ids_ = NULL;
  }
}

void Bitmap::prepareGL() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, 640, 480, 0, 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  // A lot more may need to be put there : disabling Alpha test, blending, ...
  // For now, just keep this here :-)
  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
}

void Bitmap::draw() const {
  if (format_ == 1) {		// Normal image
    if (curr_image_ != 0) {
      warning("Animation not handled yet in GL texture path !\n");
    }
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_SCISSOR_TEST);
    int cur_tex_idx = 0;
    for (int y = y_; y < (y_ + height_); y += BITMAP_TEXTURE_SIZE) {
      for (int x = x_; x < (x_ + width_); x += BITMAP_TEXTURE_SIZE) {
	int width  = (x + BITMAP_TEXTURE_SIZE >= (x_ + width_))  ? ((x_ + width_)  - x) : BITMAP_TEXTURE_SIZE;
	int height = (y + BITMAP_TEXTURE_SIZE >= (y_ + height_)) ? ((y_ + height_) - y) : BITMAP_TEXTURE_SIZE;
	glBindTexture(GL_TEXTURE_2D, tex_ids_[cur_tex_idx]);
	glScissor(x, 480 - (y + height), x + width, 480 - y);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex2i(x, y);
	glTexCoord2f(1.0, 0.0);
	glVertex2i(x + BITMAP_TEXTURE_SIZE, y);
	glTexCoord2f(1.0, 1.0);
	glVertex2i(x + BITMAP_TEXTURE_SIZE, y + BITMAP_TEXTURE_SIZE);
	glTexCoord2f(0.0, 1.0);
	glVertex2i(x, y + BITMAP_TEXTURE_SIZE);
	glEnd();
	cur_tex_idx++;
      }
    }
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_TEXTURE_2D);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
  } else if (format_ == 5) {	// ZBuffer image
    // Only draw the manual zbuffer when we are not using screenblocks, and when enabled
    if ((ZBUFFER_GLOBAL == 0) || (SCREENBLOCKS_GLOBAL == 1))
	return;

    if (curr_image_ != 0) {
      warning("Animation not handled yet in GL texture path !\n");
    }
    glRasterPos2i(x_, y_);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 
    glDepthMask(GL_TRUE);
    /* This loop here is to prevent using PixelZoom that may be unoptimized for the 1.0 / -1.0 case
       in some drivers...
    */
    for (int row = 0; row < height_; row++) {
      glRasterPos2i(x_, y_ + row + 1);
    //  glDrawPixels(width_, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, data_[curr_image_] + (2 * row * width_));
    }
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
    glDepthFunc(GL_LESS);
  }
}

Bitmap::~Bitmap() {
  for (int i = 0; i < num_images_; i++)
    delete[] data_[i];
  delete[] data_;
  if (tex_ids_) {
    glDeleteTextures(num_tex_, tex_ids_);
    delete[] tex_ids_;
  }
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
