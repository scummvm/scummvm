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

#ifndef ULTIMA4_GFX_IMAGELOADER_FMTOWNS_H_
#define ULTIMA4_GFX_IMAGELOADER_FMTOWNS_H_

#include "ultima/ultima4/gfx/imageloader.h"

namespace Ultima {
namespace Ultima4 {

class FMTOWNSImageLoader : public ImageLoader {
protected:
	int _offset;
public:
	FMTOWNSImageLoader(int offset) : _offset(offset) {}
	virtual ~FMTOWNSImageLoader() {}

	/**
	 * Loads in an FM TOWNS files, which we assume is 16 bits.
	 */
	Image *load(Common::SeekableReadStream &stream, int width, int height, int bpp) override;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
