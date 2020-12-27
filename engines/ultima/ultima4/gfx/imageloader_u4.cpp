/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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

#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/core/lzw/u4decode.h"
#include "ultima/ultima4/gfx/image.h"
#include "ultima/ultima4/gfx/imageloader.h"
#include "ultima/ultima4/gfx/imageloader_u4.h"
#include "ultima/ultima4/filesys/rle.h"
#include "common/file.h"

namespace Ultima {
namespace Ultima4 {

RGBA *U4PaletteLoader::_bwPalette = nullptr;
RGBA *U4PaletteLoader::_egaPalette = nullptr;
RGBA *U4PaletteLoader::_vgaPalette = nullptr;

Image *U4RawImageLoader::load(Common::SeekableReadStream &stream, int width, int height, int bpp) {
	if (width == -1 || height == -1 || bpp == -1) {
		error("dimensions not set for u4raw image");
	}

	assertMsg(bpp == 1 || bpp == 4 || bpp == 8 || bpp == 24 || bpp == 32, "invalid bpp: %d", bpp);

	long rawLen = stream.size();
	byte *raw = (byte *)malloc(rawLen);
	stream.read(raw, rawLen);

	long requiredLength = (width * height * bpp / 8);
	if (rawLen < requiredLength) {
		if (raw)
			free(raw);
		warning("u4Raw Image of size %ld does not fit anticipated size %ld", rawLen, requiredLength);
		return nullptr;
	}

	Image *image = Image::create(width, height, bpp <= 8, Image::HARDWARE);
	if (!image) {
		if (raw)
			free(raw);
		return nullptr;
	}

	U4PaletteLoader paletteLoader;
	if (bpp == 8)
		image->setPalette(paletteLoader.loadVgaPalette(), 256);
	else if (bpp == 4)
		image->setPalette(paletteLoader.loadEgaPalette(), 16);
	else if (bpp == 1)
		image->setPalette(paletteLoader.loadBWPalette(), 2);

	setFromRawData(image, width, height, bpp, raw);

	free(raw);

	return image;
}

/**
 * Loads in the rle-compressed image and apply the standard U4 16 or
 * 256 color palette.
 */
Image *U4RleImageLoader::load(Common::SeekableReadStream &stream, int width, int height, int bpp) {
	if (width == -1 || height == -1 || bpp == -1) {
		error("dimensions not set for u4rle image");
	}

	assertMsg(bpp == 1 || bpp == 4 || bpp == 8 || bpp == 24 || bpp == 32, "invalid bpp: %d", bpp);

	long compressedLen = stream.size();
	byte *compressed = (byte *) malloc(compressedLen);
	stream.read(compressed, compressedLen);

	byte *raw = nullptr;
	long rawLen = rleDecompressMemory(compressed, compressedLen, (void **) &raw);
	free(compressed);

	if (rawLen != (width * height * bpp / 8)) {
		if (raw)
			free(raw);
		return nullptr;
	}

	Image *image = Image::create(width, height, bpp <= 8, Image::HARDWARE);
	if (!image) {
		if (raw)
			free(raw);
		return nullptr;
	}

	U4PaletteLoader paletteLoader;
	if (bpp == 8)
		image->setPalette(paletteLoader.loadVgaPalette(), 256);
	else if (bpp == 4)
		image->setPalette(paletteLoader.loadEgaPalette(), 16);
	else if (bpp == 1)
		image->setPalette(paletteLoader.loadBWPalette(), 2);

	setFromRawData(image, width, height, bpp, raw);

	free(raw);

	return image;
}

/**
 * Loads in the lzw-compressed image and apply the standard U4 16 or
 * 256 color palette.
 */
Image *U4LzwImageLoader::load(Common::SeekableReadStream &stream, int width, int height, int bpp) {
	if (width == -1 || height == -1 || bpp == -1) {
		error("dimensions not set for u4lzw image");
	}

	assertMsg(bpp == 1 || bpp == 4 || bpp == 8 || bpp == 24 || bpp == 32, "invalid bpp: %d", bpp);

	long compressedLen = stream.size();
	byte *compressed = (byte *) malloc(compressedLen);
	stream.read(compressed, compressedLen);

	byte *raw = nullptr;
	long rawLen = LZW::decompress_u4_memory(compressed, compressedLen, (void **) &raw);
	free(compressed);

	if (rawLen != (width * height * bpp / 8)) {
		if (raw)
			free(raw);
		return nullptr;
	}

	Image *image = Image::create(width, height, bpp <= 8, Image::HARDWARE);
	if (!image) {
		if (raw)
			free(raw);
		return nullptr;
	}

	U4PaletteLoader paletteLoader;
	if (bpp == 8)
		image->setPalette(paletteLoader.loadVgaPalette(), 256);
	else if (bpp == 4)
		image->setPalette(paletteLoader.loadEgaPalette(), 16);
	else if (bpp == 1)
		image->setPalette(paletteLoader.loadBWPalette(), 2);

	setFromRawData(image, width, height, bpp, raw);

	free(raw);

	return image;
}

/**
 * Loads a simple black & white palette
 */
RGBA *U4PaletteLoader::loadBWPalette() {
	if (_bwPalette == nullptr) {
		_bwPalette = new RGBA[2];

		_bwPalette[0].r = 0;
		_bwPalette[0].g = 0;
		_bwPalette[0].b = 0;

		_bwPalette[1].r = 255;
		_bwPalette[1].g = 255;
		_bwPalette[1].b = 255;

	}
	return _bwPalette;
}

/**
 * Loads the basic EGA palette from egaPalette.xml
 */
RGBA *U4PaletteLoader::loadEgaPalette() {
	if (_egaPalette == nullptr) {
		int index = 0;
		const Config *config = Config::getInstance();

		_egaPalette = new RGBA[16];

		Std::vector<ConfigElement> paletteConf = config->getElement("egaPalette").getChildren();
		for (Std::vector<ConfigElement>::iterator i = paletteConf.begin(); i != paletteConf.end(); i++) {

			if (i->getName() != "color")
				continue;

			_egaPalette[index].r = i->getInt("red");
			_egaPalette[index].g = i->getInt("green");
			_egaPalette[index].b = i->getInt("blue");

			index++;
		}
	}
	return _egaPalette;
}

/**
 * Load the 256 color VGA palette from a file.
 */
RGBA *U4PaletteLoader::loadVgaPalette() {
	if (_vgaPalette == nullptr) {
		Common::File pal;
		if (!pal.open("u4vga.pal"))
			return nullptr;

		_vgaPalette = new RGBA[256];

		for (int i = 0; i < 256; i++) {
			_vgaPalette[i].r = pal.readByte() * 255 / 63;
			_vgaPalette[i].g = pal.readByte() * 255 / 63;
			_vgaPalette[i].b = pal.readByte() * 255 / 63;
		}
	}

	return _vgaPalette;
}

} // End of namespace Ultima4
} // End of namespace Ultima
