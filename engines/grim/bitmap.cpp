/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/endian.h"

#include "graphics/colormasks.h"
#include "graphics/pixelbuffer.h"
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
	_colorFormat = 0;
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
//  uint32 redBits = data->readUint32LE();
//  uint32 greenBits = data->readUint32LE();
//  uint32 blueBits = data->readUint32LE();
//  uint32 redShift = data->readUint32LE();
//  uint32 greenShift = data->readUint32LE();
//  uint32 blueShift = data->readUint32LE();

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
				warning(".. when loading image %s.", _fname.c_str());
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
	_texIds = nullptr;

	g_driver->createBitmap(this);
	return true;
}

BitmapData::BitmapData(const Graphics::PixelBuffer &buf, int w, int h, const char *fname) : _fname(fname) {
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
	_bpp = buf.getFormat().bytesPerPixel * 8;
	_hasTransparency = false;
	_colorFormat = BM_RGB565;
	_data = new Graphics::PixelBuffer[_numImages];
	_data[0].create(buf.getFormat(), w * h, DisposeAfterUse::YES);
	_data[0].copyBuffer(0, w * h, buf);
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
		_bpp(0), _colorFormat(0), _texIds(nullptr), _hasTransparency(false), _data(nullptr),
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

	const Graphics::Surface *origSurf = dec.getSurface();
	Graphics::PixelFormat pixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
	Graphics::Surface *surf = origSurf->convertTo(pixelFormat);

	_width = surf->w;
	_height = surf->h;
	_format = 1;
	_x = _y = 0;
	_bpp = 4;
	_colorFormat = BM_RGBA;
	_numImages = 1;
	_data = new Graphics::PixelBuffer[1];
	_data[0].set(pixelFormat, (unsigned char *)surf->getPixels());

	g_driver->createBitmap(this);

	freeData();
	delete surf;

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
	int numSubImages = o->readUint32LE();

	char **data = new char *[numSubImages];

	o->seek(16, SEEK_CUR);
	_bpp = o->readUint32LE();

	o->seek(bmoffset + 128);

	_width = o->readUint32LE();
	_height = o->readUint32LE();
	o->seek(-8, SEEK_CUR);

	int size = 4 * _width * _height;
	for (int i = 0; i < numSubImages; ++i) {
		data[i] = new char[size];
		o->seek(8, SEEK_CUR);
		if (_bpp == 16) {
			uint32 *d = (uint32 *)data[i];
			for (int j = 0; j < _width * _height; ++j) {
				uint16 p = o->readUint16LE();
				// These values are shifted left by 3 so that they saturate the color channel
				uint8 b = (p & 0x7C00) >> 7;
				uint8 g = (p & 0x03E0) >> 2;
				uint8 r = (p & 0x001F) << 3;
				uint8 a = (p & 0x8000) ? 0xFF : 0x00;
				// Recombine the color components into a 32 bit RGB value
				uint32 tmp = (r << 24) | (g << 16) | (b << 8) | a;
				WRITE_BE_UINT32(&d[j], tmp);
			}
		} else if (_bpp == 32) {
			uint32 *d = (uint32 *)data[i];
			for (int j = 0; j < _width * _height; ++j) {
				o->read(&(d[j]), 4);
			}
		}
	}
	_bpp = 32;

	Graphics::PixelFormat pixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
	_colorFormat = BM_RGBA;

	_width = 256;
	_height = 256;
	_numImages = numSubImages;
	_data = new Graphics::PixelBuffer[_numImages];
	for (int i = 0; i < _numImages; ++i) {
		_data[i].create(pixelFormat, _width * _height, DisposeAfterUse::YES);
		_data[i].set(pixelFormat, (byte *)data[i]);
	}

	delete[] data;

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
	_currImage = 0;
}

Bitmap *Bitmap::create(const Common::String &filename) {
	if (!SearchMan.hasFile(filename)) {
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
	return getData(num).getFormat();
}

void BitmapData::convertToColorFormat(int num, const Graphics::PixelFormat &format) {
	if (_data[num].getFormat() == format) {
		return;
	}

	Graphics::PixelBuffer dst(format, _width * _height, DisposeAfterUse::NO);
	dst.copyBuffer(0, _width * _height, _data[num]);
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
