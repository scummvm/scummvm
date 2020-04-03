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

#ifndef ULTIMA4_IMAGELOADER_U5_H
#define ULTIMA4_IMAGELOADER_U5_H

#include "ultima/ultima4/gfx/imageloader.h"

namespace Ultima {
namespace Ultima4 {

/**
 * Loader for U5 images with LZW compression.  Similar to U4 LZW
 * images, but with a slightly different variation on the LZW
 * algorithm.
 */
class U5LzwImageLoader : public ImageLoader {
public:
	virtual ~U5LzwImageLoader() {}

	/**
	 * Loads in the lzw-compressed image and apply the standard U4 16 or
	 * 256 color palette.
	 */
	virtual Image *load(U4FILE *file, int width, int height, int bpp);
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
