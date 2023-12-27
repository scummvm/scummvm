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

Darkseed::Room::Room(int roomNumber) : _roomNumber(roomNumber) {
	room1.resize(8);
	room2.resize(16);
	room3.resize(30);

	if(!load()) {
		error("Failed to load room %d", roomNumber);
	}
}

bool Darkseed::Room::load() {
	Common::String filenameBase = getRoomFilenameBase(_roomNumber);
	Common::String romFilename;
	Common::File file;
	romFilename = Common::String::format("%s.rom", filenameBase.c_str(), _roomNumber);
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
		room1[i].unk4 = file.readUint16BE();
		room1[i].unk6 = file.readUint16BE();
		room1[i].roomNumber = file.readUint16BE();
		room1[i].unka = file.readByte();
	}

	for (int i = 0; i < 16; i++) {
		file.read(room2[i].strip, 40);
	}

	for (int i = 0; i < 30; i++) {
		room3[i].unk0 = file.readUint16BE();
		room3[i].unk2 = file.readUint16BE();
		room3[i].xOffset = file.readUint16BE();
		room3[i].yOffset = file.readUint16BE();
		room3[i].width = file.readUint16BE();
		room3[i].height = file.readUint16BE();
		room3[i].unkc = file.readByte();
		room3[i].unkd = file.readByte();

		if (room3[i].unkd >= 0x29 && room3[i].unk0 != 0 && room3[i].unk0 != 1000) {
			room3[i].height = 0x14;
			room3[i].width = 0x14;
			room3[i].unk0 = 0;
		}

		if (room3[i].unk2 == 0 && room3[i].unk0 == 1) {
			if (connectors.size() == 0xc) {
				error("Too many connectors in this room, max of %", 0xc);
			}
			RoomConnector connector;
			connector.x = room3[i].xOffset;
			connector.y = room3[i].yOffset;

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
			connectors.push_back(connector);
			room3[i].unk0 = 0xff;
		}
	}

	file.close();

	if(!pic.load(picFilename)) {
		return false;
	}

	_pal.load(Common::String::format("%s.pal", filenameBase.c_str()));

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
	for (int i = 0; i < connectors.size(); i++) {
		g_engine->_screen->drawLine(connectors[i].x, connectors[i].y, connectors[i].x + 7, connectors[i].y + 13, 2);
	}
}

int Darkseed::Room::getObjectAtPoint(int x, int y) {
	const Sprite &cursorSprite = g_engine->_cursor.getSprite();
	ActionMode actionMode = g_engine->_actionMode;
	bool hasObject = false;
	for (int i = 0; i < room3.size(); i++) {
		if (room3[i].unk0 == 0
			&& room3[i].xOffset <= cursorSprite.width + g_engine->_cursor.getX()
			&& g_engine->_cursor.getX() <= room3[i].width + room3[i].xOffset
			&& room3[i].yOffset <= cursorSprite.height + g_engine->_cursor.getY()
			&& g_engine->_cursor.getY() <= room3[i].height + room3[i].yOffset
		) {
			if (actionMode != PointerAction && room3[i].unk2 >= 5) {
				hasObject = true;
			}

			if (actionMode == PointerAction && room3[i].unk2 < 6) {
				hasObject = true;
			}

			if (room3[i].unk2 == 0x3b || room3[i].unk2 == 0x4e) {
				// TODO
//				if (DAT_2c85_8186 == 1) {
//					hasObject = true;
//				}
//				else {
//					hasObject = false;
//				}
			}

			if (room3[i].unk2 == 0x19 && hasObject) {
//				if (DAT_2c85_81e2 < 2) {
//					hasObject = false;
//				}
//				else {
//					hasObject = true;
//				}
			}

			if (room3[i].unk2 == 0x74 && hasObject && (int)actionMode != 0x13) {
				hasObject = false;
			}

			if (hasObject) {
				return i;
			}
		}
	}
	return -1;
}

void Darkseed::Room::update() {
	int objectUnderCursor = getObjectAtPoint(g_engine->_cursor.getX(), g_engine->_cursor.getY());
	if (g_engine->_actionMode == PointerAction) {
		g_engine->_cursor.setCursorType(objectUnderCursor != -1 ? ConnectorEntrance : Pointer);
	}
}

bool Darkseed::Room::exitRoom() {
	return false;
}
int Darkseed::Room::getExitRoomNumberAtPoint(int x, int y) {
	int obj = getObjectAtPoint(x, y);
	for (int i = 0; i < room1.size(); i++) {
		if (
			room1[i].roomNumber != 0xff
			&& room3[obj].xOffset <= room1[i].x
			&& room1[i].x <= room3[obj].width + room3[obj].xOffset
			&& room3[obj].yOffset <= room1[i].y
			&& room1[i].y <= room3[obj].yOffset + room3[obj].height
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
