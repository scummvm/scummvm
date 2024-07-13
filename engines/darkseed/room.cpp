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

#include "room.h"
#include "darkseed.h"

const static int roomDescriptionTextTbl[] = {
	0, 138, 165, 165,
	181, 243, 254, 292,
	369, 397, 420, 429,
	438, 447, 463, 473,
	502, 520, 546, 564,
	564, 564, 564, 0,
	569, 574, 574, 574,
	563, 0, 748, 739,
	738, 723, 704, 612,
	619, 0, 758, 0,
	770, 777, 779, 790,
	778, 801, 809, 0,
	810, 811, 814, 821,
	0, 828, 810, 810,
	850, 860, 867, 874,
	886, 651, 682, 701,
	702, 701, 898, 899,
	903};

Darkseed::Room::Room(int roomNumber) : _roomNumber(roomNumber) {
	room1.resize(8);
	walkableLocationsMap.resize(16);
	_roomObj.resize(30);

	if(!load()) {
		error("Failed to load room %d", roomNumber);
	}
}

bool Darkseed::Room::load() {
	Common::String filenameBase = getRoomFilenameBase(_roomNumber);
	Common::String romFilename;
	Common::File file;
	romFilename = g_engine->getRoomFilePath(Common::String::format("%s.rom", filenameBase.c_str(), _roomNumber));
	if(!file.open(romFilename)) {
		return false;
	}

	Common::String nspFilename = stripSpaces(file.readString());
	file.seek(0xd);
	Common::String picFilename = stripSpaces(file.readString());
	file.seek(0x1a);
	Common::String palFilename = stripSpaces(file.readString());
	file.seek(0x27);

	for (int i = 0; i < 8; i++) {
		room1[i].x = file.readUint16BE();
		room1[i].y = file.readUint16BE();
		if (room1[i].y > 233) {
			room1[i].y = 233;
		}
		room1[i].width = file.readUint16BE();
		room1[i].height = file.readUint16BE();
		room1[i].roomNumber = file.readUint16BE();
		room1[i].direction = file.readByte();
	}

	for (int i = 0; i < 16; i++) {
		file.read(walkableLocationsMap[i].strip, 40);
	}

	for (int i = 0; i < 30; i++) {
		_roomObj[i].type = file.readUint16BE();
		_roomObj[i].objNum = file.readUint16BE();
		_roomObj[i].xOffset = file.readUint16BE();
		_roomObj[i].yOffset = file.readUint16BE();
		_roomObj[i].width = file.readUint16BE();
		_roomObj[i].height = file.readUint16BE();
		_roomObj[i].depth = file.readByte();
		_roomObj[i].spriteNum = file.readByte();

		if (_roomObj[i].spriteNum >= 0x29 && _roomObj[i].type != 0 && _roomObj[i].type != 1000) {
			_roomObj[i].height = 0x14;
			_roomObj[i].width = 0x14;
			_roomObj[i].type = 0;
		}

		if (_roomObj[i].objNum == 0 && _roomObj[i].type == 1) {
			if (_connectors.size() == MAX_CONNECTORS) {
				error("Too many connectors in this room, max of %d", MAX_CONNECTORS);
			}
			Common::Point connector;
			connector.x = _roomObj[i].xOffset;
			connector.y = _roomObj[i].yOffset;

			if (connector.x > 565) {
				connector.x = 565;
			}
			if (connector.x < 75) {
				connector.x = 75;
			}
			if (connector.y > 235) {
				connector.y = 235;
			}
			if (connector.y < 45) {
				connector.y = 45;
			}

			debug("Room Connector: %d %d", connector.x, connector.y);
			_connectors.push_back(connector);
			_roomObj[i].type = 0xff;
		}
	}

	file.close();

	_collisionType = 0;
	if(!pic.load(picFilename)) {
		return false;
	}

	_pal.load(g_engine->getPictureFilePath(Common::String::format("%s.pal", filenameBase.c_str())));

	_locationSprites.load(Common::String::format("%s.nsp", filenameBase.c_str()));

	return true;
}

Common::String Darkseed::Room::stripSpaces(Common::String source) {
	Common::String out;
	const char *src = source.c_str();
	for (int i = 0; i < source.size(); i++) {
		if (src[i] != ' ') {
			out += src[i];
		}
	}
	return out;
}

void Darkseed::Room::draw() {
	pic.draw(0x45, 0x28);

	// print walkable area map.
	if (g_engine->_debugShowWalkPath) {
		for (int y = 0x28; y < pic.getHeight() + 0x28; y++) {
			for (int x = 0x45; x < pic.getWidth() + 0x45; x++) {
				if (canWalkAtLocation(x, y)) {
					g_engine->_screen->drawLine(x, y, x, y, 14);
				}
			}
		}

		for (int i = 0; i < _connectors.size(); i++) {
			g_engine->_baseSprites.getSpriteAt(0).draw(_connectors[i].x, _connectors[i].y);
		}
	}
}

int Darkseed::Room::checkCursorAndMoveableObjects() {
	ActionMode actionMode = g_engine->_actionMode;
	const Sprite &cursorSprite = (actionMode == LookAction)
									 ? g_engine->_cursor.getSpriteForType(ExclamationMark)
									 : g_engine->_cursor.getSprite();
	bool hasObject = false;
	int objNum = -1;
	_collisionType = 0;
	for (int i = 0; i < _roomObj.size(); i++) {
		if ((_roomObj[i].type == 1 || _roomObj[i].type == 3)
			&& _roomObj[i].xOffset <= cursorSprite.width + g_engine->_cursor.getX()
			&& g_engine->_cursor.getX() <= _roomObj[i].width + _roomObj[i].xOffset
			&& _roomObj[i].yOffset <= cursorSprite.height + g_engine->_cursor.getY()
			&& g_engine->_cursor.getY() <= _roomObj[i].height + _roomObj[i].yOffset
		) {
			if (_roomObj[i].objNum == 25) {
				if (g_engine->_objectVar.getVar(80) < 3) {
					hasObject = false;
				} else {
					hasObject = true;
				}
			}

			if (_roomObj[i].objNum == 14 && g_engine->_cursor.getY() > 40 && g_engine->_objectVar.getVar(86) == 0) {
				hasObject = false;
			}

			if (hasObject) {
				objNum = i;
			}
		}
	}
	return hasObject ? objNum : -1;
}

int Darkseed::Room::checkCursorAndStaticObjects(int x, int y) {
	ActionMode actionMode = g_engine->_actionMode;
	const Sprite &cursorSprite = (actionMode == LookAction)
									 ? g_engine->_cursor.getSpriteForType(ExclamationMark)
									 : g_engine->_cursor.getSprite();
	bool hasObject = false;
	_collisionType = 0;
	for (int i = 0; i < _roomObj.size(); i++) {
		if (_roomObj[i].type == 0
			&& _roomObj[i].xOffset <= cursorSprite.width + g_engine->_cursor.getX()
			&& g_engine->_cursor.getX() <= _roomObj[i].width + _roomObj[i].xOffset
			&& _roomObj[i].yOffset <= cursorSprite.height + g_engine->_cursor.getY()
			&& g_engine->_cursor.getY() <= _roomObj[i].height + _roomObj[i].yOffset
		) {
			if (actionMode != PointerAction && _roomObj[i].objNum >= 5) {
				hasObject = true;
			}

			if (actionMode == PointerAction && _roomObj[i].objNum < 6) {
				hasObject = true;
			}

			if (_roomObj[i].objNum == 59 || _roomObj[i].objNum == 78) {
				if (g_engine->_objectVar.getVar(34) == 1) {
					hasObject = true;
				} else {
					hasObject = false;
				}
			}

			if (_roomObj[i].objNum == 0x19 && hasObject) {
				if (g_engine->_objectVar.getVar(80) < 2) {
					hasObject = false;
				} else {
					hasObject = true;
				}
			}

			if (_roomObj[i].objNum == 0x74 && hasObject && (int)actionMode != 0x13) {
				hasObject = false;
			}

			if (hasObject) {
				return i;
			}
		}
	}
	return -1;
}

int Darkseed::Room::CheckCursorAndMovedObjects() {
	ActionMode actionMode = g_engine->_actionMode;
	const Sprite &cursorSprite = (actionMode == LookAction)
									 ? g_engine->_cursor.getSpriteForType(ExclamationMark)
									 : g_engine->_cursor.getSprite();
	_collisionType = 1;
	for (int i = 0; i < Objects::MAX_MOVED_OBJECTS; i++) {
		if (g_engine->_objectVar.getMoveObjectRoom(i) == _roomNumber) {
			Common::Point movedObjPos = g_engine->_objectVar.getMoveObjectPosition(i);
			int16 spriteWidth = 0;
			int16 spriteHeight = 0;
			if (i == 22) {
				uint8 spriteIdx = g_engine->_objectVar.getVar(5) != 0 ? 1 : 0;
				const Sprite &sprite = _locationSprites.getSpriteAt(spriteIdx);
				spriteWidth = sprite.width;
				spriteHeight = sprite.height;
			} else {
				const Sprite &sprite = g_engine->_baseSprites.getSpriteAt(i);
				spriteWidth = sprite.width;
				spriteHeight = sprite.height;
			}
			calculateScaledSpriteDimensions(spriteWidth, spriteHeight, movedObjPos.y);

			if (
				((spriteWidth / 2 + movedObjPos.x) - g_engine->scaledSpriteWidth / 2 <= cursorSprite.width + g_engine->_cursor.getX()) &&
				(g_engine->_cursor.getX() <= ((spriteWidth / 2 + movedObjPos.x) - g_engine->scaledSpriteWidth / 2) + g_engine->scaledSpriteWidth) &&
				((movedObjPos.y + spriteHeight) - g_engine->scaledSpriteHeight <= cursorSprite.height + g_engine->_cursor.getY()) &&
				g_engine->_cursor.getY() <= movedObjPos.y + spriteHeight) {
				return i;
			}
		}
	}
	return -1;
}

void Darkseed::Room::update() {
	if (g_engine->_actionMode == HandAction) {
		int moveableObj = checkCursorAndMoveableObjects();
		if (moveableObj == -1) {
			int objectUnderCursor = checkCursorAndStaticObjects(g_engine->_cursor.getX(), g_engine->_cursor.getY());
			if (objectUnderCursor == -1) {
				int movedObject = CheckCursorAndMovedObjects();
				if (movedObject == -1) {
					g_engine->_cursor.setCursorType(Hand);
				} else {
					g_engine->_cursor.setCursorType(HandPointing);
				}
			} else {
				g_engine->_cursor.setCursorType(HandPointing);
			}
		} else {
			g_engine->_cursor.setCursorType(HandPointing);
		}
	} else if (g_engine->_actionMode == LookAction) {
		if (checkCursorAndMoveableObjects() != -1 || CheckCursorAndMovedObjects() != -1) {
			g_engine->_cursor.setCursorType(ExclamationMark);
		} else {
			int objIdx = checkCursorAndStaticObjects(0,0);
			if (objIdx != -1 && _roomObj[objIdx].objNum > 7) {
				g_engine->_cursor.setCursorType(ExclamationMark);
			} else {
				g_engine->_cursor.setCursorType(Look);
			}
		}
	} else {
		if (g_engine->_actionMode != PointerAction) {
			g_engine->_cursor.setCursorType(Pointer);
		}

		int objectUnderCursor = checkCursorAndStaticObjects(g_engine->_cursor.getX(), g_engine->_cursor.getY());
		if (objectUnderCursor == -1 ||
			((_roomObj[objectUnderCursor].objNum > 5 || _roomObj[objectUnderCursor].type != 0) &&
			(g_engine->_objectVar[59] != 2 || _roomObj[objectUnderCursor].objNum != 59) &&
			(g_engine->_objectVar[78] != 2 || _roomObj[objectUnderCursor].objNum != 78))
			) {
			g_engine->_cursor.setCursorType(Pointer);
		} else {
			int roomExitObjNum = getRoomExitAtCursor();
			uint16 targetRoomNumber = getDoorTargetRoom(roomExitObjNum);
			if ((((((_roomNumber == 59) && (g_engine->_objectVar[190] < 2)) ||
				   ((_roomNumber == 61 && ((g_engine->_objectVar[22] < 3 && (targetRoomNumber == 13)))))) ||
				  ((_roomNumber == 7 && ((targetRoomNumber == 38 && (g_engine->_objectVar[137] == 0)))))) ||
				 ((((_roomNumber == 7 && ((targetRoomNumber == 38 && (g_engine->_objectVar[57] == 1)))) ||
					((_roomNumber == 46 && ((targetRoomNumber == 60 && (g_engine->_objectVar[57] == 1)))))) ||
				   ((((_roomNumber == 13 && ((targetRoomNumber == 31 && (g_engine->_objectVar[23] != 1)))) ||
					  ((_roomNumber == 2 && ((targetRoomNumber == 0 && (g_engine->_objectVar[78] != 2)))))) ||
					 ((_roomNumber == 0 && ((targetRoomNumber == 2 && (g_engine->_objectVar[78] != 2)))))))))) ||
				(((_roomNumber == 32 && ((targetRoomNumber == 13 && (g_engine->_objectVar[23] != 1)))) ||
				  (((((_roomNumber == 13 && ((targetRoomNumber == 32 && (g_engine->_objectVar[23] != 1)))) ||
					  ((_roomNumber == 39 && ((targetRoomNumber == 46 && (g_engine->_objectVar[117] == 0)))))) ||
					 ((_roomNumber == 3 && ((targetRoomNumber == 9 && (g_engine->_objectVar[59] != 2)))))) ||
					((_roomNumber == 9 && ((targetRoomNumber == 3 && (g_engine->_objectVar[59] != 2)))))))))) {
				g_engine->_cursor.setCursorType(Pointer);
			} else {
				g_engine->_cursor.setCursorType(ConnectorEntrance);
			}
		}
	}
}

bool Darkseed::Room::exitRoom() {
	return false;
}
int Darkseed::Room::getExitRoomNumberAtPoint(int x, int y) {
	int obj = checkCursorAndStaticObjects(x, y);
	for (int i = 0; i < room1.size(); i++) {
		if (
			room1[i].roomNumber != 0xff
			&& _roomObj[obj].xOffset <= room1[i].x
			&& room1[i].x <= _roomObj[obj].width + _roomObj[obj].xOffset
			&& _roomObj[obj].yOffset <= room1[i].y
			&& room1[i].y <= _roomObj[obj].yOffset + _roomObj[obj].height
			) {
			return room1[i].roomNumber;
		}
	}
	return -1;
}
Common::String Darkseed::Room::getRoomFilenameBase(int roomNumber) {
	if (roomNumber == 20 || roomNumber == 22) {
		return "room19";
	}
	return Common::String::format("room%d", roomNumber);
}

bool Darkseed::Room::canWalkAtLocation(int x, int y) {
	if (x < 69 || x >= 570 || y < 40 || y >= 239) {
		return false;
	}

	int t = (x - 69) / 5;

	return (walkableLocationsMap[t / 8].strip[(y - 40) / 5] >> (7 - (t % 8) & 0x1f) & 1);
}

bool Darkseed::Room::canWalkInLineToTarget(int srcX, int srcY, int destX, int destY) {
	int iVar1;
	int iVar2;
	int iVar4;

	iVar1 = destX - srcX;
	destY = destY - srcY;
	iVar4 = 0;
	if ((iVar1 < 0) || (0 < destY)) {
		if ((iVar1 < 1) && (destY < 1)) {
			iVar2 = -iVar1;
			if (-iVar1 <= -destY) { //destY == iVar1 || SBORROW2(iVar2,-destY) != iVar2 + destY < 0) { //-iVar1 <= -destY
				while (srcX != destX) {
					iVar4 = iVar4 - iVar1;
					if (-iVar4 == destY || -destY < iVar4) {
						iVar4 = iVar4 + destY;
						srcX = srcX + -1;
					}
					srcY = srcY + -1;
					iVar2 = canWalkAtLocation(srcX,srcY);
					if (iVar2 == 0) {
						return 0;
					}
				}
			}
			else {
				while (srcX != destX) {
					iVar4 = iVar4 - destY;
					if (-iVar4 == iVar1 || -iVar1 < iVar4) {
						iVar4 = iVar4 + iVar1;
						srcY = srcY + -1;
					}
					srcX = srcX + -1;
					iVar2 = canWalkAtLocation(srcX,srcY);
					if (iVar2 == 0) {
						return 0;
					}
				}
			}
		}
		else if ((iVar1 < 1) && (0 < destY)) {
			iVar2 = -iVar1;
			if (-destY == iVar1 || iVar2 < destY) {
				while (srcX != destX) {
					iVar4 = iVar4 - iVar1;
					if (destY <= iVar4) {
						iVar4 = iVar4 - destY;
						srcX = srcX + -1;
					}
					srcY = srcY + 1;
					iVar2 = canWalkAtLocation(srcX,srcY);
					if (iVar2 == 0) {
						return 0;
					}
				}
			}
			else {
				while (srcX != destX) {
					iVar4 = iVar4 + destY;
					if (-iVar4 == iVar1 || -iVar1 < iVar4) {
						iVar4 = iVar4 + iVar1;
						srcY = srcY + 1;
					}
					srcX = srcX + -1;
					iVar2 = canWalkAtLocation(srcX,srcY);
					if (iVar2 == 0) {
						return 0;
					}
				}
			}
		}
		else {
			iVar2 = iVar1;
			if (destY < iVar1) {
				while (srcX != destX) {
					iVar4 = iVar4 + destY;
					if (iVar1 <= iVar4) {
						iVar4 = iVar4 - iVar1;
						srcY = srcY + 1;
					}
					srcX = srcX + 1;
					iVar2 = canWalkAtLocation(srcX,srcY);
					if (iVar2 == 0) {
						return 0;
					}
				}
			}
			else {
				while (srcX != destX) {
					iVar4 = iVar4 + iVar1;
					if (destY <= iVar4) {
						iVar4 = iVar4 - destY;
						srcX = srcX + 1;
					}
					srcY = srcY + 1;
					iVar2 = canWalkAtLocation(srcX,srcY);
					if (iVar2 == 0) {
						return 0;
					}
				}
			}
		}
	}
	else {
		iVar2 = -destY;
		if (iVar2 < iVar1) {
			while (srcX != destX) {
				iVar4 = iVar4 - destY;
				if (iVar1 <= iVar4) {
					iVar4 = iVar4 - iVar1;
					srcY = srcY + -1;
				}
				srcX = srcX + 1;
				iVar2 = canWalkAtLocation(srcX,srcY);
				if (iVar2 == 0) {
					return 0;
				}
			}
		}
		else {
			while (srcX != destX) {
				iVar4 = iVar4 + iVar1;
				if (-iVar4 == destY || -destY < iVar4) {
					iVar4 = iVar4 + destY;
					srcX = srcX + 1;
				}
				srcY = srcY + -1;
				iVar2 = canWalkAtLocation(srcX,srcY);
				if (iVar2 == 0) {
					return 0;
				}
			}
		}
	}
	return true;
//	int iVar2 = targetX - x;
//	int iVar3 = targetY - y;
//
//	if (iVar2 < 0 || iVar3 > 0) {
//		if (iVar2 < 1 && iVar3 < 1) {
//			if (-iVar2 <= -iVar3) {
//				int iVar4 = 0;
//				do {
//					if (x == targetX) {
//						return true;
//					}
//					iVar4 -= iVar2;
//					if (-iVar4 == iVar3 || -iVar3 < iVar4) {
//						iVar4 += iVar3;
//						x--;
//					}
//					y--;
//				} while (canWalkAtLocation(x, y));
//			}
//		} else {
//			int iVar4 = 0;
//			do {
//				if (x == targetX) {
//					return true;
//				}
//				iVar4 -= iVar3;
//				if (-iVar4 == iVar2 || -iVar2 < iVar4) {
//					iVar4 += iVar2;
//					y--;
//				}
//				x--;
//			} while (canWalkAtLocation(x, y));
//		}
//	} else if (-iVar3 < iVar2) {
//
//	} else {
//
//	}
//	return false;
}

void Darkseed::Room::printRoomDescriptionText() const {
	int textId = roomDescriptionTextTbl[_roomNumber];
	if (textId != 0) {
		g_engine->_console->printTosText(textId);
	}
}

int Darkseed::Room::getRoomExitAtCursor() {
	for (int i = 0; i < _roomObj.size(); i++) {
		Common::Rect roomRect(_roomObj[i].xOffset, _roomObj[i].yOffset, _roomObj[i].xOffset + _roomObj[i].width, _roomObj[i].yOffset + _roomObj[i].height);
		if (_roomObj[i].type == 0 && _roomObj[i].objNum < 6 && roomRect.contains(g_engine->_cursor.getPosition())) {
			selectedObjIndex = i;
			return _roomObj[i].objNum;
		}
	}
	return 0;
}

void Darkseed::Room::getWalkTargetForObjectType_maybe(int objId) {
	for (int i = 0; i < _roomObj.size(); i++) {
		if (_roomObj[i].objNum == objId && _roomObj[i].type == 4) {
			g_engine->_player->_walkTarget.x = _roomObj[i].xOffset;
			g_engine->_player->_walkTarget.y = _roomObj[i].yOffset;
			for (int j = 0; j < room1.size(); j++) {
				if (room1[j].roomNumber != 0xff
					&& _roomObj[selectedObjIndex].xOffset < room1[j].x
					&& room1[j].x < _roomObj[selectedObjIndex].xOffset + _roomObj[selectedObjIndex].width
					&& _roomObj[selectedObjIndex].yOffset < room1[j].y
					&& room1[j].y < _roomObj[selectedObjIndex].yOffset + _roomObj[selectedObjIndex].height
					) {
					if (_roomNumber != 0x3d || room1[j].roomNumber == 5 || g_engine->trunkPushCounter > 2) {
						g_engine->useDoorTarget = true;
					}
					g_engine->targetRoomNumber = room1[j].roomNumber;
					g_engine->targetPlayerDirection = room1[j].direction;
					break;
				}
			}
			return;
		}
	}
	g_engine->_player->_walkTarget.x = g_engine->_cursor.getX();
	g_engine->_player->_walkTarget.y = g_engine->_cursor.getY();
}

static const int scaleTbl[] = {
	 1000,          1000,          1000,          1000,
	 1000,          1000,          1000,          1000,
	 1000,          1000,           400,           750,
	  800,          1000,          1000,          1000,
	 1000,          1000,          1000,          1000,
	 1000,          1000,          1000,          1000,
	  750,           850,          1000,          1000,
	 1000,          1000,          1000,           800,
	 1000,          1000,          1000,           900,
	 1000,          1000,          1000,          1000,
	 1000,          1000,          1000,           830,
	 1000,           750,           550,           500,
	  650,          1000,           950,          1000,
	  500,           750,           700,           800,
	  800,          1000,          1000,          1000,
	 1000,          1000,          1000,           245,
	  750,           800,           500,           700,
	  800
};

static const uint8 room_sprite_related_2c85_4303[] = {
	 13,            13,            25,            25,
	28,            15,            22,            18,
	18,            13,            15,            15,
	35,            18,            40,            45,
	25,            22,            20,            10,
	10,            10,            10,            10,
	40,            20,            50,            30,
	25,            10,            10,            35,
	55,            35,            10,            45,
	15,            20,            13,            20,
	20,            15,            25,            30,
	20,            20,            30,            40,
	40,            60,            20,            15,
	5,            20,            10,            35,
	40,            15,            45,            10,
	34,            20,            25,             5,
	15,            25,            10,            10,
	15
};

void Darkseed::Room::calculateScaledSpriteDimensions(int width, int height, int curYPosition) {
	int local_6 = (g_engine->sprite_y_scaling_threshold_maybe - 2) - curYPosition;
	if (local_6 <= 0) {
		local_6 = 0;
	}
	g_engine->scaledWalkSpeed_maybe = scaleTbl[_roomNumber] - ((room_sprite_related_2c85_4303[_roomNumber] * local_6) / 5);
	g_engine->scaledSpriteWidth = (width * g_engine->scaledWalkSpeed_maybe) / 1000;
	g_engine->scaledSpriteHeight = (height * g_engine->scaledWalkSpeed_maybe) / 1000;
}

uint16 Darkseed::Room::getDoorTargetRoom(int objId) {
	for (int i = 0; i < _roomObj.size(); i++) {
		if (_roomObj[i].objNum == objId && _roomObj[i].type == 4) {
			for (int j = 0; j < room1.size(); j++) {
				if (room1[j].roomNumber != 0xff
					&& _roomObj[selectedObjIndex].xOffset < room1[j].x
					&& room1[j].x < _roomObj[selectedObjIndex].xOffset + _roomObj[selectedObjIndex].width
					&& _roomObj[selectedObjIndex].yOffset < room1[j].y
					&& room1[j].y < _roomObj[selectedObjIndex].yOffset + _roomObj[selectedObjIndex].height
				) {
					return room1[j].roomNumber;
				}
			}
		}
	}
	return g_engine->targetRoomNumber; //TODO is this a safe fallback if no door exists?
}

int Darkseed::Room::getObjectUnderCursor() {
	_collisionType = 0;
	int objIdx = checkCursorAndMoveableObjects();
	if (objIdx == -1) {
		objIdx = CheckCursorAndMovedObjects();
		if (objIdx == -1) {
			objIdx = checkCursorAndStaticObjects(0,0);
		}
	}
	return objIdx;
}

bool Darkseed::Room::isOutside() {
	bool isRoomOutside;

	if (_roomNumber == 61) {
		isRoomOutside = true;
	}
	else if (_roomNumber < 10 || _roomNumber > 14) {
		if (_roomNumber < 24 || _roomNumber > 27) {
			if (_roomNumber >= 63 && _roomNumber <= 65) {
				isRoomOutside = true;
			}
			else if (_roomNumber == 31 || _roomNumber == 32 || _roomNumber == 36) {
				isRoomOutside = true;
			}
			else {
				isRoomOutside = false;
			}
		}
		else {
			isRoomOutside = true;
		}
	}
	else {
		isRoomOutside = true;
	}
	return isRoomOutside;
}

void Darkseed::Room::runRoomObjects() {
	if (_roomNumber == 0 && g_engine->_objectVar[78] == 2) {
		const Sprite &sprite = _locationSprites.getSpriteAt(0);
		g_engine->_sprites.addSpriteToDrawList(519, 80, &sprite, 255, sprite.width, sprite.height, false);
	}
	if (_roomNumber == 0 && g_engine->isPlayingAnimation_maybe) {
		const Sprite &sprite = _locationSprites.getSpriteAt(1);
		g_engine->_sprites.addSpriteToDrawList(111, 136, &sprite, 255, sprite.width, sprite.height, false);
	}
	if (_roomNumber == 2 && g_engine->_player->_isAutoWalkingToBed && g_engine->_player->_position.x < 150) {
			g_engine->_objectVar[78] = 2; // open door for player.
	}
	if (_roomNumber == 2 && g_engine->_objectVar[78] == 2) {
		const Sprite &sprite = _locationSprites.getSpriteAt(0);
		g_engine->_sprites.addSpriteToDrawList(69, 104, &sprite, 255, sprite.width, sprite.height, false);
	}
}

bool Darkseed::Room::isGiger() {
	return _roomNumber >= 38 && (_roomNumber <= 60 || _roomNumber >= 66);
}

void Darkseed::Room::removeObjectFromRoom(int16 objNum) {
	if (_collisionType == 0) {
		for (auto &roomObj : _roomObj) {
			if (roomObj.objNum == objNum) {
				roomObj.type = 255;
				roomObj.objNum = 999;
				break;
			}
		}
	} else {
		g_engine->_objectVar.setMoveObjectRoom(objNum, 253);
	}
}
