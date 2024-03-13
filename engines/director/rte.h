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

#ifndef DIRECTOR_RTE_H
#define DIRECTOR_RTE_H

#include "common/array.h"
#include "common/stream.h"
namespace Common {
class SeekableReadStreamEndian;
}

namespace Director {

class Cast;

class RTE0 {
public:
	RTE0(Cast *cast, Common::SeekableReadStreamEndian &stream);

	Cast *_cast;
	Common::Array<byte> data;
};

class RTE1 {
public:
	RTE1(Cast *cast, Common::SeekableReadStreamEndian &stream);

	Cast *_cast;
	Common::Array<byte> data;
};

class RTE2 {
public:
	RTE2(Cast *cast, Common::SeekableReadStreamEndian &stream);

	Cast *_cast;
	uint16 width;
	uint16 height;
	uint32 bpp;
	Common::Array<byte> alphaMap;
};

} // End of namespace Director

#endif
