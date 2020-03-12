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

#include "ultima/ultima4/config.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/error.h"
#include "ultima/ultima4/image.h"
#include "ultima/ultima4/imageloader.h"
#include "ultima/ultima4/imageloader_u4.h"
#include "ultima/ultima4/rle.h"
//#include "ultima/ultima4/lzw/u4decode.h"

namespace Ultima {
namespace Ultima4 {

using Std::vector;

ImageLoader *U4RawImageLoader::_instance = ImageLoader::registerLoader(new U4RawImageLoader, "image/x-u4raw");
ImageLoader *U4RleImageLoader::_instance = ImageLoader::registerLoader(new U4RleImageLoader, "image/x-u4rle");
ImageLoader *U4LzwImageLoader::_instance = ImageLoader::registerLoader(new U4LzwImageLoader, "image/x-u4lzw");

RGBA *U4PaletteLoader::_bwPalette = NULL;
RGBA *U4PaletteLoader::_egaPalette = NULL;
RGBA *U4PaletteLoader::_vgaPalette = NULL;

/**
 * Loads in the raw image and apply the standard U4 16 or 256 color
 * palette.
 */
Image *U4RawImageLoader::load(U4FILE *file, int width, int height, int bpp) {
    if (width == -1 || height == -1 || bpp == -1) {
          errorFatal("dimensions not set for u4raw image");
    }

    ASSERT(bpp == 1 || bpp == 4 || bpp == 8 || bpp == 24 || bpp == 32, "invalid bpp: %d", bpp);

    long rawLen = file->length();
    unsigned char *raw = (unsigned char *) malloc(rawLen);
    file->read(raw, 1, rawLen);

    long requiredLength = (width * height * bpp / 8);
    if (rawLen < requiredLength) {
        if (raw)
            free(raw);
        errorWarning("u4Raw Image of size %ld does not fit anticipated size %ld", rawLen, requiredLength);
        return NULL;
    }

    Image *image = Image::create(width, height, bpp <= 8, Image::HARDWARE);
    if (!image) {
        if (raw)
            free(raw);
        return NULL;
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
Image *U4RleImageLoader::load(U4FILE *file, int width, int height, int bpp) {
    if (width == -1 || height == -1 || bpp == -1) {
          errorFatal("dimensions not set for u4rle image");
    }

    ASSERT(bpp == 1 || bpp == 4 || bpp == 8 || bpp == 24 || bpp == 32, "invalid bpp: %d", bpp);

    long compressedLen = file->length();
    unsigned char *compressed = (unsigned char *) malloc(compressedLen);
    file->read(compressed, 1, compressedLen);

    unsigned char *raw = NULL;
    long rawLen = rleDecompressMemory(compressed, compressedLen, (void **) &raw);
    free(compressed);

    if (rawLen != (width * height * bpp / 8)) {
        if (raw)
            free(raw);
        return NULL;
    }

    Image *image = Image::create(width, height, bpp <= 8, Image::HARDWARE);
    if (!image) {
        if (raw)
            free(raw);
        return NULL;
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
Image *U4LzwImageLoader::load(U4FILE *file, int width, int height, int bpp) {
    if (width == -1 || height == -1 || bpp == -1) {
          errorFatal("dimensions not set for u4lzw image");
    }

    ASSERT(bpp == 1 || bpp == 4 || bpp == 8 || bpp == 24 || bpp == 32, "invalid bpp: %d", bpp);

    long compressedLen = file->length();
    unsigned char *compressed = (unsigned char *) malloc(compressedLen);
    file->read(compressed, 1, compressedLen);
#ifdef TODO
    unsigned char *raw = NULL;
    long rawLen = decompress_u4_memory(compressed, compressedLen, (void **) &raw);
    free(compressed);

    if (rawLen != (width * height * bpp / 8)) {
        if (raw)
            free(raw);
        return NULL;
    }

    Image *image = Image::create(width, height, bpp <= 8, Image::HARDWARE);
    if (!image) {
        if (raw)
            free(raw);
        return NULL;
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
#else
	return nullptr;
#endif
}

/**
 * Loads a simple black & white palette
 */
RGBA *U4PaletteLoader::loadBWPalette() {
    if (_bwPalette == NULL) {
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
    if (_egaPalette == NULL) {
        int index = 0;
        const Config *config = Config::getInstance();
        
        _egaPalette = new RGBA[16];

        vector<ConfigElement> paletteConf = config->getElement("egaPalette").getChildren();
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
    if (_vgaPalette == NULL) {
        U4FILE *pal = u4fopen("u4vga.pal");
        if (!pal)
            return NULL;

        _vgaPalette = new RGBA[256];

        for (int i = 0; i < 256; i++) {
            _vgaPalette[i].r = u4fgetc(pal) * 255 / 63;
            _vgaPalette[i].g = u4fgetc(pal) * 255 / 63;
            _vgaPalette[i].b = u4fgetc(pal) * 255 / 63;
        }
        u4fclose(pal);

    }

    return _vgaPalette;
}

} // End of namespace Ultima4
} // End of namespace Ultima
