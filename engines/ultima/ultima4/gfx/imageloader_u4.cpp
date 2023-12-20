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

#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/core/lzw/u4decode.h"
#include "ultima/ultima4/gfx/imageloader_u4.h"
#include "ultima/ultima4/filesys/rle.h"

#include "common/file.h"
#include "graphics/surface.h"

namespace Ultima {
namespace Ultima4 {

const byte U4PaletteLoader::_bwPalette[2 * 3] = {
	0, 0, 0,
	255, 255, 255
};
byte *U4PaletteLoader::_egaPalette = nullptr;
byte *U4PaletteLoader::_vgaPalette = nullptr;

bool U4RawImageDecoder::loadStream(Common::SeekableReadStream &stream) {
	if (_width == -1 || _height == -1 || _bpp == -1) {
		error("dimensions not set for u4raw image");
	}

	assertMsg(_bpp == 1 || _bpp == 4 || _bpp == 8 || _bpp == 24 || _bpp == 32, "invalid bpp: %d", _bpp);

	long rawLen = stream.size();
	byte *raw = (byte *)malloc(rawLen);
	stream.read(raw, rawLen);

	long requiredLength = (_width * _height * _bpp / 8);
	if (rawLen < requiredLength) {
		if (raw)
			free(raw);
		warning("u4Raw Image of size %ld does not fit anticipated size %ld", rawLen, requiredLength);
		return false;
	}

	_surface = new Graphics::Surface();
	_surface->create(_width, _height, getPixelFormatForBpp());

	U4PaletteLoader paletteLoader;
	if (_bpp == 8) {
		_palette = paletteLoader.loadVgaPalette();
		_paletteColorCount = 256;
	} else if (_bpp == 4) {
		_palette = paletteLoader.loadEgaPalette();
		_paletteColorCount = 16;
	} else if (_bpp == 1) {
		_palette = paletteLoader.loadBWPalette();
		_paletteColorCount = 2;
	}

	setFromRawData(raw);

	free(raw);

	return true;
}

/**
 * Loads in the rle-compressed image and apply the standard U4 16 or
 * 256 color palette.
 */
bool U4RleImageDecoder::loadStream(Common::SeekableReadStream &stream) {
	if (_width == -1 || _height == -1 || _bpp == -1) {
		error("dimensions not set for u4rle image");
	}

	assertMsg(_bpp == 1 || _bpp == 4 || _bpp == 8 || _bpp == 24 || _bpp == 32, "invalid bpp: %d", _bpp);

	long compressedLen = stream.size();
	byte *compressed = (byte *) malloc(compressedLen);
	stream.read(compressed, compressedLen);

	byte *raw = nullptr;
	long rawLen = rleDecompressMemory(compressed, compressedLen, (void **) &raw);
	free(compressed);

	if (rawLen != (_width * _height * _bpp / 8)) {
		if (raw)
			free(raw);
		return false;
	}

	_surface = new Graphics::Surface();
	_surface->create(_width, _height, getPixelFormatForBpp());

	U4PaletteLoader paletteLoader;
	if (_bpp == 8) {
		_palette = paletteLoader.loadVgaPalette();
		_paletteColorCount = 256;
	} else if (_bpp == 4) {
		_palette = paletteLoader.loadEgaPalette();
		_paletteColorCount = 16;
	} else if (_bpp == 1) {
		_palette = paletteLoader.loadBWPalette();
		_paletteColorCount = 2;
	}

	setFromRawData(raw);

	free(raw);

	return true;
}

/**
 * Loads in the lzw-compressed image and apply the standard U4 16 or
 * 256 color palette.
 */
bool U4LzwImageDecoder::loadStream(Common::SeekableReadStream &stream) {
	if (_width == -1 || _height == -1 || _bpp == -1) {
		error("dimensions not set for u4lzw image");
	}

	assertMsg(_bpp == 1 || _bpp == 4 || _bpp == 8 || _bpp == 24 || _bpp == 32, "invalid bpp: %d", _bpp);

	long compressedLen = stream.size();
	byte *compressed = (byte *) malloc(compressedLen);
	stream.read(compressed, compressedLen);

	byte *raw = nullptr;
	long rawLen = LZW::decompress_u4_memory(compressed, compressedLen, (void **) &raw);
	free(compressed);

	if (rawLen != (_width * _height * _bpp / 8)) {
		if (raw)
			free(raw);
		return false;
	}

	_surface = new Graphics::Surface();
	_surface->create(_width, _height, getPixelFormatForBpp());

	U4PaletteLoader paletteLoader;
	if (_bpp == 8) {
		_palette = paletteLoader.loadVgaPalette();
		_paletteColorCount = 256;
	} else if (_bpp == 4) {
		_palette = paletteLoader.loadEgaPalette();
		_paletteColorCount = 16;
	} else if (_bpp == 1) {
		_palette = paletteLoader.loadBWPalette();
		_paletteColorCount = 2;
	}

	setFromRawData(raw);

	free(raw);

	return true;
}


/**
 * Loads a simple black & white palette
 */
const byte *U4PaletteLoader::loadBWPalette() {
	return _bwPalette;
}

/**
 * Loads the basic EGA palette from egaPalette.xml
 */
const byte *U4PaletteLoader::loadEgaPalette() {
	if (_egaPalette == nullptr) {
		int index = 0;
		const Config *config = Config::getInstance();

		_egaPalette = new byte[16 * 3];

		Std::vector<ConfigElement> paletteConf = config->getElement("egaPalette").getChildren();
		for (Std::vector<ConfigElement>::iterator i = paletteConf.begin(); i != paletteConf.end(); i++) {

			if (i->getName() != "color")
				continue;

			_egaPalette[index++] = i->getInt("red");
			_egaPalette[index++] = i->getInt("green");
			_egaPalette[index++] = i->getInt("blue");
		}
	}
	return _egaPalette;
}

/**
 * Load the 256 color VGA palette from a file.
 */
const byte *U4PaletteLoader::loadVgaPalette() {
	if (_vgaPalette == nullptr) {
		Common::File pal;
		if (!pal.open("u4vga.pal"))
			return nullptr;

		_vgaPalette = new byte[256 * 3];
		pal.read(_vgaPalette, 256 * 3);

		for (int i = 0; i < 256 * 3; i++) {
			_vgaPalette[i] = _vgaPalette[i] * 255 / 63;
		}
	}

	return _vgaPalette;
}

} // End of namespace Ultima4
} // End of namespace Ultima
