
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

#ifndef BAGEL_BAGLIB_PAINT_TABLE_H
#define BAGEL_BAGLIB_PAINT_TABLE_H

#include "common/stream.h"

namespace Bagel {
namespace SpaceBar {

struct stripEnds {
	int16 _top;
	int16 _bottom;
};

extern stripEnds STRIP_POINTS[153][120];

class PaintTable {
public:
	/**
	 * Loads the data used for the pseudo-3d projection of panning windows
	 */
	static void initialize(Common::SeekableReadStream &src);
};

} // namespace SpaceBar
} // namespace Bagel

#endif
