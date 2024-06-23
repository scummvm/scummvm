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
namespace Darkseed {

class Objects {
private:
	Common::Array<int16> _objectVar;
	Common::Array<int16> _objectRunningCode;
	Common::Array<uint16> _moveObjectX;
	Common::Array<uint16> _moveObjectY;
	Common::Array<uint8> _moveObjectRoom;

public:
	Objects();
	void reset();

	void setVar(uint16 varIdx, int16 newValue);
	int16 getVar(uint16 varIdx);
};

} // namespace Darkseed

#endif // DARKSEED_OBJECTS_H
