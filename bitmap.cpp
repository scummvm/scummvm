// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2004 The ScummVM-Residual Team (www.scummvm.org)
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
#include "bitmap.h"
#include "bits.h"
#include "smush.h"
#include "debug.h"

#include "driver_gl.h"

#define BITMAP_TEXTURE_SIZE 256

static void decompress_codec3(const char *compressed, char *result);

Bitmap::Bitmap(const char *filename, const char *data, int len) :
Resource(filename) {
	if (len < 8 || memcmp(data, "BM  F\0\0\0", 8) != 0)
		error("Invalid magic loading bitmap\n");

	int codec = READ_LE_UINT32(data + 8);
	num_images_ = READ_LE_UINT32(data + 16);
	x_ = READ_LE_UINT32(data + 20);
	y_ = READ_LE_UINT32(data + 24);
	format_ = READ_LE_UINT32(data + 32);
	width_ = READ_LE_UINT32(data + 128);
	height_ = READ_LE_UINT32(data + 132);
	curr_image_ = 1;

	data_ = new char*[num_images_];
	int pos = 0x88;
	for (int i = 0; i < num_images_; i++) {
		data_[i] = new char[2 * width_ * height_];
		if (codec == 0) {
			memcpy(data_[i], data + pos, 2 * width_ * height_);
			pos += 2 * width_ * height_ + 8;
		} else if (codec == 3) {
				int compressed_len = READ_LE_UINT32(data + pos);
				decompress_codec3(data + pos + 4, data_[i]);
				pos += compressed_len + 12;
		}

	#ifdef SYSTEM_BIG_ENDIAN
		for (int j = 0; j < width_ * height_; ++j) {
			((uint16 *)data_[i])[j] = SWAP_BYTES_16(((uint16 *)data_[i])[j]);
		}
	#endif
	}	
	
	if (format_ == 1) {
		hasTransparency_ = false;
		num_tex_ = ((width_ + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE) *
			((height_ + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE);
		tex_ids_ = new GLuint[num_tex_ * num_images_];
		glGenTextures(num_tex_ * num_images_, tex_ids_);

		char *texData = new char[4 * width_ * height_];

		for (int pic = 0; pic < num_images_; pic++) {
			// Convert data to 32-bit RGBA format
			char *texDataPtr = texData;
			uint16 *bitmapData = reinterpret_cast<uint16 *>(data_[pic]);
			for (int i = 0; i < width_ * height_;
			     i++, texDataPtr += 4, bitmapData++) {
				uint16 pixel = *bitmapData;
				int r = pixel >> 11;
				texDataPtr[0] = (r << 3) | (r >> 2);
				int g = (pixel >> 5) & 0x3f;
				texDataPtr[1] = (g << 2) | (g >> 4);
				int b = pixel & 0x1f;
				texDataPtr[2] = (b << 3) | (b >> 2);
				if (pixel == 0xf81f) { // transparent
					texDataPtr[3] = 0;
					hasTransparency_ = true;
				}
				else
					texDataPtr[3] = 255;
			}

			for (int i = 0; i < num_tex_; i++) {
				glBindTexture(GL_TEXTURE_2D, tex_ids_[num_tex_ * pic + i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
					     BITMAP_TEXTURE_SIZE, BITMAP_TEXTURE_SIZE, 0,
					     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			}

			glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, width_);

			int cur_tex_idx = num_tex_ * pic;

			for (int y = 0; y < height_; y += BITMAP_TEXTURE_SIZE) {
				for (int x = 0; x < width_; x += BITMAP_TEXTURE_SIZE) {
					int width  = (x + BITMAP_TEXTURE_SIZE >= width_)  ? (width_  - x) : BITMAP_TEXTURE_SIZE;
					int height = (y + BITMAP_TEXTURE_SIZE >= height_) ? (height_ - y) : BITMAP_TEXTURE_SIZE;
					glBindTexture(GL_TEXTURE_2D, tex_ids_[cur_tex_idx]);
					glTexSubImage2D(GL_TEXTURE_2D,
							0,
							0, 0,
							width, height,
							GL_RGBA,
							GL_UNSIGNED_BYTE,
							texData + (y * 4 * width_) + (4 * x));
					cur_tex_idx++;
				}
			}
		}

		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		delete [] texData;
	} else {
		for (int pic = 0; pic < num_images_; pic++) {
			uint16 *zbufPtr = reinterpret_cast<uint16*>(data_[pic]);
			for (int i = 0; i < (width_ * height_); i++) {
				uint16 val = READ_LE_UINT16(data_[pic] + 2 * i);
				zbufPtr[i] = 0xffff - ((uint32) val) * 0x10000 / 100 / (0x10000 - val);
			}

			// Flip the zbuffer image to match what GL expects
			for (int y = 0; y < height_ / 2; y++) {
				uint16 *ptr1 = zbufPtr + y * width_;
				uint16 *ptr2 = zbufPtr + (height_ - 1 - y) * width_;
				for (int x = 0; x < width_; x++, ptr1++, ptr2++) {
					uint16 tmp = *ptr1;
					*ptr1 = *ptr2;
					*ptr2 = tmp;
				}
			}
		}

		tex_ids_ = NULL;
	}
}

void Bitmap::draw() const {
	if (curr_image_ == 0)
		return;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 640, 480, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	// A lot more may need to be put there : disabling Alpha test, blending, ...
	// For now, just keep this here :-)
	if (hasTransparency_) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
		glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	if (format_ == 1) {		// Normal image
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glEnable(GL_SCISSOR_TEST);
		glScissor(x_, 480 - (y_ + height_), width_, height_);
		int cur_tex_idx = num_tex_ * (curr_image_ - 1);
		for (int y = y_; y < (y_ + height_); y += BITMAP_TEXTURE_SIZE) {
			for (int x = x_; x < (x_ + width_); x += BITMAP_TEXTURE_SIZE) {
				glBindTexture(GL_TEXTURE_2D, tex_ids_[cur_tex_idx]);
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
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
	} else if (format_ == 5) {	// ZBuffer image
		// Only draw the manual zbuffer when we are not using screenblocks, and when enabled
		if ((ZBUFFER_GLOBAL == 0) || (SCREENBLOCKS_GLOBAL == 1))
			return;

		g_driver->drawDepthBitmap(x_, y_, width_, height_, data_[curr_image_ - 1]);
	}
}

Bitmap::~Bitmap() {
	for (int i = 0; i < num_images_; i++)
		delete[] data_[i];
	delete[] data_;
	if (tex_ids_) {
		glDeleteTextures(num_tex_ * num_images_, tex_ids_);
		delete[] tex_ids_;
	}
}

#define GET_BIT do { bit = bitstr_value & 1; \
	bitstr_len--; \
	bitstr_value >>= 1; \
	if (bitstr_len == 0) { \
		bitstr_value = READ_LE_UINT16(compressed); \
		bitstr_len = 16; \
		compressed += 2; \
	} \
} while (0)

static void decompress_codec3(const char *compressed, char *result) {
	int bitstr_value = READ_LE_UINT16(compressed);
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
				copy_offset = *(uint8 *)(compressed++) - 0x100;
			} else {
				copy_offset = (*(uint8 *)(compressed) |
					(*(uint8 *)(compressed + 1) & 0xf0) << 4) - 0x1000;
				copy_len = (*(uint8 *)(compressed + 1) & 0xf) + 3;
				compressed += 2;
				if (copy_len == 3) {
					copy_len = *(uint8 *)(compressed++) + 1;
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
