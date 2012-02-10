/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/myst3/database.h"
#include "engines/myst3/myst3.h"

#include "common/file.h"
#include "common/debug.h"
#include "common/md5.h"
#include "common/memstream.h"
#include "common/substream.h"

namespace Myst3 {

Database::Database(Myst3Engine *vm) :
		_vm(vm),
		_currentRoomID(0),
		_executableVersion(0),
		_currentRoomData(0) {

	_executableVersion = _vm->getExecutableVersion();

	if (_executableVersion != 0) {
		debug("Initializing database from %s (Platform: %s) (%s)", _executableVersion->executable, getPlatformDescription(_vm->getPlatform()), _executableVersion->description);

		if (_executableVersion->flags & kFlagSafeDisc) {
			// TODO: SafeDisc encrypted binary
			error("Unhandled SafeDisc encrypted executable");
		}
	} else {
		error("Could not find any executable to load");
	}

	// Load the ages and rooms description
	Common::SeekableSubReadStreamEndian *file = openDatabaseFile();
	file->seek(_executableVersion->ageTableOffset);
	_ages = loadAges(*file);

	for (uint i = 0; i < _ages.size(); i++) {
		file->seek(_ages[i].roomsOffset);

		// Read the room offset table
		Common::Array<uint32> roomsOffsets;
		for (uint j = 0; j < _ages[i].roomCount; j++) {
			uint32 offset = file->readUint32() - _executableVersion->baseOffset;
			roomsOffsets.push_back(offset);
		}

		// Load the rooms
		for (uint j = 0; j < roomsOffsets.size(); j++) {
			file->seek(roomsOffsets[j]);

			_ages[i].rooms.push_back(loadRoomDescription(*file));
		}
	}

	file->seek(_executableVersion->nodeInitScriptOffset);
	_nodeInitScript = loadOpcodes(*file);

	file->seek(_executableVersion->soundNamesOffset);
	loadSoundNames(file);

	delete file;

	preloadCommonRooms();
}

void Database::preloadCommonRooms() {
	// XXXX, MENU, JRNL
	static const uint32 commonRooms[3] = { 101, 901, 902 };

	for (uint i = 0; i < 3; i++) {
		RoomData *data = findRoomData(commonRooms[i]);
		_roomNodesCache.setVal(commonRooms[i], loadRoomScripts(data));
	}
}

Common::Array<uint16> Database::listRoomNodes(uint32 roomID, uint32 ageID) {
	Common::Array<NodePtr> nodes;
	Common::Array<uint16> list;

	if (roomID == 0)
		roomID = _currentRoomID;

	if (_roomNodesCache.contains(roomID)) {
		nodes = _roomNodesCache.getVal(roomID);
	} else {
		RoomData *data = findRoomData(roomID);
		nodes = loadRoomScripts(data);
	}

	for (uint i = 0; i < nodes.size(); i++) {
		list.push_back(nodes[i]->id);
	}

	return list;
}

NodePtr Database::getNodeData(uint16 nodeID, uint32 roomID, uint32 ageID) {
	Common::Array<NodePtr> nodes;

	if (roomID == 0)
		roomID = _currentRoomID;

	if (_roomNodesCache.contains(roomID)) {
		nodes = _roomNodesCache.getVal(roomID);
	} else {
		RoomData *data = findRoomData(roomID);
		nodes = loadRoomScripts(data);
	}

	for (uint i = 0; i < nodes.size(); i++) {
		if (nodes[i]->id == nodeID)
			return nodes[i];
	}

	return NodePtr();
}

RoomData *Database::findRoomData(const uint32 & roomID)
{
	for (uint i = 0; i < _ages.size(); i++)
		for (uint j = 0; j < _ages[i].rooms.size(); j++) {
			if (_ages[i].rooms[j].id == roomID) {
				return &_ages[i].rooms[j];
			}
		}
	return 0;
}

Common::Array<NodePtr> Database::loadRoomScripts(RoomData *room) {
	Common::Array<NodePtr> nodes;
	Common::SeekableSubReadStreamEndian *file = openDatabaseFile();

	// Load the node scripts
	if (room->scriptsOffset) {
		file->seek(room->scriptsOffset);
		loadRoomNodeScripts(file, nodes);
	}

	// Load the ambient sound scripts, if any
	if (room->ambSoundsOffset) {
		file->seek(room->ambSoundsOffset);
		loadRoomSoundScripts(file, nodes, false);
	}

	if (room->unkOffset) {
		file->seek(room->unkOffset);
		loadRoomSoundScripts(file, nodes, true);
	}

	delete file;

	return nodes;
}

void Database::loadRoomNodeScripts(Common::SeekableSubReadStreamEndian *file, Common::Array<NodePtr> &nodes) {
	while (1) {
		int16 id = file->readUint16();

		// End of list
		if (id == 0)
			break;

		if (id <= -10)
			error("Unimplemented node list command");

		if (id > 0) {
			// Normal node
			NodePtr node = NodePtr(new NodeData());
			node->id = id;
			node->scripts = loadCondScripts(*file);
			node->hotspots = loadHotspots(*file);

			nodes.push_back(node);
		} else {
			// Several nodes sharing the same scripts
			Common::Array<int16> nodeIds;

			for (int i = 0; i < -id; i++) {
				nodeIds.push_back(file->readUint16());
			}

			Common::Array<CondScript> scripts = loadCondScripts(*file);
			Common::Array<HotSpot> hotspots = loadHotspots(*file);

			for (int i = 0; i < -id; i++) {
				NodePtr node = NodePtr(new NodeData());
				node->id = nodeIds[i];
				node->scripts = scripts;
				node->hotspots = hotspots;

				nodes.push_back(node);
			}
		}
	}
}

void Database::loadRoomSoundScripts(Common::SeekableSubReadStreamEndian *file, Common::Array<NodePtr> &nodes, bool background) {
	while (1) {
		int16 id = file->readUint16();

		// End of list
		if (id == 0)
			break;

		if (id < -10)
			error("Unimplemented node list command");

		if (id > 0) {
			// Normal node, find the node if existing
			NodePtr node;
			for (uint i = 0; i < nodes.size(); i++)
				if (nodes[i]->id == id) {
					node = nodes[i];
					break;
				}

			// Node not found, create a new one
			if (!node) {
				node = NodePtr(new NodeData());
				node->id = id;
				nodes.push_back(node);
			}

			if (background)
				node->backgroundSoundScripts.push_back(loadCondScripts(*file));
			else
				node->soundScripts.push_back(loadCondScripts(*file));
		} else {
			// Several nodes sharing the same scripts
			// Find the node ids the script applies to
			Common::Array<int16> nodeIds;

			if (id == -10)
				do {
					id = file->readUint16();
					if (id < 0) {
						uint16 end = file->readUint16();
						for (int i = -id; i < end; i++)
							nodeIds.push_back(i);

					} else if (id > 0) {
						nodeIds.push_back(id);
					}
				} while (id);
			else
				for (int i = 0; i < -id; i++) {
					nodeIds.push_back(file->readUint16());
				}

			// Load the script
			Common::Array<CondScript> scripts = loadCondScripts(*file);

			// Add the script to each matching node
			for (uint i = 0; i < nodeIds.size(); i++) {
				NodePtr node;

				// Find the current node if existing
				for (uint j = 0; j < nodes.size(); j++)
					if (nodes[j]->id == nodeIds[i]) {
						node = nodes[j];
						break;
					}

				// Node not found, skip it
				if (!node)
					continue;

				if (background)
					node->backgroundSoundScripts.push_back(scripts);
				else
					node->soundScripts.push_back(scripts);
			}
		}
	}
}

void Database::setCurrentRoom(const uint32 roomID) {
	if (roomID == _currentRoomID)
		return;

	_currentRoomData = findRoomData(roomID);

	if (!_currentRoomData || !_currentRoomData->scriptsOffset)
		return;

	// Remove old room from cache and add the new one
	_roomNodesCache.erase(_currentRoomID);
	_roomNodesCache.setVal(roomID, loadRoomScripts(_currentRoomData));

	_currentRoomID = roomID;
}

Common::Array<CondScript> Database::loadCondScripts(Common::ReadStreamEndian &s) {
	Common::Array<CondScript> scripts;

	while (1) {
		CondScript script = loadCondScript(s);

		if (!script.condition)
			break;

		scripts.push_back(script);
	}

	return scripts;
}

Common::Array<HotSpot> Database::loadHotspots(Common::ReadStreamEndian &s) {
	Common::Array<HotSpot> scripts;

	while (1) {
		HotSpot hotspot = loadHotspot(s);

		if (!hotspot.condition)
			break;

		scripts.push_back(hotspot);
	}

	return scripts;
}

Common::Array<Opcode> Database::loadOpcodes(Common::ReadStreamEndian &s) {
	Common::Array<Opcode> script;

	while (1) {
		Opcode opcode;
		uint16 code = s.readUint16();

		opcode.op = code & 0xff;
		uint8 count = code >> 8;
		if (count == 0 && opcode.op == 0)
			break;

		// The v1.0 executables use a slightly different opcode set
		// Since it's a simple conversion, we'll handle that here
		if ((_executableVersion->flags & kFlagVersion10) && opcode.op >= 122)
			opcode.op++;

		for (int i = 0; i < count; i++) {
			int16 value = s.readSint16();
			opcode.args.push_back(value);
		}

		script.push_back(opcode);
	}

	return script;
}

CondScript Database::loadCondScript(Common::ReadStreamEndian &s)
{
	CondScript script;
	script.condition = s.readUint16();
	if(!script.condition)
		return script;

	script.script = loadOpcodes(s);

	return script;
}

HotSpot Database::loadHotspot(Common::ReadStreamEndian &s) {
	HotSpot hotspot;

	hotspot.condition = s.readUint16();

	if (hotspot.condition == 0)
		return hotspot;

	if (hotspot.condition != -1) {
		hotspot.rects = loadRects(s);
		hotspot.cursor = s.readUint16();
	}

	hotspot.script = loadOpcodes(s);

	return hotspot;
}

Common::Array<PolarRect> Database::loadRects(Common::ReadStreamEndian &s) {
	Common::Array<PolarRect> rects;

	bool lastRect = false;
	do {
		PolarRect rect;
		rect.centerPitch = s.readUint16();
		rect.centerHeading = s.readUint16();
		rect.width = s.readUint16();
		rect.height = s.readUint16();

		if (rect.width < 0) {
			rect.width = -rect.width;
		} else {
			lastRect = true;
		}

		rects.push_back(rect);
	} while (!lastRect);

	return rects;
}

Common::Array<AgeData> Database::loadAges(Common::ReadStreamEndian &s)
{
	Common::Array<AgeData> ages;

	for (uint i = 0; i < 10; i++) {
		AgeData age;

		if (_vm->getPlatform() == Common::kPlatformPS2) {
			// Really 64-bit values
			age.id = s.readUint32LE();
			s.readUint32LE();
			age.disk = s.readUint32LE();
			s.readUint32LE();
			age.roomCount = s.readUint32LE();
			s.readUint32LE();
			age.roomsOffset = s.readUint32LE() - _executableVersion->baseOffset;
			s.readUint32LE();
			age.labelId = s.readUint32LE();
			s.readUint32LE();
		} else {
			age.id = s.readUint32();
			age.disk = s.readUint32();
			age.roomCount = s.readUint32();
			age.roomsOffset = s.readUint32() - _executableVersion->baseOffset;
			age.labelId = s.readUint32();
		}

		ages.push_back(age);
	}

	return ages;
}

RoomData Database::loadRoomDescription(Common::ReadStreamEndian &s) {
	RoomData room;

	if (_vm->getPlatform() == Common::kPlatformPS2) {
		room.id = s.readUint32LE(); s.readUint32LE();
		s.read(&room.name, 8);
		room.scriptsOffset = s.readUint32LE(); s.readUint32LE();
		room.ambSoundsOffset = s.readUint32LE(); s.readUint32LE();
		room.unkOffset = s.readUint32LE(); // not 64-bit -- otherwise roomUnk5 is incorrect
		room.roomUnk4 = s.readUint32LE();
		room.roomUnk5 = s.readUint32LE();
	} else {
		room.id = s.readUint32();
		s.read(&room.name, 8);
		room.scriptsOffset = s.readUint32();
		room.ambSoundsOffset = s.readUint32();
		room.unkOffset = s.readUint32();
		room.roomUnk4 = s.readUint32();
		room.roomUnk5 = s.readUint32();
	}

	if (room.scriptsOffset != 0)
		room.scriptsOffset -= _executableVersion->baseOffset;

	if (room.ambSoundsOffset != 0)
		room.ambSoundsOffset -= _executableVersion->baseOffset;

	if (room.unkOffset != 0)
		room.unkOffset -= _executableVersion->baseOffset;

	return room;
}

void Database::getRoomName(char name[8], uint32 roomID) {
	if (roomID != 0 && roomID != _currentRoomID) {
		RoomData *data = findRoomData(roomID);
		memcpy(&name[0], &data->name, 8);
	} else if (_currentRoomData) {
		memcpy(&name[0], &_currentRoomData->name, 8);
	}
}

uint32 Database::getRoomId(const char *name) {
	for (uint i = 0; i < _ages.size(); i++)
		for (uint j = 0; j < _ages[i].rooms.size(); j++) {
			if (!scumm_stricmp(_ages[i].rooms[j].name, name)) {
				return _ages[i].rooms[j].id;
			}
		}

	return 0;
}

uint32 Database::getAgeLabelId(uint32 ageID) {
	for (uint i = 0; i < _ages.size(); i++)
		if (_ages[i].id == ageID)
			return _ages[i].labelId;

	return 0;
}

Common::SeekableSubReadStreamEndian *Database::openDatabaseFile() const {
	assert(_executableVersion);

	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(_executableVersion->executable);
	bool bigEndian = false;

	if (_vm->getPlatform() == Common::kPlatformMacintosh) {
		// The data we need is always in segment 1
		Common::SeekableReadStream *segment = decompressPEFDataSegment(stream, 1);
		delete stream;
		stream = segment;
		bigEndian = true;
	}

	return new Common::SeekableSubReadStreamEndian(stream, 0, stream->size(), bigEndian, DisposeAfterUse::YES);
}

static uint32 getPEFArgument(Common::SeekableReadStream *stream, uint &pos) {
	uint32 r = 0;
	byte numEntries = 0;

	for (;;) {
		numEntries++;

		byte in = stream->readByte();
		pos++;

		if (numEntries == 5) {
			r <<= 4;
		} else {
			r <<= 7;
		}

		r += (in & 0x7f);

		if (!(in & 0x80))
			return r;

		if (numEntries == 5)
			error("bad argument in PEF");
	}
}

// decompressPEFDataSegment is entirely based on https://github.com/fuzzie/unity/blob/master/data.cpp
Common::SeekableReadStream *Database::decompressPEFDataSegment(Common::SeekableReadStream *stream, uint segmentID) const {
	// Read the header
	if (stream->readUint32BE() != MKTAG('J','o','y','!'))
		error("Bad PEF header tag 1");
	if (stream->readUint32BE() != MKTAG('p','e','f','f'))
		error("Bad PEF header tag 2");
	if (stream->readUint32BE() != MKTAG('p','w','p','c'))
		error("PEF header is not PowerPC");
	if (stream->readUint32BE() != 1)
		error("PEF header is not version 1");

	stream->skip(16); // dateTimeStamp, oldDefVersion, oldImpVersion, currentVersion
	uint16 sectionCount = stream->readUint16BE();
	stream->skip(6); // instSectionCount, reservedA

	if (segmentID >= sectionCount)
		error("Not enough segments in PEF");

	stream->skip(28 * segmentID);

	stream->skip(8); // nameOffset, defaultAddress
	uint32 totalSize = stream->readUint32BE();
	uint32 unpackedSize = stream->readUint32BE();
	uint32 packedSize = stream->readUint32BE();
	assert(unpackedSize <= totalSize);
	assert(packedSize <= unpackedSize);
	uint32 containerOffset = stream->readUint32BE();
	byte sectionKind = stream->readByte();

	switch (sectionKind) {
	case 2:
		break; // pattern-initialized data
	default:
		error("Unsupported PEF sectionKind %d", sectionKind);
	}

	debug(1, "Unpacking PEF segment of size %d (total %d, packed %d) at 0x%x", unpackedSize, totalSize, packedSize, containerOffset);

	bool r = stream->seek(containerOffset, SEEK_SET);
	assert(r);

	// note that we don't bother with the zero-initialised section..
	byte *data = (byte *)malloc(unpackedSize);

	// unpack the data
	byte *targ = data;
	unsigned int pos = 0;
	while (pos < packedSize) {
		byte next = stream->readByte();
		byte opcode = next >> 5;
		uint32 count = next & 0x1f;
		pos++;

		if (count == 0)
			count = getPEFArgument(stream, pos);

		switch (opcode) {
		case 0: // Zero
			memset(targ, 0, count);
			targ += count;
			break;

		case 1: // blockCopy
			stream->read(targ, count);
			targ += count;
			pos += count;
			break;

		case 2:	{ // repeatedBlock
				uint32 repeatCount = getPEFArgument(stream, pos);

				byte *src = targ;
				stream->read(src, count);
				targ += count;
				pos += count;

				for (uint i = 0; i < repeatCount; i++) {
					memcpy(targ, src, count);
					targ += count;
				}
			} break;

		case 3: { // interleaveRepeatBlockWithBlockCopy
				uint32 customSize = getPEFArgument(stream, pos);
				uint32 repeatCount = getPEFArgument(stream, pos);

				byte *commonData = targ;
				stream->read(commonData, count);
				targ += count;
				pos += count;

				for (uint i = 0; i < repeatCount; i++) {
					stream->read(targ, customSize);
					targ += customSize;
					pos += customSize;

					memcpy(targ, commonData, count);
					targ += count;
				}
			} break;

		case 4: { // interleaveRepeatBlockWithZero
				uint32 customSize = getPEFArgument(stream, pos);
				uint32 repeatCount = getPEFArgument(stream, pos);

				for (uint i = 0; i < repeatCount; i++) {
					memset(targ, 0, count);
					targ += count;

					stream->read(targ, customSize);
					targ += customSize;
					pos += customSize;
				}
				memset(targ, 0, count);
				targ += count;
			} break;

		default:
			error("Unknown opcode %d in PEF pattern-initialized section", opcode);
		}
	}

	if (pos != packedSize)
		error("Failed to parse PEF pattern-initialized section (parsed %d of %d)", pos, packedSize);
	if (targ != data + unpackedSize)
		error("Failed to unpack PEF pattern-initialized section");

	return new Common::MemoryReadStream(data, unpackedSize, DisposeAfterUse::YES);
}

void Database::loadSoundNames(Common::ReadStreamEndian *s) {
	_soundNames.clear(false);

	while (1) {
		uint32 id = s->readUint32();

		// 64-bit value in the PS2 binary
		if (_vm->getPlatform() == Common::kPlatformPS2)
			s->readUint32();

		if (!id)
			break;

		char name[32];
		s->read(name, sizeof(name));
		_soundNames[id] = Common::String(name);
	}
}

Common::String Database::getSoundName(uint32 id) {
	const Common::String &result = _soundNames.getVal(id, "");
	if (result.empty())
		error("Unable to find a sound with id %d", id);

	return result;
}

} /* namespace Myst3 */
