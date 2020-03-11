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

#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/image.h"
#include "ultima/ultima4/imageloader.h"

namespace Ultima {
namespace Ultima4 {

Std::map<Common::String, ImageLoader *> *ImageLoader::loaderMap = NULL;

/**
 * This class method returns the registered concrete subclass
 * appropriate for loading images of a type given by fileType.
 */
ImageLoader *ImageLoader::getLoader(const Common::String &fileType) {
    ASSERT(loaderMap != NULL, "ImageLoader::getLoader loaderMap not initialized");
    if (loaderMap->find(fileType) == loaderMap->end())
        return NULL;
    return (*loaderMap)[fileType];
}

/**
 * Register an image loader.  Concrete subclasses should register an
 * instance at startup.  This method is safe to call from a global
 * object constructor or static initializer.
 */
ImageLoader *ImageLoader::registerLoader(ImageLoader *loader, const Common::String &type) {
    if (loaderMap == NULL) {
        loaderMap = new Std::map<Common::String, ImageLoader *>;
    }
    (*loaderMap)[type] = loader;
    return loader;
}

/**
 * Fill in the image pixel data from an uncompressed string of bytes.
 */
void ImageLoader::setFromRawData(Image *image, int width, int height, int bpp, unsigned char *rawData) {
    int x, y;

    switch (bpp) {

    case 32:
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++)
                image->putPixel(x, y, 
                                rawData[(y * width + x) * 4], 
                                rawData[(y * width + x) * 4 + 1], 
                                rawData[(y * width + x) * 4 + 2],
                                rawData[(y * width + x) * 4 + 3]);
        }
        break;

    case 24:
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++)
                image->putPixel(x, y, 
                                rawData[(y * width + x) * 3], 
                                rawData[(y * width + x) * 3 + 1], 
                                rawData[(y * width + x) * 3 + 2],
                                IM_OPAQUE);
        }
        break;

    case 8:
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++)
                image->putPixelIndex(x, y, rawData[y * width + x]);
        }
        break;

    case 4:
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x+=2) {
                image->putPixelIndex(x, y, rawData[(y * width + x) / 2] >> 4);
                image->putPixelIndex(x + 1, y, rawData[(y * width + x) / 2] & 0x0f);
            }
        }
        break;

    case 1:
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x+=8) {
                image->putPixelIndex(x + 0, y, (rawData[(y * width + x) / 8] >> 7) & 0x01);
                image->putPixelIndex(x + 1, y, (rawData[(y * width + x) / 8] >> 6) & 0x01);
                image->putPixelIndex(x + 2, y, (rawData[(y * width + x) / 8] >> 5) & 0x01);
                image->putPixelIndex(x + 3, y, (rawData[(y * width + x) / 8] >> 4) & 0x01);
                image->putPixelIndex(x + 4, y, (rawData[(y * width + x) / 8] >> 3) & 0x01);
                image->putPixelIndex(x + 5, y, (rawData[(y * width + x) / 8] >> 2) & 0x01);
                image->putPixelIndex(x + 6, y, (rawData[(y * width + x) / 8] >> 1) & 0x01);
                image->putPixelIndex(x + 7, y, (rawData[(y * width + x) / 8] >> 0) & 0x01);
            }
        }
        break;

    default:
        ASSERT(0, "invalid bits-per-pixel (bpp): %d", bpp);
    }
}

} // End of namespace Ultima4
} // End of namespace Ultima
