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

#ifndef ULTIMA4_GFX_IMAGELOADER_U4_H
#define ULTIMA4_GFX_IMAGELOADER_U4_H

#include "ultima/ultima4/gfx/imageloader.h"

namespace Ultima {
namespace Ultima4 {

/**
 * Decoder for U4 raw images.  Raw images are just an uncompressed
 * stream of pixel data with no palette information (e.g. shapes.ega,
 * charset.ega).  This decoder handles the original 4-bit images, as
 * well as the 8-bit VGA upgrade images.
 */
class U4RawImageDecoder : public U4ImageDecoder {
public:
	U4RawImageDecoder(int width, int height, int bpp)
	  : U4ImageDecoder(width, height, bpp) {}

	bool loadStream(Common::SeekableReadStream &stream) override;
};

/**
 * Decoder for U4 images with RLE compression.  Like raw images, the
 * data is just a stream of pixel data with no palette information
 * (e.g. start.ega, rune_*.ega).  This decoder handles the original
 * 4-bit images, as well as the 8-bit VGA upgrade images.
 */
class U4RleImageDecoder : public U4ImageDecoder {
public:
	U4RleImageDecoder(int width, int height, int bpp)
	  : U4ImageDecoder(width, height, bpp) {}

	bool loadStream(Common::SeekableReadStream &stream) override;
};

/**
 * Decoder for U4 images with LZW compression.  Like raw images, the
 * data is just a stream of pixel data with no palette information
 * (e.g. title.ega, tree.ega).  This decoder handles the original 4-bit
 * images, as well as the 8-bit VGA upgrade images.
 */
class U4LzwImageDecoder : public U4ImageDecoder {
public:
	U4LzwImageDecoder(int width, int height, int bpp)
	  : U4ImageDecoder(width, height, bpp) {}

	bool loadStream(Common::SeekableReadStream &stream) override;
};

class U4PaletteLoader {
	static const byte _bwPalette[2 * 3];
	static byte *_egaPalette;
	static byte *_vgaPalette;

public:
	const byte *loadBWPalette();
	const byte *loadEgaPalette();
	const byte *loadVgaPalette();
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
