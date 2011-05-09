/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "common/endian.h"
#include "common/zlib.h"
#include "common/memstream.h"

#include "engines/grim/grim.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/savegame.h"
#include "engines/grim/colormap.h"

namespace Grim {

static void decompress_codec3(const char *compressed, char *result);

Common::HashMap<Common::String, BitmapData *> *BitmapData::_bitmaps = NULL;

// Helper function for makeBitmapFromTile
char *getLine(int lineNum, char *data, unsigned int width, int bpp) {
	return data + (lineNum *(width * bpp));
}

char *makeBitmapFromTile(char **bits, int width, int height, int bpp) {
	bpp = bpp / 8;
	char *fullImage = new char[width * height * bpp];

	const int tWidth = 256 * bpp; // All tiles so far are 256 wide
	const int tWidth2 = 256;

	char *target = fullImage;
	for (int i = 0; i < 256; i++) {
		/* This can be modified to actually use the last 32 lines.
		 * We simply put the lower half on line 223 and down to line 32,
		 * then skip the last 32.
		 * While the upper half is put on line 479 and down to line 224.
		 */
		if (i < 224) { // Skip blank space
			target=getLine(223 - i, fullImage, width, bpp);

			memcpy(target, getLine(i, bits[3], tWidth2, bpp), tWidth);
			target += tWidth;

			memcpy(target, getLine(i, bits[4], tWidth2, bpp), tWidth);
			target += tWidth;

			memcpy(target, getLine(i, bits[2], tWidth2, bpp) + 128 * bpp, 128 * bpp);
			target += tWidth / 2;
		}

		// Top half of course

		target = getLine(479 - i, fullImage, width, bpp);

		memcpy(target, getLine(i, bits[0], tWidth2, bpp), tWidth);
		target += tWidth;

		memcpy(target, getLine(i, bits[1], tWidth2, bpp), tWidth);
		target += tWidth;

		memcpy(target, getLine(i, bits[2], tWidth2, bpp), 128 * bpp);
		target += tWidth / 2;

	}

	return fullImage;
}

char *upconvertto32(char *data, unsigned int width, unsigned int height, int bpp) {
	if (bpp==16) {
		bpp = 2;
		char *newData = new char[width * height * 4];
		char *to = newData;
		char red = 0, green = 0, blue = 0;
		for (unsigned int i = 0; i< height; i++) {
			for(unsigned int j = 0; j < width; j++) {
				char byte2 = data[i * width * bpp + j *2];
				char byte1 = data[i * width * bpp + j * 2 + 1];
			// Probably Alpha, then 555.
			// Red
				red = (byte1 >> 2) & 31;
				red = red << 3 | red >> 2;
			// Green
				char green1 = (byte1 & 3);
				char green2 = (((byte2) >> 5) & 7);
				char green3 = green1 << 3 | green2;
				green = green3 << 3 | green3 >> 2 ;
			// Blue
				blue = (byte2) & 31;
				blue = blue << 3 | blue >> 2;
			// Some more magic to stretch the values
				*to = red;
				to++;
				*to = green;
				to++;
				*to = blue;
				to++;
				*to = 0;
				to++;
			}
		}
		delete data;
		return newData;
	}
	return data;
}

BitmapData *BitmapData::getBitmapData(const char *fname, const char *data, int len) {
	Common::String str(fname);
	if (_bitmaps && _bitmaps->contains(str)) {
		BitmapData *b = (*_bitmaps)[str];
		++b->_refCount;
		return b;
	}

	BitmapData *b = new BitmapData(fname, data, len);
	if (!_bitmaps) {
		_bitmaps = new Common::HashMap<Common::String, BitmapData *>();
	}
	(*_bitmaps)[str] = b;
	return b;
}

BitmapData::BitmapData(const char *fname, const char *data, int len) {
	_fname = fname;
	_refCount = 1;
	if (len > 4 && memcmp(data, "\x1f\x8b\x08\0", 4) == 0) {
		loadTile(fname, data, len);
		return;
	} else if (len < 8 || memcmp(data, "BM  F\0\0\0", 8) != 0) {
		if (gDebugLevel == DEBUG_BITMAPS || gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL)
			error("Invalid magic loading bitmap");
	}

	int codec = READ_LE_UINT32(data + 8);
//	_paletteIncluded = READ_LE_UINT32(data + 12);
	_numImages = READ_LE_UINT32(data + 16);
	_x = READ_LE_UINT32(data + 20);
	_y = READ_LE_UINT32(data + 24);
//	_transparentColor = READ_LE_UINT32(data + 28);
	_format = READ_LE_UINT32(data + 32);
	_bpp = READ_LE_UINT32(data + 36);
//	_blueBits = READ_LE_UINT32(data + 40);
//	_greenBits = READ_LE_UINT32(data + 44);
//	_redBits = READ_LE_UINT32(data + 48);
//	_blueShift = READ_LE_UINT32(data + 52);
//	_greenShift = READ_LE_UINT32(data + 56);
//	_redShift = READ_LE_UINT32(data + 60);
	_width = READ_LE_UINT32(data + 128);
	_height = READ_LE_UINT32(data + 132);

	_data = new char *[_numImages];
	int pos = 0x88;
	for (int i = 0; i < _numImages; i++) {
		_data[i] = new char[_bpp / 8 * _width * _height];
		if (codec == 0) {
			memcpy(_data[i], data + pos, _bpp / 8 * _width * _height);
			pos += _bpp / 8 * _width * _height + 8;
		} else if (codec == 3) {
			int compressed_len = READ_LE_UINT32(data + pos);
			decompress_codec3(data + pos + 4, _data[i]);
			pos += compressed_len + 12;
		}

#ifdef SCUMM_BIG_ENDIAN
		if (_format == 1)
			for (int j = 0; j < _width * _height; ++j) {
				((uint16 *)_data[i])[j] = SWAP_BYTES_16(((uint16 *)_data[i])[j]);
			}
#endif
	}

	g_driver->createBitmap(this);
}

BitmapData::BitmapData(const char *data, int w, int h, int bpp, const char *fname) {
	_fname = fname;
	_refCount = 1;
	if (gDebugLevel == DEBUG_BITMAPS || gDebugLevel == DEBUG_NORMAL || gDebugLevel == DEBUG_ALL)
		printf("New bitmap loaded: %s\n", fname);
	_numImages = 1;
	_x = 0;
	_y = 0;
	_width = w;
	_height = h;
	_format = 1;
	_numTex = 0;
	_texIds = NULL;
	_bpp = bpp;
	_hasTransparency = false;
	_data = new char *[_numImages];
	_data[0] = new char[_bpp / 8 * _width * _height];
	memcpy(_data[0], data, _bpp / 8 * _width * _height);
	g_driver->createBitmap(this);
}

BitmapData::BitmapData() :
		_data(NULL), _refCount(1) {

}

BitmapData::~BitmapData() {
	if (_data) {
		for (int i = 0; i < _numImages; i++)
			if (_data[i])
				delete[] _data[i];

		delete[] _data;
		_data = NULL;

		g_driver->destroyBitmap(this);
	}
	if (_bitmaps) {
		if (_bitmaps->contains(_fname)) {
			_bitmaps->erase(_fname);
		}
		if (_bitmaps->empty()) {
			delete _bitmaps;
			_bitmaps = NULL;
		}
	}
}

bool BitmapData::loadTile(const char *filename, const char *data, int len) {
	_x = 0;
	_y = 0;
	//warning("Loading TILE: %s",filename);
	Common::MemoryReadStream stream((const byte *)data, len);
	Common::SeekableReadStream *o = Common::wrapCompressedReadStream(&stream);

	uint32 id, bmoffset;
	id = o->readUint32LE();
	// Should check that we actually HAVE a TIL
	bmoffset = o->readUint32LE();
	o->seek(bmoffset + 16);
	int numSubImages = o->readUint32LE();
	if (numSubImages < 5)
		error("Can not handle a tile with less than 5 sub images");

	_data = new char *[numSubImages];

	o->seek(16, SEEK_CUR);
	_bpp = o->readUint32LE();

	o->seek(bmoffset + 128);

	_width = o->readUint32LE();
	_height = o->readUint32LE();
	o->seek(-8, SEEK_CUR);

	int size = _bpp / 8 * _width * _height;
	for (int i = 0; i < numSubImages; ++i) {
		_data[i] = new char[size];
		o->seek(8, SEEK_CUR);
		o->read(_data[i], size);
	}
	char* bMap = makeBitmapFromTile(_data, 640, 480, _bpp);
	for (int i = 0; i < numSubImages; ++i) {
		delete[] _data[i];
	}
	_data[0] = bMap;
	_numImages = 1;

	if (_bpp == 16) {
		_data[0] = upconvertto32(_data[0], 640, 480, _bpp);
		_bpp = 32;
	}
	_width = 640;
	_height = 480;

	g_driver->createBitmap(this);
	return true;
}

// Bitmap

Bitmap::Bitmap(const char *fname, const char *data, int len) :
Object() {
	_data = BitmapData::getBitmapData(fname, data, len);
	_x = _data->_x;
	_y = _data->_y;
	_currImage = 1;
}

Bitmap::Bitmap(const char *data, int w, int h, int bpp, const char *fname) :
Object() {
	_data = new BitmapData(data, w, h, bpp, fname);
	_x = _data->_x;
	_y = _data->_y;
	_currImage = 1;
}

Bitmap::Bitmap() :
Object() {
	_data = new BitmapData();
}

void Bitmap::draw() const {
	if (_currImage == 0)
		return;

	g_driver->drawBitmap(this);
}

Bitmap::~Bitmap() {
	--_data->_refCount;
	if (_data->_refCount < 1) {
		delete _data;
	}
	g_grim->killBitmap(this);
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
