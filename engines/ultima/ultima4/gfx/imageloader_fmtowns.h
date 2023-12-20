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

#ifndef ULTIMA4_GFX_IMAGELOADER_FMTOWNS_H_
#define ULTIMA4_GFX_IMAGELOADER_FMTOWNS_H_

#include "ultima/ultima4/gfx/imageloader.h"

namespace Ultima {
namespace Ultima4 {

class FMTOWNSImageDecoder : public U4ImageDecoder {
protected:
	int _offset;
public:
	FMTOWNSImageDecoder(int width, int height, int bpp, int offset)
	  : U4ImageDecoder(width, height, bpp), _offset(offset) {}

	bool loadStream(Common::SeekableReadStream &stream) override;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
