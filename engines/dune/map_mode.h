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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DUNE_MAP_MODE_H
#define DUNE_MAP_MODE_H

#include "common/scummsys.h"

namespace Dune {

// 5A10 complements the room mode when GIVE ORDERS enters the map.
// Ordinary map entry instead uses 186E's negation below.
inline byte roomOrderViewMode(byte mode) {
	return (byte)~mode;
}

// 186E negates the retained byte; it does not normalize its magnitude.
// Separate open/close owners can also receive an already-correct sign.
inline byte strategicViewMode(byte mode) {
	return (int8)mode > 0 ? (byte)(0 - mode) : mode;
}

inline byte groundViewMode(byte mode) {
	return (int8)mode < 0 ? (byte)(0 - mode) : mode;
}

} // namespace Dune

#endif
