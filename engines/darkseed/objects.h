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

#ifndef DARKSEED_OBJECTS_H
#define DARKSEED_OBJECTS_H

#include "common/array.h"
#include "common/error.h"
#include "common/rect.h"
#include "common/serializer.h"

namespace Darkseed {

enum ObjType {
	OBJ_21_HEADBAND = 21
};

class Objects {
	Common::Array<int16> _objectVar;
	Common::Array<Common::Point> _moveObjectXY;
	Common::Array<uint8> _moveObjectRoom;

public:
	Common::Array<int16> _objectRunningCode;

	static constexpr int MAX_MOVED_OBJECTS = 42;
	static constexpr int MAX_OBJECTS = 199;
	Objects();
	void reset();

	Common::Error sync(Common::Serializer &s);

	void setVar(uint16 varIdx, int16 newValue);
	int16 getVar(uint16 varIdx);

	int16 getObjectRunningCode(int idx);
	void setObjectRunningCode(int idx, int16 value);

	Common::Point getMoveObjectPosition(uint8 objIdx);
	void setMoveObjectPosition(uint8 objIdx, const Common::Point &newPoint);
	void setMoveObjectX(uint8 objIdx, int16 xPos);
	int getEyeDescriptionTosIdx(uint16 objNum);

	int getMoveObjectRoom(uint16 idx);
	void setMoveObjectRoom(uint16 idx, uint8 value);

	const char *getObjectName(int idx);

	int16 &operator[](uint16 varIdx);
	const int16 &operator[](uint16 varIdx) const;
};

} // namespace Darkseed

#endif // DARKSEED_OBJECTS_H
