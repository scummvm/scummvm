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

#ifndef MEDIASTATION_MINIGAMES_BARBIEMAZE_H
#define MEDIASTATION_MINIGAMES_BARBIEMAZE_H

#include "common/array.h"
#include "common/rect.h"

#include "mediastation/mediascript/scriptvalue.h"

namespace MediaStation {

class Collection;

namespace BarbieMazeMinigame {

void solve(const Common::Array<ScriptValue> &args, ScriptValue &returnValue);
Common::Array<byte> doCreateMaze(uint16 width, uint16 height);
Common::Array<byte> doMaskMaze(uint16 width, uint16 height, Common::Array<byte> &mazeBitmap);

void findShortestPath(
	const Collection &maskData, Common::Point mazeDimensions,
	uint startIndex, uint endIndex,
	Collection &pathResult, Common::Array<uint> &visitedPositions);

} // End of namespace BarbieMazeMinigame

} // End of namespace MediaStation

#endif
