/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/myst3/database.h"

#include "common/file.h"
#include "common/debug.h"
#include "common/md5.h"

namespace Myst3 {

Database::Database(const Common::String &executable) :
		_exePath(executable),
		_currentRoomID(0),
		_gameVersion(0),
		_currentRoomData(0) {

	// Game versions database
	static GameVersion versions[] = {
			{ "1.22 French", "554612b239ff2d9a3364fa38e3f32b45", 0x486108, 0x486040 }
	};

	Common::File file;
	file.open(_exePath);

	// Check whether the game version is known
	Common::String md5 = Common::computeStreamMD5AsString(file, 0);

	for (uint i = 0; i < sizeof(versions) / sizeof(GameVersion); i++) {
		if (md5.equals(versions[i].md5)) {
			_gameVersion = &versions[i];
			break;
		}
	}

	if (_gameVersion != 0) {
		debug("Initializing database from %s (%s)", _exePath.c_str(), _gameVersion->description);
	} else {
		error("Unknown game version: %s (md5: %s)", _exePath.c_str(), md5.c_str());
	}

	// Load the ages and rooms description
	file.seek(_gameVersion->ageTableOffset - _baseOffset);
	_ages = loadAges(file);

	for (uint i = 0; i < _ages.size(); i++) {
		file.seek(_ages[i].roomsOffset);

		// Read the room offset table
		Common::Array<uint32> roomsOffsets;
		for (uint j = 0; j < _ages[i].roomCount; j++) {
			uint32 offset = file.readUint32LE() - _baseOffset;
			roomsOffsets.push_back(offset);
		}

		// Load the rooms
		for (uint j = 0; j < roomsOffsets.size(); j++) {
			file.seek(roomsOffsets[j]);

			_ages[i].rooms.push_back(loadRoomDescription(file));
		}
	}

	file.seek(_gameVersion->nodeInitScriptOffset - _baseOffset);
	_nodeInitScript = loadOpcodes(file);

	file.close();
}

Common::Array<uint16> Database::listRoomNodes(uint8 roomID, uint32 ageID) {
	Common::Array<NodePtr> nodes;
	Common::Array<uint16> list;

	if (roomID != 0 && roomID != _currentRoomID) {
		RoomData *data = findRoomData(roomID);
		nodes = loadRoomScripts(data);
	} else {
		nodes = _currentRoomNodes;
	}

	for (uint i = 0; i < nodes.size(); i++) {
		list.push_back(nodes[i]->id);
	}

	return list;
}

NodePtr Database::getNodeData(uint16 nodeID, uint8 roomID, uint32 ageID) {
	Common::Array<NodePtr> nodes;

	if (roomID != 0 && roomID != _currentRoomID) {
		RoomData *data = findRoomData(roomID);
		nodes = loadRoomScripts(data);
	} else {
		nodes = _currentRoomNodes;
	}

	for (uint i = 0; i < nodes.size(); i++) {
		if (nodes[i]->id == nodeID)
			return nodes[i];
	}

	return NodePtr();
}

RoomData *Database::findRoomData(const uint8 & roomID)
{
	for (uint i = 0; i < _ages.size(); i++)
		for (uint j = 0; j < _ages[i].rooms.size(); j++) {
			if (_ages[i].rooms[j].id == roomID) {
				return &_ages[i].rooms[j];
				break;
			}
		}
	return 0;
}

Common::Array<NodePtr> Database::loadRoomScripts(RoomData *room) {
	Common::Array<NodePtr> nodes;

	Common::File file;
	file.open(_exePath);
	file.seek(room->scriptsOffset);


	while (1) {
		int16 id = file.readUint16LE();

		if (id <= 0)
			break;

		NodePtr node = NodePtr(new NodeData());
		node->id = id;
		node->scripts = loadCondScripts(file);
		node->hotspots = loadHotspots(file);

		nodes.push_back(node);
	}

	file.close();

	return nodes;
}

void Database::setCurrentRoom(const uint8 roomID) {
	if (roomID == _currentRoomID)
		return;

	_currentRoomData = findRoomData(roomID);

	if (!_currentRoomData || !_currentRoomData->scriptsOffset)
		return;

	_currentRoomID = roomID;
	_currentRoomNodes = loadRoomScripts(_currentRoomData);
}

Common::Array<CondScript> Database::loadCondScripts(Common::ReadStream &s) {
	Common::Array<CondScript> scripts;

	while (1) {
		CondScript script = loadCondScript(s);

		if (!script.condition)
			break;

		scripts.push_back(script);
	}

	return scripts;
}

Common::Array<HotSpot> Database::loadHotspots(Common::ReadStream &s) {
	Common::Array<HotSpot> scripts;

	while (1) {
		HotSpot hotspot = loadHotspot(s);

		if (!hotspot.condition)
			break;

		scripts.push_back(hotspot);
	}

	return scripts;
}

Common::Array<Opcode> Database::loadOpcodes(Common::ReadStream &s)
{
	Common::Array<Opcode> script;

	while(1){
		Opcode opcode;
		opcode.op = s.readByte();
		uint8 count = s.readByte();
		if(count == 0 && opcode.op == 0)
			break;

		for(int i = 0;i < count;i++){
			uint16 value = s.readUint16LE();
			opcode.args.push_back(value);
		}
		script.push_back(opcode);
	}

	return script;
}

CondScript Database::loadCondScript(Common::ReadStream & s)
{
	CondScript script;
	script.condition = s.readUint16LE();
	if(!script.condition)
		return script;

	script.script = loadOpcodes(s);

	return script;
}

HotSpot Database::loadHotspot(Common::ReadStream &s) {
	HotSpot hotspot;

	hotspot.condition = s.readUint16LE();

	if (hotspot.condition == 0)
		return hotspot;

	if (hotspot.condition != -1) {
		hotspot.rects = loadRects(s);
		hotspot.unk2 = s.readUint16LE();
	}

	hotspot.script = loadOpcodes(s);

	return hotspot;
}

Common::Array<PolarRect> Database::loadRects(Common::ReadStream &s) {
	Common::Array<PolarRect> rects;

	bool lastRect = false;
	do {
		PolarRect rect;
		rect.centerPitch = s.readUint16LE();
		rect.centerHeading = s.readUint16LE();
		rect.width = s.readUint16LE();
		rect.height = s.readUint16LE();

		if (rect.width < 0) {
			rect.width = -rect.width;
		} else {
			lastRect = true;
		}

		rects.push_back(rect);
	} while (!lastRect);

	return rects;
}

Common::Array<AgeData> Database::loadAges(Common::ReadStream &s)
{
	Common::Array<AgeData> ages;

	for (uint i = 0; i < 10; i++) {
		AgeData age;
		age.id = s.readUint32LE();
		age.disk = s.readUint32LE();
		age.roomCount = s.readUint32LE();
		age.roomsOffset = s.readUint32LE() - _baseOffset;
		age.ageUnk1 = s.readUint32LE();

		ages.push_back(age);
	}

	return ages;
}

RoomData Database::loadRoomDescription(Common::ReadStream &s) {
	RoomData room;

	room.id = s.readByte();
	room.roomUnk1 = s.readByte();
	room.roomUnk2 = s.readByte();
	room.roomUnk3 = s.readByte();
	s.read(&room.name, 8);
	room.scriptsOffset = s.readUint32LE();
	room.ambSoundsOffset = s.readUint32LE();
	room.unkOffset = s.readUint32LE();
	room.roomUnk4 = s.readUint32LE();
	room.roomUnk5 = s.readUint32LE();

	if (room.scriptsOffset != 0)
		room.scriptsOffset -= _baseOffset;

	if (room.ambSoundsOffset != 0)
		room.ambSoundsOffset -= _baseOffset;

	if (room.unkOffset != 0)
		room.unkOffset -= _baseOffset;

	return room;
}

void Database::getRoomName(char name[8], uint8 roomID) {
	if (roomID != 0 && roomID != _currentRoomID) {
		RoomData * data = findRoomData(roomID);
		memcpy(&name[0], &data->name, 8);
	} else if (_currentRoomData) {
		memcpy(&name[0], &_currentRoomData->name, 8);
	}
}

} /* namespace Myst3 */
