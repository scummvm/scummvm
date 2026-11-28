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

#ifndef TESORO_CORE_STREAMUTIL_H
#define TESORO_CORE_STREAMUTIL_H

#include "common/stream.h"

namespace Tesoro {

/** Reads exactly size bytes; true for a size of 0 and when the whole read succeeded. */
inline bool readExactly(Common::SeekableReadStream &s, byte *buf, uint32 size) {
	return size == 0 || s.read(buf, size) == size;
}

} // End of namespace Tesoro

#endif
