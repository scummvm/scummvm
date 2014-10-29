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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ILLUSIONS_GRAPHICS_H
#define ILLUSIONS_GRAPHICS_H

#include "common/rect.h"
#include "common/stream.h"

namespace Illusions {

struct WidthHeight {
	int16 _width, _height;
	void load(Common::SeekableReadStream &stream);
};

struct SurfInfo {
	uint32 _pixelSize;
	WidthHeight _dimensions;
	void load(Common::SeekableReadStream &stream);
};

void loadPoint(Common::SeekableReadStream &stream, Common::Point &pt);

} // End of namespace Illusions

#endif // ILLUSIONS_GRAPHICS_H
