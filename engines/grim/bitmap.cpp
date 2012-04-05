/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "graphics/colormasks.h"
#include "graphics/pixelbuffer.h"

#include "engines/grim/savegame.h"
#include "engines/grim/debug.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/resource.h"
#include "engines/grim/gfx_base.h"

namespace Grim {

static bool decompress_codec3(const char *compressed, char *result, int maxBytes);

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

BitmapData *BitmapData::getBitmapData(const Common::String &fname) {
	Common::String str(fname);
	if (_bitmaps && _bitmaps->contains(str)) {
		BitmapData *b = (*_bitmaps)[str];
		++b->_refCount;
		return b;
	}

	BitmapData *b = new BitmapData(fname);
	if (!_bitmaps) {
		_bitmaps = new Common::HashMap<Common::String, BitmapData *>();
	}
	(*_bitmaps)[str] = b;
	return b;
}

BitmapData::BitmapData(const Common::String &fname) {
	_fname = fname;
	_refCount = 1;
	_data = 0;
	_loaded = false;
	_keepData = true;
}

void BitmapData::load() {
	if (_loaded) {
		return;
	}
	Common::SeekableReadStream *data = g_resourceloader->openNewStreamFile(_fname.c_str());

	uint32 tag = data->readUint32BE();
	switch(tag) {
		case(MKTAG('B','M',' ',' ')):				//Grim bitmap
			loadGrimBm(data);
			break;
		case(MKTAG('T','I','L','0')):				// MI4 bitmap
			loadTile(data);
			break;
		default:
			if (!loadTGA(data))	// Try to load as TGA.
				Debug::error(Debug::Bitmaps, "Invalid magic loading bitmap");
			break;
	}
	delete data;
	_loaded = true;
}

bool BitmapData::loadGrimBm(Common::SeekableReadStream *data) {
	uint32 tag2 = data->readUint32BE();
	if(tag2 != (MKTAG('F','\0','\0','\0')))
		return false;

	int codec = data->readUint32LE();
	data->readUint32LE(); 				//_paletteIncluded
	_numImages = data->readUint32LE();
	_x = data->readUint32LE();
	_y = data->readUint32LE();
	data->readUint32LE(); 				//_transparentColor
	_format = data->readUint32LE();
	_bpp = data->readUint32LE();
// 	uint32 redBits = data->readUint32LE();
// 	uint32 greenBits = data->readUint32LE();
// 	uint32 blueBits = data->readUint32LE();
// 	uint32 redShift = data->readUint32LE();
// 	uint32 greenShift = data->readUint32LE();
// 	uint32 blueShift = data->readUint32LE();

	// Hardcode the format, since the values saved in the files are garbage for some, like "ha_0_elvos.zbm".
	Graphics::PixelFormat pixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);

	data->seek(128, SEEK_SET);
	_width = data->readUint32LE();
	_height = data->readUint32LE();
	_colorFormat = BM_RGB565;
	_hasTransparency = false;

	_data = new Graphics::PixelBuffer[_numImages];
	data->seek(0x80, SEEK_SET);
	for (int i = 0; i < _numImages; i++) {
		data->seek(8, SEEK_CUR);
		_data[i].create(pixelFormat, _width * _height, DisposeAfterUse::YES);
		if (codec == 0) {
			uint32 dsize = _bpp / 8 * _width * _height;
			data->read(_data[i].getRawBuffer(), dsize);
		} else if (codec == 3) {
			int compressed_len = data->readUint32LE();
			char *compressed = new char[compressed_len];
			data->read(compressed, compressed_len);
			bool success = decompress_codec3(compressed, (char *)_data[i].getRawBuffer(), _bpp / 8 * _width * _height);
			delete[] compressed;
			if (!success)
				warning(".. when loading image %s.\n", _fname.c_str());
		} else
			Debug::error(Debug::Bitmaps, "Unknown image codec in BitmapData ctor!");

#ifdef SCUMM_BIG_ENDIAN
		if (_format == 1) {
			uint16 *d = (uint16 *)_data[i].getRawBuffer();
			for (int j = 0; j < _width * _height; ++j) {
				d[j] = SWAP_BYTES_16(d[j]);
			}
		}
#endif
	}

	// Initially, no GPU-side textures created. the createBitmap
	// function will allocate some if necessary (and successful)
	_numTex = 0;
	_texIds = NULL;

	g_driver->createBitmap(this);
	return true;
}

BitmapData::BitmapData(const Graphics::PixelBuffer &buf, int w, int h, const char *fname) {
	_fname = fname;
	_refCount = 1;
	Debug::debug(Debug::Bitmaps, "New bitmap loaded: %s\n", fname);
	_numImages = 1;
	_x = 0;
	_y = 0;
	_width = w;
	_height = h;
	_format = 1;
	_numTex = 0;
	_texIds = NULL;
	_bpp = buf.getFormat().bytesPerPixel * 8;
	_hasTransparency = false;
	_colorFormat = BM_RGB565;
	_data = new Graphics::PixelBuffer[_numImages];
	_data[0].create(buf.getFormat(), w * h, DisposeAfterUse::YES);
	_data[0].copyBuffer(0, w * h, buf);
	_loaded = true;
	_keepData = true;

	g_driver->createBitmap(this);
}

BitmapData::BitmapData() :
	_numImages(0), _width(0), _height(0), _x(0), _y(0), _format(0), _numTex(0),
	_bpp(0), _colorFormat(0), _texIds(0), _hasTransparency(false), _data(NULL), _refCount(1), _loaded(false) {
}

BitmapData::~BitmapData() {
	_keepData = false;
	if (_loaded) {
		g_driver->destroyBitmap(this);
	}
	freeData();
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

void BitmapData::freeData() {
	if (!_keepData) {
		delete[] _data;
		_data = NULL;
	}
}

bool BitmapData::loadTGA(Common::SeekableReadStream *data) {
	data->seek(0, SEEK_SET);
	if (data->readByte() != 0)	// Verify that description-field is empty
		return false;
	data->seek(1, SEEK_CUR);

	int format = data->readByte();
	if (format != 2)
		return false;

	data->seek(9, SEEK_CUR);
	_width = data->readUint16LE();
	_height = data->readUint16LE();;
	_format = 1;
	_x = 0;
	_y = 0;

	int bpp = data->readByte();
	Graphics::PixelFormat pixelFormat;
	if (bpp == 32) {
		_colorFormat = BM_RGBA;
		pixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
		_bpp = 4;
	} else {
		return false;
	}

	uint8 desc = data->readByte();
	uint8 flipped = !(desc & 32);

	if (!(bpp == 24 || bpp == 32)) // Assure we have 24/32 bpp
		return false;
	_data = new Graphics::PixelBuffer[1];
	_data[0].create(pixelFormat, _width * _height, DisposeAfterUse::YES);
	char *writePtr = (char *)_data[0].getRawBuffer(_width * (_height - 1));

	if (flipped) {
		for (int i = 0; i < _height; i++) {
			data->read(writePtr, _width * (bpp / 8));
			writePtr -= (_width * bpp / 8);
		}
	} else {
		data->read(_data[0].getRawBuffer(), _width * _height * (bpp / 8));
	}

	uint8 x;
	for (int i = 0; i < _width * _height * (bpp / 8); i+=4) {
		byte *b = _data[0].getRawBuffer();
		x = b[i];
		b[i] = b[i + 2];
		b[i + 2] = x;
	}

	_numImages = 1;
	g_driver->createBitmap(this);
	return true;
}

bool BitmapData::loadTile(Common::SeekableReadStream *o) {
#ifdef ENABLE_MONKEY4
	_x = 0;
	_y = 0;
	_format = 1;
	o->seek(0, SEEK_SET);
	//warning("Loading TILE: %s",fname.c_str());

	uint32 id, bmoffset;
	id = o->readUint32LE();
	// Should check that we actually HAVE a TIL
	bmoffset = o->readUint32LE();
	o->seek(bmoffset + 16);
	int numSubImages = o->readUint32LE();
	if (numSubImages < 5)
		error("Can not handle a tile with less than 5 sub images");

	char **data = new char *[numSubImages];

	o->seek(16, SEEK_CUR);
	_bpp = o->readUint32LE();

	o->seek(bmoffset + 128);

	_width = o->readUint32LE();
	_height = o->readUint32LE();
	o->seek(-8, SEEK_CUR);

	int size = _bpp / 8 * _width * _height;
	for (int i = 0; i < numSubImages; ++i) {
		data[i] = new char[size];
		o->seek(8, SEEK_CUR);
		o->read(data[i], size);
	}

	char *bMap = makeBitmapFromTile(data, 640, 480, _bpp);
	for (int i = 0; i < numSubImages; ++i) {
		delete[] data[i];
	}
	delete[] data;
	Graphics::PixelFormat pixelFormat;
	if (_bpp == 16) {
		_colorFormat = BM_RGB1555;
		pixelFormat = Graphics::createPixelFormat<1555>();
		//convertToColorFormat(0, BM_RGBA);
	} else {
		pixelFormat = Graphics::PixelFormat(4, 8,8,8,8, 0, 8, 16, 24);
		_colorFormat = BM_RGBA;
	}

	_width = 640;
	_height = 480;
	_numImages = 1;
	_data = new Graphics::PixelBuffer[_numImages];
	_data[0].create(pixelFormat, _width * _height, DisposeAfterUse::YES);
	_data[0].set(pixelFormat, (byte *)bMap);

	g_driver->createBitmap(this);
#endif // ENABLE_MONKEY4
	return true;
}

const Graphics::PixelBuffer &BitmapData::getImageData(int num) const {
	assert(num >= 0);
	assert(num < _numImages);
	return _data[num];
}

// Bitmap

Bitmap::Bitmap(const Common::String &fname) {
	_data = BitmapData::getBitmapData(fname);
	_currImage = 1;
}

Bitmap::Bitmap(const Graphics::PixelBuffer &buf, int w, int h, const char *fname) {
	_data = new BitmapData(buf, w, h, fname);
	_currImage = 1;
}

Bitmap::Bitmap() {
	_data = new BitmapData();
}

Bitmap *Bitmap::create(const Common::String &filename) {
	if (!g_resourceloader->getFileExists(filename)) {
		warning("Could not find bitmap %s", filename.c_str());
		return NULL;
	}
	Bitmap *b = new Bitmap(filename);
	return b;
}

void Bitmap::saveState(SaveGame *state) const {
	state->writeString(getFilename());

	state->writeLESint32(getActiveImage());
}

void Bitmap::restoreState(SaveGame *state) {
	freeData();

	Common::String fname = state->readString();
	_data = BitmapData::getBitmapData(fname);

	_currImage = state->readLESint32();
}

void Bitmap::draw() {
	_data->load();
	if (_currImage == 0)
		return;

	g_driver->drawBitmap(this, _data->_x, _data->_y);
}

void Bitmap::draw(int x, int y) {
	_data->load();
	if (_currImage == 0)
		return;

	g_driver->drawBitmap(this, x, y);
}

void Bitmap::setActiveImage(int n) {
	assert(n >= 0);
	_data->load();
	if ((n - 1) >= _data->_numImages) {
		warning("Bitmap::setActiveImage: no anim image: %d. (%s)", n, _data->_fname.c_str());
	} else {
		_currImage = n;
	}
}

int Bitmap::getNumImages() const {
	_data->load();
	return _data->_numImages;
}

void Bitmap::freeData() {
	--_data->_refCount;
	if (_data->_refCount < 1) {
		delete _data;
		_data = 0;
	}
}

Bitmap::~Bitmap() {
	freeData();
}

const Graphics::PixelFormat &Bitmap::getPixelFormat(int num) const {
	return getData(num).getFormat();
}

void BitmapData::convertToColorFormat(int num, const Graphics::PixelFormat &format) {
	if (_data[num].getFormat() == format) {
		return;
	}

	Graphics::PixelBuffer dst(format, _width * _height, DisposeAfterUse::NO);

	for (int i = 0; i < _width * _height; ++i) {
		if (_data[num].getValueAt(i) == 0xf81f) { //transparency
			dst.setPixelAt(i, 0xf81f);
		} else {
			dst.setPixelAt(i, _data[num]);
		}
	}
	_data[num].free();
	_data[num] = dst;
}

void BitmapData::convertToColorFormat(const Graphics::PixelFormat &format) {
	for (int i = 0; i < _numImages; ++i) {
		convertToColorFormat(i, format);
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

static bool decompress_codec3(const char *compressed, char *result, int maxBytes) {
	int bitstr_value = READ_LE_UINT16(compressed);
	int bitstr_len = 16;
	compressed += 2;
	bool bit;

	int byteIndex = 0;
	for (;;) {
		GET_BIT;
		if (bit == 1) {
			if (byteIndex >= maxBytes) {
				warning("Buffer overflow when decoding image: decompress_codec3 walked past the input buffer!");
				return false;
			}
			else
				*result++ = *compressed++;
			++byteIndex;
		}
		else {
			GET_BIT;
			int copy_len, copy_offset;
			if (bit == 0) {
				GET_BIT;
				copy_len = 2 * bit;
				GET_BIT;
				copy_len += bit + 3;
				copy_offset = *(const uint8 *)(compressed++) - 0x100;
			} else {
				copy_offset = (*(const uint8 *)(compressed) | (*(const uint8 *)(compressed + 1) & 0xf0) << 4) - 0x1000;
				copy_len = (*(const uint8 *)(compressed + 1) & 0xf) + 3;
				compressed += 2;
				if (copy_len == 3) {
					copy_len = *(const uint8 *)(compressed++) + 1;
					if (copy_len == 1)
						return true;
				}
			}
			while (copy_len > 0) {
				if (byteIndex >= maxBytes) {
					warning("Buffer overflow when decoding image: decompress_codec3 walked past the input buffer!");
					return false;
				}
				else {
					assert(byteIndex + copy_offset >= 0);
					assert(byteIndex + copy_offset < maxBytes);
					*result = result[copy_offset];
					result++;
				}
				++byteIndex;
				copy_len--;
			}
		}
	}
	return true;
}

} // end of namespace Grim
