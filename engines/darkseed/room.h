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

#ifndef DARKSEED_ROOM_H
#define DARKSEED_ROOM_H

#include "pal.h"
#include "pic.h"
#include "common/rect.h"

namespace Darkseed {

struct RoomStruct1 {
	uint16 x = 0;
	uint16 y = 0;
	uint16 unk4 = 0;
	uint16 unk6 = 0;
	uint16 roomNumber = 0;
	uint8 unka = 0;
};

struct RoomStruct2 {
	uint8 strip[40];
};

struct RoomStruct3 {
	uint16 unk0 = 0;
	uint16 unk2 = 0;
	uint16 xOffset = 0;
	uint16 yOffset = 0;
	uint16 width = 0;
	uint16 height = 0;
	uint8 unkc = 0;
	uint8 unkd = 0;
};

struct RoomConnector {
	uint16 x = 0;
	uint16 y = 0;
};

class Room {
public:
	int _roomNumber;
	Pic pic;
	Pal _pal;

	Common::Array<RoomStruct1> room1;
	Common::Array<RoomStruct2> walkableLocationsMap;
	Common::Array<RoomStruct3> room3;
	Common::Array<RoomConnector> connectors;

	uint16 selectedObjIndex = 0;
public:
	explicit Room(int roomNumber);

	void draw();

	void update();

	int getObjectAtPoint(int x, int y);
	int getObjectNumUnder6AtCursor();
	void getWalkTargetForObjectType_maybe(int objId);
	int getExitRoomNumberAtPoint(int x, int y);
	bool exitRoom();
	Common::String getRoomFilenameBase(int roomNumber);
	bool canWalkAtLocation(int x, int y);
	void printRoomDescriptionText() const;
private:
	bool load();
	static Common::String stripSpaces(Common::String source);
};

} // namespace Darkseed

#endif // DARKSEED_ROOM_H
