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

#ifndef ULTIMA8_GFX_FRAMEID_H
#define ULTIMA8_GFX_FRAMEID_H

#include "common/stream.h"

namespace Ultima {
namespace Ultima8 {

struct FrameID {
	uint16 _flexId;
	uint32 _shapeNum;
	uint32 _frameNum;

	FrameID() : _flexId(0), _shapeNum(0), _frameNum(0) { }
	FrameID(uint16 flex, uint32 shape, uint32 frame)
		: _flexId(flex), _shapeNum(shape), _frameNum(frame) {
	}

	void save(Common::WriteStream *ws);
	bool load(Common::ReadStream *rs);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
