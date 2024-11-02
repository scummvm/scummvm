/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/endian.h"

#include "image/tga.h"

#include "engines/grim/savegame.h"
#include "engines/grim/debug.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/resource.h"
#include "engines/grim/gfx_base.h"

namespace Grim {

static bool decompress_codec3(const char *compressed, char *result, int maxBytes);

Common::HashMap<Common::String, BitmapData *> *BitmapData::_bitmaps = nullptr;

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
	_data = nullptr;
	_loaded = false;
	_keepData = true;

	// Initialize members to avoid warnings:
	_numImages = 0;
	_width = 0;
	_height = 0;
	_x = 0;
	_y = 0;
	_format = 0;
	_numTex = 0;
	_bpp = 0;
	_texIds = nullptr;
	_hasTransparency = 0;

	_texc = nullptr;

	_verts = nullptr;
	_layers = nullptr;

	_numCoords = 0;
	_numVerts = 0;
	_numLayers = 0;

	_userData = nullptr;
}

void BitmapData::load() {
	if (_loaded) {
		return;
	}
	Common::SeekableReadStream *data = g_resourceloader->openNewStreamFile(_fname.c_str());
	if (!data)
		error("Couldn't open %s", _fname.c_str());

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
	if (tag2 != (MKTAG('F','\0','\0','\0')))
		return false;

	int codec = data->readUint32LE();
	data->readUint32LE();               //_paletteIncluded
	_numImages = data->readUint32LE();
	_x = data->readUint32LE();
	_y = data->readUint32LE();
	data->readUint32LE();               //_transparentColor
	_format = data->readUint32LE();
	_bpp = data->readUint32LE();
//	uint32 redBits = data->readUint32LE();
//	uint32 greenBits = data->readUint32LE();
//	uint32 blueBits = data->readUint32LE();
//	uint32 redShift = data->readUint32LE();
//	uint32 greenShift = data->readUint32LE();
//	uint32 blueShift = data->readUint32LE();

	// Hardcode the format, since the values saved in the files are garbage for some, like "ha_0_elvos.zbm".
	Graphics::PixelFormat pixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);

	data->seek(128, SEEK_SET);
	_width = data->readUint32LE();
	_height = data->readUint32LE();
	_hasTransparency = false;

	_data = new Graphics::Surface[_numImages];
	data->seek(0x80, SEEK_SET);
	for (int i = 0; i < _numImages; i++) {
		data->seek(8, SEEK_CUR);
		_data[i].create(_width, _height, pixelFormat);
		if (codec == 0) {
			uint32 dsize = _bpp / 8 * _width * _height;
			data->read(_data[i].getPixels(), dsize);
		} else if (codec == 3) {
			int compressed_len = data->readUint32LE();
			char *compressed = new char[compressed_len];
			data->read(compressed, compressed_len);
			bool success = decompress_codec3(compressed, (char *)_data[i].getPixels(), _bpp / 8 * _width * _height);
			delete[] compressed;
			if (!success)
				warning(".. when loading image %s.", _fname.c_str());
		} else
			Debug::error(Debug::Bitmaps, "Unknown image codec in BitmapData ctor!");

#ifdef SCUMM_BIG_ENDIAN
		uint16 *d = (uint16 *)_data[i].getPixels();
		for (int j = 0; j < _width * _height; ++j) {
			d[j] = SWAP_BYTES_16(d[j]);
		}
#endif
	}

	// Initially, no GPU-side textures created. the createBitmap
	// function will allocate some if necessary (and successful)
	_numTex = 0;
	_texIds = nullptr;

	g_driver->createBitmap(this);
	return true;
}

BitmapData::BitmapData(const Graphics::Surface &buf, int w, int h, const char *fname) : _fname(fname) {
	_refCount = 1;
	Debug::debug(Debug::Bitmaps, "New bitmap loaded: %s\n", fname);
	_numImages = 1;
	_x = 0;
	_y = 0;
	_width = w;
	_height = h;
	_format = 1;
	_numTex = 0;
	_texIds = nullptr;
	_bpp = buf.format.bytesPerPixel * 8;
	_hasTransparency = false;
	_data = new Graphics::Surface[_numImages];
	_data[0].copyFrom(buf);
	_loaded = true;
	_keepData = true;

	_userData = nullptr;
	_texc = nullptr;
	_verts = nullptr;
	_layers = nullptr;

	g_driver->createBitmap(this);
}

BitmapData::BitmapData() :
		_numImages(0), _width(0), _height(0), _x(0), _y(0), _format(0), _numTex(0),
		_bpp(0), _texIds(nullptr), _hasTransparency(false), _data(nullptr),
		_refCount(1), _loaded(false), _keepData(false), _texc(nullptr), _verts(nullptr),
		_layers(nullptr), _numCoords(0), _numVerts(0), _numLayers(0), _userData(nullptr) {
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
			_bitmaps = nullptr;
		}
	}
	delete[] _texc;
	delete[] _layers;
	delete[] _verts;
}

void BitmapData::freeData() {
	if (!_keepData && _data) {
		for (int i = 0; i < _numImages; ++i) {
			_data[i].free();
		}
		delete[] _data;
		_data = nullptr;
	}
}

bool BitmapData::loadTGA(Common::SeekableReadStream *data) {
	Image::TGADecoder dec;
	bool success = dec.loadStream(*data);

	if (!success)
		return false;

	const Graphics::Surface *surf = dec.getSurface();

	_width = surf->w;
	_height = surf->h;
	_format = 1;
	_x = _y = 0;
	_bpp = surf->format.bytesPerPixel * 8;
	_numImages = 1;
	_data = new Graphics::Surface[1];
	_data[0].copyFrom(*surf);

	g_driver->createBitmap(this);

	freeData();

	return true;
}

bool BitmapData::loadTile(Common::SeekableReadStream *o) {
#ifdef ENABLE_MONKEY4
	_x = 0;
	_y = 0;
	_format = 1;
	o->seek(0, SEEK_SET);

	/*uint32 id = */o->readUint32LE();
	// Should check that we actually HAVE a TIL
	uint32 bmoffset = o->readUint32LE();
	_numCoords = o->readUint32LE();
	_numLayers = o->readUint32LE();
	_numVerts = o->readUint32LE();

	// skip some 0
	o->seek(16, SEEK_CUR);
	_texc = new float[_numCoords * 4];

	for (uint32 i = 0; i < _numCoords * 4; ++i) {
		_texc[i] = o->readFloatLE();
	}

	_layers = new Layer[_numLayers];
	for (uint32 i = 0; i < _numLayers; ++i) {
		_layers[i]._offset = o->readUint32LE();
		_layers[i]._numImages = o->readUint32LE();
	}

	_verts = new Vert[_numVerts];
	for (uint32 i = 0; i < _numVerts; ++i) {
		_verts[i]._texid = o->readUint32LE();
		_verts[i]._pos = o->readUint32LE();
		_verts[i]._verts = o->readUint32LE();
	}

	o->seek(16, SEEK_CUR);
	_numImages = o->readUint32LE();

	o->seek(16, SEEK_CUR);
	_bpp = o->readUint32LE();

	o->seek(bmoffset + 128);

	_width = o->readUint32LE();
	_height = o->readUint32LE();
	o->seek(-8, SEEK_CUR);

	_data = new Graphics::Surface[_numImages];

	Graphics::PixelFormat format_16bpp = Graphics::PixelFormat(2, 5, 5, 5, 1, 0, 5, 10, 15);
#ifdef SCUMM_BIG_ENDIAN
	Graphics::PixelFormat format_32bpp = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
#else
	Graphics::PixelFormat format_32bpp = Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#endif

	int width = 256;
	int height = 256;

	for (int i = 0; i < _numImages; ++i) {
		_data[i].create(width, height, (_bpp == 16) ? format_16bpp : format_32bpp);
		uint8 *d = (uint8 *)_data[i].getPixels();
		o->seek(8, SEEK_CUR);
		switch (_bpp) {
		case 16:
#ifndef SCUMM_LITTLE_ENDIAN
			for (int y = 0; y < _height; ++y) {
				uint16 *d16 = (uint16 *)d;
				for (int x = 0; x < _width; ++x)
					d16[x] = o->readUint16LE();
				d += _data[i].pitch;
			}
			break;
#endif
		case 32:
			for (int y = 0; y < _height; ++y) {
				o->read(d, _width * (_bpp / 8));
				d += _data[i].pitch;
			}
			break;
		}
	}

	_width = width;
	_height = height;

	g_driver->createBitmap(this);
#endif // ENABLE_MONKEY4
	return true;
}

const Graphics::Surface &BitmapData::getImageData(int num) const {
	assert(num >= 0);
	assert(num < _numImages);
	return _data[num];
}

// Bitmap

Bitmap::Bitmap(const Common::String &fname) {
	_data = BitmapData::getBitmapData(fname);
	_currImage = 1;
}

Bitmap::Bitmap(const Graphics::Surface &buf, int w, int h, const char *fname) {
	_data = new BitmapData(buf, w, h, fname);
	_currImage = 1;
}

Bitmap::Bitmap() {
	_data = new BitmapData();
	_currImage = 0;
}

Bitmap *Bitmap::create(const Common::String &filename) {
	if (!SearchMan.hasFile(Common::Path(filename))) {
		warning("Could not find bitmap %s", filename.c_str());
		return nullptr;
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

	g_driver->drawBitmap(this, x, y, _data->_numLayers - 1);
}

void Bitmap::drawLayer(uint32 layer) {
	_data->load();
	if (_currImage == 0)
		return;

	g_driver->drawBitmap(this, _data->_x, _data->_y, layer);
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

int Bitmap::getNumLayers() const {
	_data->load();
	return _data->_numLayers;
}

void Bitmap::freeData() {
	--_data->_refCount;
	if (_data->_refCount < 1) {
		delete _data;
		_data = nullptr;
	}
}

Bitmap::~Bitmap() {
	freeData();
}

const Graphics::PixelFormat &Bitmap::getPixelFormat(int num) const {
	return getData(num).format;
}

void BitmapData::convertToColorFormat(int num, const Graphics::PixelFormat &format) {
	if (_data[num].format == format) {
		return;
	}
	_data[num].convertToInPlace(format);
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
			} else {
				*result++ = *compressed++;
			}
			++byteIndex;
		} else {
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
				} else {
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
