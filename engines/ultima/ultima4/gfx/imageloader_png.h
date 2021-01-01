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

#ifndef ULTIMA4_GFX_IMAGELOADER_PNG_H
#define ULTIMA4_GFX_IMAGELOADER_PNG_H

#include "ultima/ultima4/gfx/imageloader.h"

namespace Ultima {
namespace Ultima4 {

/**
 * Loader for PNG images.  All PNG images should be supported: indexed
 * images with palette, or true color, with or without an alpha
 * channel.
 */
class PngImageLoader : public ImageLoader {
public:
	virtual ~PngImageLoader() {}

	/**
	 * Loads in the PNG with the libpng library.
	 */
	Image *load(Common::SeekableReadStream &stream, int width, int height, int bpp) override;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
