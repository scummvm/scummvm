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

#include "common/rect.h"
#include "darkseed/pal.h"
#include "darkseed/pic.h"
#include "darkseed/nsp.h"

namespace Darkseed {

struct RoomExit {
	uint16 x = 0;
	uint16 y = 0;
	uint16 width = 0;
	uint16 height = 0;
	uint16 roomNumber = 0;
	uint8 direction = 0;
};

struct RoomStruct2 {
	uint8 strip[40];
};

struct RoomObjElement {
	uint16 type = 0;
	uint16 objNum = 0;
	uint16 xOffset = 0;
	uint16 yOffset = 0;
	uint16 width = 0;
	uint16 height = 0;
	uint8 depth = 0;
	uint8 spriteNum = 0;
};

class Room {
private:
	bool _palLoaded = false;
public:
	static constexpr int MAX_CONNECTORS = 12;
	uint8 _roomNumber;
	Pic _pic;
	Pal _pal;
	Nsp _locationSprites;
	Common::Array<int16> _locObjFrame;
	Common::Array<int16> _locObjFrameTimer;

	Common::Array<RoomExit> _room1;
	Common::Array<RoomStruct2> _walkableLocationsMap;
	Common::Array<RoomObjElement> _roomObj;
	Common::Array<Common::Point> _connectors;

	uint16 _selectedObjIndex = 0;
	int16 _collisionType = 0;

public:
	explicit Room(int roomNumber);

	void initRoom();
	void draw();

	void update();

	int checkCursorAndMoveableObjects();
	int checkCursorAndStaticObjects(int x, int y);
	int CheckCursorAndMovedObjects();
	int getRoomExitAtCursor();
	void getWalkTargetForObjectType_maybe(int objId);
	int getObjectUnderCursor();
	uint16 getDoorTargetRoom(int objId);
	int getExitRoomNumberAtPoint(int x, int y);
	bool exitRoom();
	Common::String getRoomFilenameBase(int roomNumber);
	bool canWalkAtLocation(int x, int y);
	bool canWalkInLineToTarget(int x, int y, int targetX, int targetY);
	void printRoomDescriptionText() const;
	void calculateScaledSpriteDimensions(int width, int height, int curYPosition);
	bool isOutside();
	bool isGiger();
	void runRoomObjects();
	void removeObjectFromRoom(int16 objNum);
	void updateRoomObj(int16 objNum, int16 x, int16 width, int16 y, int16 height);
	bool advanceFrame(int animIdx);
	void mikeStickThrowAnim();
	void loadRoom61AWalkableLocations();
	void restorePalette();
	void darkenSky();
	void loadLocationSprites(const Common::Path &path);
	Common::Point getExitPointForRoom(uint8 roomNumber);
private:
	bool load();
	static Common::String stripSpaces(Common::String source);
	void drawTrunk();
	void advanceLocAnimFrame(int roomObjIdx);
};

} // namespace Darkseed

#endif // DARKSEED_ROOM_H
