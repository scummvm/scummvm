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
	_numImages = READ_LE_UINT32(data + 16);
	_x = READ_LE_UINT32(data + 20);
	_y = READ_LE_UINT32(data + 24);
	_format = READ_LE_UINT32(data + 32);
	_width = READ_LE_UINT32(data + 128);
	_height = READ_LE_UINT32(data + 132);
	_currImage = 1;

	_data = new char *[_numImages];
	int pos = 0x88;
	for (int i = 0; i < _numImages; i++) {
		_data[i] = new char[2 * _width * _height];
		if (codec == 0) {
			memcpy(_data[i], data + pos, 2 * _width * _height);
			pos += 2 * _width * _height + 8;
		} else if (codec == 3) {
			int compressed_len = READ_LE_UINT32(data + pos);
			decompress_codec3(data + pos + 4, _data[i]);
			pos += compressed_len + 12;
		}

#ifdef SYSTEM_BIG_ENDIAN
		if (_format == 1)	
			for (int j = 0; j < _width * _height; ++j) {
				((uint16 *)_data[i])[j] = SWAP_BYTES_16(((uint16 *)_data[i])[j]);
			}
#endif
	}	
	
	g_driver->createBitmap(this);
}

void Bitmap::draw() const {
	if (_currImage == 0)
		return;

	g_driver->drawBitmap(this);
}

Bitmap::~Bitmap() {
	for (int i = 0; i < _numImages; i++)
		delete[] _data[i];

	delete[] _data;
	g_driver->destroyBitmap(this);
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
				copy_offset = (*(uint8 *)(compressed) | (*(uint8 *)(compressed + 1) & 0xf0) << 4) - 0x1000;
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
