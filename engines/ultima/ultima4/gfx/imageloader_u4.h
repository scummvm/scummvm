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

#ifndef ULTIMA4_GFX_IMAGELOADER_U4_H
#define ULTIMA4_GFX_IMAGELOADER_U4_H

#include "ultima/ultima4/gfx/imageloader.h"

namespace Ultima {
namespace Ultima4 {

struct RGBA;

/**
 * Loader for U4 raw images.  Raw images are just an uncompressed
 * stream of pixel data with no palette information (e.g. shapes.ega,
 * charset.ega).  This loader handles the original 4-bit images, as
 * well as the 8-bit VGA upgrade images.
 */
class U4RawImageLoader : public ImageLoader {
public:
	/**
	 * Loads in the raw image and apply the standard U4 16 or 256 color palette.
	 */
	Image *load(Common::SeekableReadStream &stream, int width, int height, int bpp) override;
};

/**
 * Loader for U4 images with RLE compression.  Like raw images, the
 * data is just a stream of pixel data with no palette information
 * (e.g. start.ega, rune_*.ega).  This loader handles the original
 * 4-bit images, as well as the 8-bit VGA upgrade images.
 */
class U4RleImageLoader : public ImageLoader {
public:
	Image *load(Common::SeekableReadStream &stream, int width, int height, int bpp) override;
};

/**
 * Loader for U4 images with LZW compression.  Like raw images, the
 * data is just a stream of pixel data with no palette information
 * (e.g. title.ega, tree.ega).  This loader handles the original 4-bit
 * images, as well as the 8-bit VGA upgrade images.
 */
class U4LzwImageLoader : public ImageLoader {
public:
	Image *load(Common::SeekableReadStream &stream, int width, int height, int bpp) override;
};

class U4PaletteLoader {
	static RGBA *_bwPalette;
	static RGBA *_egaPalette;
	static RGBA *_vgaPalette;

public:
	RGBA *loadBWPalette();
	RGBA *loadEgaPalette();
	RGBA *loadVgaPalette();
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
