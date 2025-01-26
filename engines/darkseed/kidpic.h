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

#ifndef DARKSEED_KIDPIC_H
#define DARKSEED_KIDPIC_H

#include "common/array.h"
#include "common/scummsys.h"
#include "common/file.h"

namespace Darkseed {

class KidPic {
	Common::Array<uint8> _pixels;
	int _lineNum = 0;
	int _lineByteIdx = 0;

public:
	KidPic();
	void draw();

private:
	bool unpackRLE(Common::SeekableReadStream &readStream);
	void unpackByte(uint8 byte);
};

} // namespace Darkseed

#endif // DARKSEED_KIDPIC_H
