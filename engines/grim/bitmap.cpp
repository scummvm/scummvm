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
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "common/endian.h"
#include "common/zlib.h"
#include "common/memstream.h"

#include "engines/grim/debug.h"
#include "engines/grim/grim.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/resource.h"
#include "engines/grim/lab.h"
#include "engines/grim/gfx_base.h"

namespace Grim {

static void decompress_codec3(const char *compressed, char *result);

Common::HashMap<Common::String, BitmapData *> *BitmapData::_bitmaps = NULL;

// Helper function for makeBitmapFromTile
char *getLine(int lineNum, char *data, unsigned int width, int bpp) {
	return data + (lineNum *(width * bpp));
}

#ifdef ENABLE_MONKEY4

char *makeBitmapFromTile(char **bits, int width, int height, int bpp) {
	bpp = bpp / 8;
	char *fullImage = new char[width * height * bpp];

	const int tWidth = 256 * bpp; // All tiles so far are 256 wide
	const int tWidth2 = 256;

	char *target = fullImage;
	int line;
	for (int i = 0; i < 256; i++) {
		/* This can be modified to actually use the last 32 lines.
		 * We simply put the lower half on line 223 and down to line 32,
		 * then skip the last 32.
		 * While the upper half is put on line 479 and down to line 224.
		 */

		if (i < 224) { // Skip blank space
			line = 224 - i;
			target = getLine(479 - i, fullImage, width, bpp);

			memcpy(target, getLine(line, bits[3], tWidth2, bpp), tWidth);
			target += tWidth;

			memcpy(target, getLine(line, bits[4], tWidth2, bpp), tWidth);
			target += tWidth;

			memcpy(target, getLine(line, bits[2], tWidth2, bpp) + 128 * bpp, 128 * bpp);
		}
		line = 255 - i;
		// Top half of course

		target = getLine(line, fullImage, width, bpp);

		memcpy(target, getLine(line, bits[0], tWidth2, bpp), tWidth);
		target += tWidth;

		memcpy(target, getLine(line, bits[1], tWidth2, bpp), tWidth);
		target += tWidth;

		memcpy(target, getLine(line, bits[2], tWidth2, bpp), 128 * bpp);

	}

	return fullImage;
}

#endif

BitmapData *BitmapData::getBitmapData(const Common::String &fname, const char *data, int len) {
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

BitmapData::BitmapData(const Common::String &fname, const char *data, int len) {
	_fname = fname;
	_refCount = 1;
	if (len > 4 && memcmp(data, "\x1f\x8b\x08\0", 4) == 0) {
		loadTile(data, len);
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
	_colorFormat = BM_RGB565;

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
	_colorFormat = BM_RGB565;
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

bool BitmapData::loadTile(const char *data, int len) {
#ifdef ENABLE_MONKEY4
	_x = 0;
	_y = 0;
	_format = 1;
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

	char *bMap = makeBitmapFromTile(_data, 640, 480, _bpp);
	for (int i = 0; i < numSubImages; ++i) {
		delete[] _data[i];
	}
	_width = 640;
	_height = 480;
	_data[0] = bMap;
	_numImages = 1;

	if (_bpp == 16) {
		_colorFormat = BM_RGB1555;
		//convertToColorFormat(0, BM_RGBA);
	} else {
		_colorFormat = BM_RGBA;
	}

	g_driver->createBitmap(this);
	return true;
#endif // ENABLE_MONKEY4
}

char *BitmapData::getImageData(int num) const {
	return _data[num];
}

// Bitmap

Bitmap::Bitmap(const Common::String &fname, const char *data, int len) :
		PoolObject<Bitmap, MKTAG('V', 'B', 'U', 'F')>() {
	_data = BitmapData::getBitmapData(fname, data, len);
	_x = _data->_x;
	_y = _data->_y;
	_currImage = 1;
}

Bitmap::Bitmap(const char *data, int w, int h, int bpp, const char *fname) :
		PoolObject<Bitmap, MKTAG('V', 'B', 'U', 'F')>() {
	_data = new BitmapData(data, w, h, bpp, fname);
	_x = _data->_x;
	_y = _data->_y;
	_currImage = 1;
}

Bitmap::Bitmap() :
		PoolObject<Bitmap, MKTAG('V', 'B', 'U', 'F')>() {
	_data = new BitmapData();
}

void Bitmap::saveState(SaveGame *state) const {
	state->writeString(getFilename());

	state->writeLESint32(getCurrentImage());
	state->writeLESint32(getX());
	state->writeLESint32(getY());
}

void Bitmap::restoreState(SaveGame *state) {
	freeData();

	Common::String fname = state->readString();
	Block *b = g_resourceloader->getBlock(fname);
	_data = BitmapData::getBitmapData(fname, b->getData(), b->getLen());

	_currImage = state->readLESint32();
	_x = state->readLESint32();
	_y = state->readLESint32();
}

void Bitmap::draw() const {
	if (_currImage == 0)
		return;

	g_driver->drawBitmap(this);
}

void Bitmap::setNumber(int n) {
	if ((n - 1) >= _data->_numImages) {
		warning("Bitmap::setNumber: no anim image: %d. (%s)", n, _data->_fname.c_str());
	} else {
		_currImage = n;
	}
}

void Bitmap::freeData() {
	--_data->_refCount;
	if (_data->_refCount < 1) {
		delete _data;
	}
}

Bitmap::~Bitmap() {
	freeData();
}

void BitmapData::convertToColorFormat(int num, int format) {
	// Supports 1555->RGBA, RGBA->565
	unsigned char red = 0, green = 0, blue = 0, alpha = 0;
	int size = _width * _height * (_bpp / 8);
	if (_colorFormat == BM_RGB1555) {
		uint16 *bitmapData = reinterpret_cast<uint16 *>(_data[num]);

		if (format == BM_RGBA && _bpp == 16) {
			// Convert data to 32-bit RGBA format
			char *newData = new char[_width * _height * 4];
			char *to = newData;

			for (int i = 0; i< _height * _width; i++, bitmapData++, to += 4) {
				uint pixel = *bitmapData;
				// Alpha, then 555 (BGR).
				blue = (pixel >> 10) & 0x1f;
				to[2] = blue << 3 | blue >> 2;
				green = (pixel >> 5) & 0x1f;
				to[1] = green << 3 | green >> 2;
				red = (pixel & 0x1f);
				to[0] = red << 3 | red >> 2;

				if (pixel >> 15 & 1)
					alpha = 255;
				else
					alpha = 0;
				to[3] = alpha;
			}
			delete _data[num];
			_data[num] = newData;
			_colorFormat = BM_RGBA;
			_bpp = 32;
		} else if (format == BM_RGB565){ // 1555 -> 565 (Incomplete)
			convertToColorFormat(num, BM_RGBA);
			convertToColorFormat(num, BM_RGB565);
			warning("Conversion 1555->565 done with 1555->RGBA->565");
			return;
			warning("Conversion 1555->565 is not properly implemented");
			// This doesn't work properly, so falling back to double-conversion via RGBA for now.
			uint16 *to = reinterpret_cast<uint16 *>(_data[num]);
			for (int i = 1; i < _height * _width; i++, bitmapData++, to++) {
				uint pixel = *bitmapData;
				// Alpha, then 555.
				if (to[0] & 128) { // Chroma key
					to[0] = 0xf8;
					to[1] = 0x1f;
				} else {
					blue = (pixel >> 10) & 0x1f;
					//red = red << 3 | red >> 2;
					green = (pixel >> 5) & 0x1f;
					green = green << 1 | green >> 1;
					red = (pixel) & 0x1f;
					to[0] = (red << 3) | green >> 3;
					to[1] = (green << 5) | blue;
				}
			}
			_colorFormat = BM_RGB565;
		}
	} else if (_colorFormat == BM_RGBA) {
		if (format == BM_RGB565) { // RGBA->565
			char* tempStore = _data[num];
			char* newStore = new char[size / 2];
			uint16 *to = reinterpret_cast<uint16 *>(newStore);
			for(int j = 0; j < size;j += 4, to++){
				red = (tempStore[j] >> 3) & 0x1f;
				green = (tempStore[j + 1] >> 2) & 0x3f;
				blue = (tempStore[j + 2] >> 3) &0x1f;
				*to = (red << 11) | (green << 5) | blue;
			}
			delete[] tempStore;
			_data[num] = newStore;
			_colorFormat = BM_RGB565;
		}

	} else if (_colorFormat == BM_RGB565) {
		if (format == BM_RGBA && _bpp == 16) {
			byte *tempData = new byte[4 * _width * _height];
			// Convert data to 32-bit RGBA format
			byte *tempDataPtr = tempData;
			uint16 *bitmapData = reinterpret_cast<uint16 *>(_data[num]);
			for (int i = 0; i < _width * _height; i++, tempDataPtr += 4, bitmapData++) {
				uint16 pixel = *bitmapData;
				int r = pixel >> 11;
				tempDataPtr[0] = (r << 3) | (r >> 2);
				int g = (pixel >> 5) & 0x3f;
				tempDataPtr[1] = (g << 2) | (g >> 4);
				int b = pixel & 0x1f;
				tempDataPtr[2] = (b << 3) | (b >> 2);
				if (pixel == 0xf81f) { // transparent
					tempDataPtr[3] = 0;
					_hasTransparency = true;
				} else {
					tempDataPtr[3] = 255;
				}
			}
			delete[] _data[num];
			_data[num] = (char *)tempData;
			_colorFormat = BM_RGBA;
			_bpp = 32;
		}
	} else {
		error("Conversion between format: %d and format %d not implemented",_colorFormat, format);
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
