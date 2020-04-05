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
#include "ultima/ultima4/core/error.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/gfx/image.h"
#include "ultima/ultima4/gfx/imageloader.h"
#include "ultima/ultima4/gfx/imageloader_u4.h"
#include "ultima/ultima4/gfx/imageloader_u5.h"

namespace Ultima {
namespace Ultima4 {

using Std::vector;

Image *U5LzwImageLoader::load(U4FILE *file, int width, int height, int bpp) {
	if (width == -1 || height == -1 || bpp == -1) {
		errorFatal("dimensions not set for u5lzw image");
	}

	ASSERT(bpp == 4 || bpp == 8 || bpp == 24 || bpp == 32, "invalid bpp: %d", bpp);

#ifdef TODO
	long compressedLen = file->length();
	unsigned char *compressed = new unsigned char[compressedLen];
	file->read(compressed, 1, compressedLen);

	long rawLen = compressed[0] + (compressed[1] << 8) + (compressed[2] << 16) + (compressed[3] << 24);
	unsigned char *raw = new unsigned char[rawLen];

	U6Decode::lzw_decompress(compressed + 4, compressedLen - 4, raw, rawLen);
	delete [] compressed;

	if (rawLen != (width * height * bpp / 8)) {
		if (raw)
			delete [] raw;
		return NULL;
	}

	Image *image = Image::create(width, height, bpp == 4 || bpp == 8, Image::HARDWARE);
	if (!image) {
		if (raw)
			delete [] raw;
		return NULL;
	}

	U4PaletteLoader paletteLoader;
	if (bpp == 8)
		image->setPalette(paletteLoader.loadVgaPalette(), 256);
	else if (bpp == 4)
		image->setPalette(paletteLoader.loadEgaPalette(), 16);

	setFromRawData(image, width, height, bpp, raw);

	delete [] raw;

	return image;
#else
	return nullptr;
#endif
}

} // End of namespace Ultima4
} // End of namespace Ultima
