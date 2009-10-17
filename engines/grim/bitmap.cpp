/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "common/endian.h"

#include "engines/grim/grim.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/gfx_base.h"

namespace Grim {

static void decompress_codec3(const char *compressed, char *result);

Bitmap::Bitmap(const char *fname, const char *data, int len) {
	_fname = fname;

	if (len < 8 || memcmp(data, "BM  F\0\0\0", 8) != 0) {
		if (gDebugLevel == DEBUG_BITMAPS || gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL)
			error("Invalid magic loading bitmap");
	}

	strcpy(_filename, fname);

	int codec = READ_LE_UINT32(data + 8);
//	_paletteIncluded = READ_LE_UINT32(data + 12);
	_numImages = READ_LE_UINT32(data + 16);
	_x = READ_LE_UINT32(data + 20);
	_y = READ_LE_UINT32(data + 24);
//	_transparentColor = READ_LE_UINT32(data + 28);
	_format = READ_LE_UINT32(data + 32);
//	_numBits = READ_LE_UINT32(data + 36);
//	_blueBits = READ_LE_UINT32(data + 40);
//	_greenBits = READ_LE_UINT32(data + 44);
//	_redBits = READ_LE_UINT32(data + 48);
//	_blueShift = READ_LE_UINT32(data + 52);
//	_greenShift = READ_LE_UINT32(data + 56);
//	_redShift = READ_LE_UINT32(data + 60);
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

Bitmap::Bitmap(const char *data, int w, int h, const char *fname) {
	_fname = fname;
	if (gDebugLevel == DEBUG_BITMAPS || gDebugLevel == DEBUG_NORMAL || gDebugLevel == DEBUG_ALL)
		printf("New bitmap loaded: %s\n", fname);
	strcpy(_filename, fname);
	_currImage = 1;
	_numImages = 1;
	_x = 0;
	_y = 0;
	_width = w;
	_height = h;
	_format = 1;
	_numTex = 0;
	_texIds = NULL;
	_hasTransparency = false;
	_data = new char *[_numImages];
	_data[0] = new char[2 * _width * _height];
	memcpy(_data[0], data, 2 * _width * _height);
	g_driver->createBitmap(this);
}

void Bitmap::draw() const {
	if (_currImage == 0)
		return;

	g_driver->drawBitmap(this);
}

Bitmap::~Bitmap() {
	if (_data) {
		for (int i = 0; i < _numImages; i++)
			if (_data[i])
				delete[] _data[i];

		delete[] _data;
		_data = NULL;
	}
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

} // end of namespace Grim
