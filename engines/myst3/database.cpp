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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/myst3/database.h"
#include "engines/myst3/myst3.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/hashmap.h"
#include "common/md5.h"
#include "common/memstream.h"
#include "common/substream.h"
#include "common/winexe_pe.h"

namespace Myst3 {

Database::Database(Myst3Engine *vm) :
		_vm(vm),
		_currentRoomID(0),
		_executableVersion(0),
		_currentRoomData(0) {

	_executableVersion = _vm->getExecutableVersion();

	if (_executableVersion != 0) {
		debug("Initializing database from %s (Platform: %s) (%s)", _executableVersion->executable, getPlatformDescription(_vm->getPlatform()), _executableVersion->description);
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

	// TODO: Remove once the offset table is complete
	if (!_executableVersion->ambientCuesOffset) {
		error("The description for this executable (%s, %s) does not contain the ambient cues offset. Please contact the ResidualVM team.",
				_executableVersion->executable, _executableVersion->description);
	}

	file->seek(_executableVersion->ambientCuesOffset);
	loadAmbientCues(file);

	preloadCommonRooms(file);
	initializeZipBitIndexTable(file);

	delete file;
}

void Database::preloadCommonRooms(Common::SeekableSubReadStreamEndian *file) {
	// XXXX, MENU, JRNL
	static const uint32 commonRooms[3] = { 101, 901, 902 };

	for (uint i = 0; i < 3; i++) {
		RoomData *data = findRoomData(commonRooms[i]);
		_roomNodesCache.setVal(commonRooms[i], loadRoomScripts(file, data));
	}
}

Common::Array<NodePtr> Database::getRoomNodes(uint32 roomID) {
	Common::Array<NodePtr> nodes;

	if (_roomNodesCache.contains(roomID)) {
		nodes = _roomNodesCache.getVal(roomID);
	} else {
		RoomData *data = findRoomData(roomID);

		Common::SeekableSubReadStreamEndian *file = openDatabaseFile();
		nodes = loadRoomScripts(file, data);
		delete file;
	}

	return nodes;
}

Common::Array<uint16> Database::listRoomNodes(uint32 roomID, uint32 ageID) {
	Common::Array<NodePtr> nodes;
	Common::Array<uint16> list;

	if (roomID == 0)
		roomID = _currentRoomID;

	nodes = getRoomNodes(roomID);

	for (uint i = 0; i < nodes.size(); i++) {
		list.push_back(nodes[i]->id);
	}

	return list;
}

NodePtr Database::getNodeData(uint16 nodeID, uint32 roomID, uint32 ageID) {
	Common::Array<NodePtr> nodes;

	if (roomID == 0)
		roomID = _currentRoomID;

	nodes = getRoomNodes(roomID);

	for (uint i = 0; i < nodes.size(); i++) {
		if (nodes[i]->id == nodeID)
			return nodes[i];
	}

	return NodePtr();
}

void Database::initializeZipBitIndexTable(Common::SeekableSubReadStreamEndian *file) {
	int16 zipBit = 0;
	for (uint i = 0; i < _ages.size(); i++) {
		for (uint j = 0; j < _ages[i].rooms.size(); j++) {
			_roomZipBitIndex.setVal(_ages[i].rooms[j].id, zipBit);

			// Add the highest zip-bit index for the current room
			// to get the zip-bit index for the next room
			int16 maxZipBitForRoom = 0;
			Common::Array<NodePtr> nodes = loadRoomScripts(file, &_ages[i].rooms[j]);
			for (uint k = 0; k < nodes.size(); k++) {
				maxZipBitForRoom = MAX(maxZipBitForRoom, nodes[k]->zipBitIndex);
			}

			zipBit += maxZipBitForRoom + 1;
		}
	}
}

int32 Database::getNodeZipBitIndex(uint16 nodeID, uint32 roomID) {
	if (roomID == 0) {
		roomID = _currentRoomID;
	}

	if (!_roomZipBitIndex.contains(roomID)) {
		error("Unable to find zip-bit index for room %d", roomID);
	}

	Common::Array<NodePtr> nodes = getRoomNodes(roomID);

	for (uint i = 0; i < nodes.size(); i++) {
		if (nodes[i]->id == nodeID) {
			return _roomZipBitIndex[roomID] + nodes[i]->zipBitIndex;
		}
	}

	error("Unable to find zip-bit index for node (%d, %d)", nodeID, roomID);
}

RoomData *Database::findRoomData(uint32 roomID) {
	for (uint i = 0; i < _ages.size(); i++) {
		for (uint j = 0; j < _ages[i].rooms.size(); j++) {
			if (_ages[i].rooms[j].id == roomID) {
				return &_ages[i].rooms[j];
			}
		}
	}

	error("No room with ID %d", roomID);
}

Common::Array<NodePtr> Database::loadRoomScripts(Common::SeekableSubReadStreamEndian *file, RoomData *room) {
	Common::Array<NodePtr> nodes;

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

	return nodes;
}

void Database::loadRoomNodeScripts(Common::SeekableSubReadStreamEndian *file, Common::Array<NodePtr> &nodes) {
	uint zipIndex = 0;
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
			node->zipBitIndex = zipIndex;
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
				node->zipBitIndex = zipIndex;
				node->scripts = scripts;
				node->hotspots = hotspots;

				nodes.push_back(node);
			}
		}

		zipIndex++;
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
				for (uint j = 0; j < nodes.size(); j++) {
					if (nodes[j]->id == nodeIds[i]) {
						node = nodes[j];
						break;
					}
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

	Common::SeekableSubReadStreamEndian *file = openDatabaseFile();

	// Remove old room from cache and add the new one
	_roomNodesCache.erase(_currentRoomID);
	_roomNodesCache.setVal(roomID, loadRoomScripts(file, _currentRoomData));

	_currentRoomID = roomID;

	delete file;
}

Common::Array<CondScript> Database::loadCondScripts(Common::SeekableSubReadStreamEndian &s) {
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

CondScript Database::loadCondScript(Common::SeekableSubReadStreamEndian &s) {
	CondScript script;
	script.condition = s.readUint16();
	if(!script.condition)
		return script;

	// WORKAROUND: Original data bug in MATO 32765
	// The script data for node MATO 32765 is missing its first two bytes
	// of data, resulting in incorrect opcodes being read

	// Original disassembly:
	// init 0 > c[v565 != 0]
	//     op 115, ifVarInRange ( )
	//     op 45, inventoryAddBack ( )
	//     op 53, varSetValue ( vSunspotColor 4090 )
	//     op 53, varSetValue ( vSunspotRadius 40 )
	//     op 33, waterEffectSetWave ( 100 80 )
	//     op 32, waterEffectSetAttenuation ( 359 )
	//     op 31, waterEffectSetSpeed ( 15 )

	// Fixed disassembly
	// init 0 > c[v1 != 0]
	//     op 53, varSetValue ( vSunspotIntensity 45 )
	//     op 53, varSetValue ( vSunspotColor 4090 )
	//     op 53, varSetValue ( vSunspotRadius 40 )
	//     op 33, waterEffectSetWave ( 100 80 )
	//     op 32, waterEffectSetAttenuation ( 359 )
	//     op 31, waterEffectSetSpeed ( 15 )

	if (script.condition == 565) {
		script.condition = 1;
		s.seek(-2, SEEK_CUR);
	}
	// END WORKAROUND

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

Common::Array<AgeData> Database::loadAges(Common::ReadStreamEndian &s) {
	Common::Array<AgeData> ages;

	uint ageCount;
	if (_vm->getPlatform() == Common::kPlatformXbox) {
		// The DVD version also has the 11th age (LOGO) but does not use it.
		ageCount = 11;
	} else {
		ageCount = 10;
	}

	for (uint i = 0; i < ageCount; i++) {
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
		s.readUint32LE(); // The zip-bit index is computed at runtime
		room.roomUnk5 = s.readUint32LE();
	} else {
		room.id = s.readUint32();
		s.read(&room.name, 8);
		room.scriptsOffset = s.readUint32();
		room.ambSoundsOffset = s.readUint32();
		room.unkOffset = s.readUint32();
		s.readUint32(); // The zip-bit index is computed at runtime
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
	} else if (_vm->getPlatform() == Common::kPlatformWindows && _vm->getDefaultLanguage() == Common::RU_RUS) {
		stream = extractRussianM3R(stream);
	} else if (_executableVersion->safeDiskKey) {
#ifdef USE_SAFEDISC
		SafeDisc sd;
		sd.setKey(_executableVersion->safeDiskKey);
		sd.setDecodingFunctions(&safeDiscDecode1, &safeDiscDecode2);

		Common::SeekableReadStream *decrypted = sd.decrypt(stream);
		delete stream;

		if (!decrypted) {
			error("Error while decrypting SafeDisc executable");
		}

		stream = decrypted;
#else

		error("Safedisc decryption is not enabled.");

#endif // USE_SAFEDISC
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


/**
 * This class performs NRV 2d decompression
 *
 * The implementation is based upon the GPLv2 licensed UCL library
 * from http://www.oberhumer.com/opensource/ucl/
 */
class NRV2D {
public :
	static void uncompress(Common::SeekableReadStream *src, byte *dst) {
		uint bc = 0;
		uint32 bb = 0;
		uint olen = 0, last_m_off = 1;

		for (;;) {
			uint32 m_off, m_len;

			while (getBit(bb, bc, src)) {
				dst[olen++] = src->readByte();
			}

			m_off = 1;
			for (;;) {
				m_off = m_off * 2 + getBit(bb, bc, src);

				if (getBit(bb, bc, src))
					break;

				m_off = (m_off - 1) * 2 + getBit(bb, bc, src);
			}

			if (m_off == 2) {
				m_off = last_m_off;
				m_len = getBit(bb, bc, src);
			} else {
				m_off = (m_off - 3) * 256 + src->readByte();

				if (m_off == 0xFFFFFFFF)
					break;

				m_len = (m_off ^ 0xFFFFFFFF) & 1;
				m_off >>= 1;
				last_m_off = ++m_off;
			}

			m_len = m_len * 2 + getBit(bb, bc, src);
			if (m_len == 0) {
				m_len++;
				do {
					m_len = m_len * 2 + getBit(bb, bc, src);
				} while (!getBit(bb, bc, src));
				m_len += 2;
			}

			m_len += (m_off > 0x500);

			{
				const byte *m_pos = dst + olen - m_off;

				dst[olen++] = *m_pos++;
				do
					dst[olen++] = *m_pos++;
				while (--m_len > 0);
			}
		}
	}

private:
	static uint getBit(uint32 &bb, uint &bc, Common::SeekableReadStream *src) {
		if (bc == 0) {
			bc = 32;
			bb = src->readUint32LE();
		}
		return (bb >> --bc) & 1;
	}
};

Common::SeekableReadStream *Database::extractRussianM3R(Common::SeekableReadStream *stream) const {
	/*
	 * The Russian version comes with a weird packaging. It doesn't have the
	 * usual M3.exe but instead comes with M3R.exe and inst.dat.
	 *
	 * inst.dat is an UPX executable with the compressed section removed.
	 *
	 * M3R.exe is a Delphi programmed launcher. Its "INFO" resource contains
	 * the missing section from inst.dat
	 *
	 * Here is how the original works :
	 * - M3R.exe loads inst.dat in memory as an executable.
	 * - MR3.exe extracts the INFO resource from itself,
	 * 		and uses a very simple patch mechanism to modify the in-memory
	 * 		representation of inst.dat.
	 * - At this point, the inst.dat in memory representation is a hacked up UPX
	 * 		compressed M3.exe. MR3.exe resumes execution of the compressed M3.exe.
	 * - M3.exe uncompresses itself in memory using the NRV algorithm.
	 * - The game starts.
	 */

	// Extract the INFO resource from M3R.exe
	Common::PEResources m3r;
	m3r.loadFromEXE(stream);
	Common::SeekableReadStream *compressed = m3r.getResource(Common::kPERCData, Common::String("INFO"));

	// Skip the header
	compressed->skip(4); // Patch offset
	compressed->skip(4); // Patch size

	// The uncompressed size is not stored anywhere, just allocate slightly
	// more memory than actually needed
	byte *data = (byte *)malloc(1100*1024);

	// Perform the decompression
	NRV2D::uncompress(compressed, data);

	delete compressed;

	return new Common::MemoryReadStream(data, 2*1024*1024, DisposeAfterUse::YES);
}

#define ROL32(x,b) (((x) << (b)) | ((x) >> (32 - (b))))
#define ROR32(x,b) (((x) >> (b)) | ((x) << (32 - (b))))

uint32 Database::safeDiscDecode1(uint32 data) {
	data += 0x15770916;
	data ^= 0x13932106;
	data = ROL32(data, 0xE2 % 32);
	data -= 0x407A1EF5;
	data ^= 0x33784C64;
	data -= 0x7BA359F3;
	data = ROR32(data, 0xB1 % 32);
	data += 0x0F5123F;
//	data = ROR32(data, 0xE0 % 32); NOP
	data += 0x50C52C7;
	data -= 0x29256E14;
	data ^= 0x64B95579;
	data = ROL32(data, 0xC7 % 32);
	data -= 0x4CF5171E;
	data ^= 0x5A4D4CF1;
	data = ROR32(data, 0x73 % 32);
	data = ROL32(data, 0xE3 % 32);
	data += 0x2D415B9B;
	data = ROL32(data, 0x04 % 32);
	data = ROL32(data, 0xC6 % 32);
	data = ROL32(data, 0xD6 % 32);
	data += 0x56A97DBD;
	data += 0x714738B4;

	return data;
}

uint32 Database::safeDiscDecode2(uint32 data) {
	data--;
	data++;
	data = ROR32(data, 0xD2 % 32);
	data = ROL32(data, 0xF3 % 32);
	data = ROR32(data, 0x25 % 32);
	data = ROL32(data, 0xDE % 32);
	data++;
	data = ROL32(data, 0xFF % 32);
	data = ROR32(data, 0x6B % 32);
	data--;
	data = ROR32(data, 0x03 % 32);
	data++;
	data += 0x3824412A;
	data -= 0x5C8137F7;
	data++;
	data -= 0x13932106;
	data--;
	data--;
	data += 0x4C645056;
	data = ROL32(data, 0xCC % 32);
	data = ROL32(data, 0xB2 % 32);
	data--;
//	data = ROR32(data, 0xE0 % 32); NOP
	data--;
	data = ROR32(data, 0x14 % 32);
	data = -data;
	data = ROL32(data, 0xC7 % 32);
	data -= 0x4CF5171E;
	data--;
	data = ROL32(data, 0x73 % 32);
	data++;
	data++;
	data++;
	data += 0x42C270B1;
	data += 0x49A602E5;
	data = ROL32(data, 0xA9 % 32);
	data -= 0x714738B4;

	return data;
}

#undef ROL32
#undef ROR32

void Database::loadSoundNames(Common::ReadStreamEndian *s) {
	_soundNames.clear();

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
	const Common::String result = _soundNames.getVal(id, "");
	if (result.empty())
		error("Unable to find a sound with id %d", id);

	return result;
}

void Database::loadAmbientCues(Common::ReadStreamEndian *s) {
	_ambientCues.clear();

	while (1) {
		uint16 id = s->readUint16();

		if (!id)
			break;

		AmbientCue cue;
		cue.id = id;
		cue.minFrames = s->readUint16();
		cue.maxFrames = s->readUint16();

		while (1) {
			uint16 track = s->readUint16();

			if (!track)
				break;

			cue.tracks.push_back(track);
		}

		_ambientCues[id] = cue;
	}
}

const AmbientCue& Database::getAmbientCue(uint16 id) {
	if (!_ambientCues.contains(id))
		error("Unable to find an ambient cue with id %d", id);

	return _ambientCues.getVal(id);
}

#ifdef USE_SAFEDISC

SafeDisc::SafeDisc() {
	_decode1 = 0;
	_decode2 = 0;
}

void SafeDisc::setKey(const SafeDiskKey *key) {
	for (uint i = 0; i < 4; i++) {
		_key[i] = (*key)[i];
	}
}

void SafeDisc::setDecodingFunctions(DecodeFunc f1, DecodeFunc f2) {
	_decode1 = f1;
	_decode2 = f2;
}

Common::SeekableReadStream *SafeDisc::decrypt(Common::SeekableReadStream *stream) const {
	if (!stream)
		return 0;

	// Check executable format
	if (stream->readUint16BE() != MKTAG16('M', 'Z')) {
		error("Not a valid Windows executable");
	}

	stream->skip(58);

	uint32 peOffset = stream->readUint32LE();

	if (!peOffset || peOffset >= (uint32)stream->size())
		return 0;

	stream->seek(peOffset);

	if (stream->readUint32BE() != MKTAG('P','E',0,0)) {
		error("Not a valid Windows executable");
	}

	stream->skip(2);
	uint16 sectionCount = stream->readUint16LE();
	stream->skip(12);
	uint16 optionalHeaderSize = stream->readUint16LE();
	stream->skip(optionalHeaderSize + 2);

	Common::HashMap<Common::String, Section, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> sections;

	// Read in all the sections
	for (uint16 i = 0; i < sectionCount; i++) {
		char sectionName[9];
		stream->read(sectionName, 8);
		sectionName[8] = 0;

		Section section;
		stream->skip(4);
		section.virtualAddress = stream->readUint32LE();
		section.size = stream->readUint32LE();
		section.offset = stream->readUint32LE();
		stream->skip(16);

		sections[sectionName] = section;
	}

	//Check sections
	static const char* sectionsToDecrypt[] = { ".text", ".data" };
	for (uint i = 0; i < ARRAYSIZE(sectionsToDecrypt); i++) {
		if (!sections.contains(sectionsToDecrypt[i])) {
			error("Executable does not contain required section '%s'", sectionsToDecrypt[i]);
		}
	}

	// Read SafeDisk version
	Section &firstSection = sections[".text"];
	stream->seek(firstSection.offset - 3 * sizeof(uint32));
	uint32 maj = stream->readUint32LE();
	uint32 min = stream->readUint32LE();
	uint32 rev = stream->readUint32LE();
	Common::String version = Common::String::format("v%d.%d.%d", maj, min, rev);

	// Check SafeDisc version
	bool known = false;
	static const char* knownVersions[] = { "v2.10.30", "v2.30.31", "v2.40.10" };
	for (uint i = 0; i < ARRAYSIZE(knownVersions); i++) {
		if (version.equals(knownVersions[i])) {
			known = true;
			break;
		}
	}

	if (!known) {
		error("Unknown SafeDisk version %s", version.c_str());
	}

	// Initialize decrypted data buffer
	uint32 dataSize = stream->size();
	uint8 *data = new uint8[dataSize];
	stream->seek(0);
	stream->read(data, dataSize);

	static const uint32 blockSize = 0x1000;

	// Decrypt sections
	for (uint i = 0; i < ARRAYSIZE(sectionsToDecrypt); i++) {
		Section &section = sections[sectionsToDecrypt[i]];

		// To ensure we can decrypt whole blocks
		assert(section.size % blockSize == 0);
		assert(section.offset + section.size < dataSize);

		// Decrypt the section, by blocks
		uint32 done = 0;
		while (done < section.size) {
			uint32 *block = (uint32*) &data[section.offset + done];
			done += blockSize;

			decryptBlock(block, blockSize >> 2);
		}
	}

	return new Common::MemoryReadStream(data, dataSize, DisposeAfterUse::YES);
}

void SafeDisc::decryptBlock(uint32 *buffer, uint32 size) const {
	assert(_decode1 && _decode2);

	// Simple data modification decoding
	for (uint32 i = 0; i < size; i++) {
		uint32 data = buffer[i];
		data ^= (i << 24 | i << 16 | i << 8 | i);
		buffer[i] = _decode1(data);
	}

	// Another simple data modification decoding
	for (uint32 i = 0; i < size; i++) {
		uint32 data = buffer[i];
		data ^= (i << 24 | i << 16 | i << 8 | i);
		buffer[i] = _decode2(data);
	}

	// Tiny Encryption Algorithm decryption
	static const uint32 teaMagic = 0x9E3779B9;
	for (uint32 j = 0; j < size; j += 2) {
		uint32 data1 = buffer[j    ];
		uint32 data2 = buffer[j + 1];

		uint32 sum = 32 * teaMagic;
		for (uint32 i = 0; i < 32; i++) {
			data2 -= (_key[3] + (data1 >> 5)) ^ (sum + data1) ^ (_key[2] + (data1 << 4));
			data1 -= (_key[1] + (data2 >> 5)) ^ (sum + data2) ^ (_key[0] + (data2 << 4));

			sum -= teaMagic;
		}

		buffer[j    ] = data1;
		buffer[j + 1] = data2;
	}
}

#endif // USE_SAFEDISC

} // End of namespace Myst3
