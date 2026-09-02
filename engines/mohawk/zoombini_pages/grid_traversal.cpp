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

#include "mohawk/zoombini_pages/grid_traversal.h"

namespace Mohawk {

ZmbGridStep ZmbGridTraversal::computeStep(const ZmbGridCell &source, ZmbGridDirection direction,
										  const ZmbGridBounds &bounds) {
	ZmbGridCell destination = source;

	switch (direction) {
	case kZmbGridUp:
		destination.row -= 1;
		break;
	case kZmbGridRight:
		destination.col += 1;
		break;
	case kZmbGridDown:
		destination.row += 1;
		break;
	case kZmbGridLeft:
		destination.col -= 1;
		break;
	default:
		return ZmbGridStep(source, false);
	}

	return ZmbGridStep(destination, bounds.contains(destination));
}

bool ZmbGridTraversal::advanceCell(ZmbGridCell &cell, ZmbGridDirection direction,
								   const ZmbGridBounds &bounds) {
	const ZmbGridStep step = computeStep(cell, direction, bounds);
	if (!step.inBounds)
		return false;

	cell = step.cell;
	return true;
}

ZmbGridDirection ZmbGridTraversal::oppositeDirection(ZmbGridDirection direction) {
	switch (direction) {
	case kZmbGridUp:
		return kZmbGridDown;
	case kZmbGridRight:
		return kZmbGridLeft;
	case kZmbGridDown:
		return kZmbGridUp;
	case kZmbGridLeft:
		return kZmbGridRight;
	default:
		return direction;
	}
}

} // End of namespace Mohawk
