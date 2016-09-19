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
#include "common/substream.h"

namespace Myst3 {

static const RoomData roomsXXXX[] = {
		{ 101, "XXXX" }
};

static const RoomData roomsINTR[] = {
		{ 201, "INTR" }
};

static const RoomData roomsTOHO[] = {
		{ 301, "TOHO" }
};

static const RoomData roomsTOHB[] = {
		{ 401, "TOHB" }
};

static const RoomData roomsLE[] = {
		{ 501, "LEIS" },
		{ 502, "LEOS" },
		{ 503, "LEET" },
		{ 504, "LELT" },
		{ 505, "LEMT" },
		{ 506, "LEOF" }
};

static const RoomData roomsLI[] = {
		{ 601, "LIDR" },
		{ 602, "LISW" },
		{ 603, "LIFO" },
		{ 604, "LISP" },
		{ 605, "LINE" }
};

static const RoomData roomsEN[] = {
		{ 701, "ENSI" },
		{ 703, "ENPP" },
		{ 704, "ENEM" },
		{ 705, "ENLC" },
		{ 706, "ENDD" },
		{ 707, "ENCH" },
		{ 708, "ENLI" }
};

static const RoomData roomsNA[] = {
		{ 801, "NACH" }
};

static const RoomData roomsMENU[] = {
		{ 901, "MENU" },
		{ 902, "JRNL" },
		{ 903, "DEMO" },
		{ 904, "ATIX" }
};

static const RoomData roomsMA[] = {
		{ 1001, "MACA" },
		{ 1002, "MAIS" },
		{ 1003, "MALL" },
		{ 1004, "MASS" },
		{ 1005, "MAWW" },
		{ 1006, "MATO" }
};

static const RoomData roomsLOGO[] = {
		{ 1101, "LOGO" }
};

const AgeData Database::_ages[] = {
	{ 1, 0, 1, roomsXXXX, 0 },
	{ 2, 1, 1, roomsINTR, 0 },
	{ 3, 2, 1, roomsTOHO, 0 },
	{ 4, 4, 1, roomsTOHB, 0 },
	{ 5, 2, 6, roomsLE, 1 },
	{ 6, 4, 5, roomsLI, 2 },
	{ 7, 3, 7, roomsEN, 3 },
	{ 8, 3, 1, roomsNA, 4 },
	{ 9, 0, 4, roomsMENU, 0 },
	{ 10, 1, 6, roomsMA, 5 },
	{ 11, 0, 1, roomsLOGO, 0 }
};

Database::Database(Myst3Engine *vm) :
		_vm(vm),
		_currentRoomID(0),
		_currentRoomData(0) {

	_datFile = SearchMan.createReadStreamForMember("myst3.dat");
	if (!_datFile) {
		error("Unable to find 'myst3.dat'");
	}

	uint magic = _datFile->readUint32LE();
	if (magic != MKTAG('M', 'Y', 'S', 'T')) {
		error("'myst3.dat' is invalid");
	}

	uint version = _datFile->readUint32LE();
	if (version != kDatVersion) {
		error("Incorrect 'myst3.dat' version. Expected '%d', found '%d'", kDatVersion, version);
	}

	bool isWindowMacVersion = _vm->getPlatform() == Common::kPlatformWindows || _vm->getPlatform() == Common::kPlatformMacintosh;
	bool isXboxVersion = _vm->getPlatform() == Common::kPlatformXbox;

	readScriptIndex(_datFile, isWindowMacVersion);                                                  // Main scripts
	readScriptIndex(_datFile, isWindowMacVersion && _vm->isDVDVersion());                           // Menu scripts DVD version
	readScriptIndex(_datFile, isWindowMacVersion && !_vm->isDVDVersion() && !_vm->isMonolingual()); // Menu scripts international CD version
	readScriptIndex(_datFile, isWindowMacVersion && !_vm->isDVDVersion() && _vm->isMonolingual());  // Menu scripts english CD version
	readScriptIndex(_datFile, isXboxVersion);                                                       // Main scripts Xbox version
	readScriptIndex(_datFile, isXboxVersion && !_vm->isMonolingual());                              // Menu scripts PAL Xbox version
	readScriptIndex(_datFile, isXboxVersion && _vm->isMonolingual());                               // Menu scripts NTSC Xbox version
	readSoundNames(_datFile, isWindowMacVersion);                                                   // Sound names
	readSoundNames(_datFile, isXboxVersion);                                                        // Sound names Xbox

	_roomScriptsStartOffset = _datFile->pos();

	Common::SeekableReadStream *initScriptStream = getRoomScriptStream("INIT", kScriptTypeNodeInit);
	_nodeInitScript = loadOpcodes(*initScriptStream);
	delete initScriptStream;

	Common::SeekableReadStream *cuesStream = getRoomScriptStream("INIT", kScriptTypeAmbientCue);
	loadAmbientCues(cuesStream);
	delete cuesStream;

	preloadCommonRooms();
	initializeZipBitIndexTable();

	if (isWindowMacVersion && !_vm->isDVDVersion() && !_vm->isMonolingual()) {
		patchLanguageMenu();
	}
}

Database::~Database() {
	delete _datFile;
}

void Database::preloadCommonRooms() {
	// XXXX, MENU, JRNL
	static const uint32 commonRooms[3] = { 101, 901, 902 };

	for (uint i = 0; i < 3; i++) {
		const RoomData *data = findRoomData(commonRooms[i]);
		_roomNodesCache.setVal(commonRooms[i], loadRoomScripts(data));
	}
}

Common::Array<NodePtr> Database::getRoomNodes(uint32 roomID) {
	Common::Array<NodePtr> nodes;

	if (_roomNodesCache.contains(roomID)) {
		nodes = _roomNodesCache.getVal(roomID);
	} else {
		const RoomData *data = findRoomData(roomID);
		nodes = loadRoomScripts(data);
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

void Database::initializeZipBitIndexTable() {
	int16 zipBit = 0;
	for (uint i = 0; i < ARRAYSIZE(_ages); i++) {
		for (uint j = 0; j < _ages[i].roomCount; j++) {
			_roomZipBitIndex.setVal(_ages[i].rooms[j].id, zipBit);

			// Add the highest zip-bit index for the current room
			// to get the zip-bit index for the next room
			int16 maxZipBitForRoom = 0;
			Common::Array<NodePtr> nodes = loadRoomScripts(&_ages[i].rooms[j]);
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

const RoomData *Database::findRoomData(uint32 roomID) {
	for (uint i = 0; i < ARRAYSIZE(_ages); i++) {
		for (uint j = 0; j < _ages[i].roomCount; j++) {
			if (_ages[i].rooms[j].id == roomID) {
				return &_ages[i].rooms[j];
			}
		}
	}

	error("No room with ID %d", roomID);
}

Common::Array<NodePtr> Database::loadRoomScripts(const RoomData *room) {
	Common::Array<NodePtr> nodes;

	// Load the node scripts
	Common::SeekableReadStream *scriptsStream = getRoomScriptStream(room->name, kScriptTypeNode);
	if (scriptsStream) {
		loadRoomNodeScripts(scriptsStream, nodes);
		delete scriptsStream;
	}

	// Load the ambient sound scripts, if any
	Common::SeekableReadStream *ambientSoundsStream = getRoomScriptStream(room->name, kScriptTypeAmbientSound);
	if (ambientSoundsStream) {
		loadRoomSoundScripts(ambientSoundsStream, nodes, false);
		delete ambientSoundsStream;
	}

	Common::SeekableReadStream *backgroundSoundsStream = getRoomScriptStream(room->name, kScriptTypeBackgroundSound);
	if (backgroundSoundsStream) {
		loadRoomSoundScripts(backgroundSoundsStream, nodes, true);
		delete backgroundSoundsStream;
	}

	return nodes;
}

void Database::loadRoomNodeScripts(Common::SeekableReadStream *file, Common::Array<NodePtr> &nodes) {
	uint zipIndex = 0;
	while (!file->eos()) {
		int16 id = file->readUint16LE();

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
				nodeIds.push_back(file->readUint16LE());
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

void Database::loadRoomSoundScripts(Common::SeekableReadStream *file, Common::Array<NodePtr> &nodes, bool background) {
	while (!file->eos()) {
		int16 id = file->readUint16LE();

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
					id = file->readUint16LE();
					if (id < 0) {
						uint16 end = file->readUint16LE();
						for (int i = -id; i < end; i++)
							nodeIds.push_back(i);

					} else if (id > 0) {
						nodeIds.push_back(id);
					}
				} while (id);
			else
				for (int i = 0; i < -id; i++) {
					nodeIds.push_back(file->readUint16LE());
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

	if (!_currentRoomData)
		return;

	// Remove old room from cache and add the new one
	_roomNodesCache.erase(_currentRoomID);
	_roomNodesCache.setVal(roomID, loadRoomScripts(_currentRoomData));

	_currentRoomID = roomID;
}

Common::Array<CondScript> Database::loadCondScripts(Common::SeekableReadStream &s) {
	Common::Array<CondScript> scripts;

	while (!s.eos()) {
		CondScript script = loadCondScript(s);

		if (!script.condition)
			break;

		scripts.push_back(script);
	}

	return scripts;
}

Common::Array<HotSpot> Database::loadHotspots(Common::SeekableReadStream &s) {
	Common::Array<HotSpot> scripts;

	while (!s.eos()) {
		HotSpot hotspot = loadHotspot(s);

		if (!hotspot.condition)
			break;

		scripts.push_back(hotspot);
	}

	return scripts;
}

Common::Array<Opcode> Database::loadOpcodes(Common::SeekableReadStream &s) {
	Common::Array<Opcode> script;

	while (!s.eos()) {
		Opcode opcode;
		uint16 code = s.readUint16LE();

		opcode.op = code & 0xff;
		uint8 count = code >> 8;
		if (count == 0 && opcode.op == 0)
			break;

		for (int i = 0; i < count; i++) {
			int16 value = s.readUint16LE();
			opcode.args.push_back(value);
		}

		script.push_back(opcode);
	}

	return script;
}

CondScript Database::loadCondScript(Common::SeekableReadStream &s) {
	CondScript script;
	script.condition = s.readUint16LE();
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

HotSpot Database::loadHotspot(Common::SeekableReadStream &s) {
	HotSpot hotspot;

	hotspot.condition = s.readUint16LE();

	if (hotspot.condition == 0)
		return hotspot;

	if (hotspot.condition != -1) {
		hotspot.rects = loadRects(s);
		hotspot.cursor = s.readUint16LE();
	}

	hotspot.script = loadOpcodes(s);

	return hotspot;
}

Common::Array<PolarRect> Database::loadRects(Common::SeekableReadStream &s) {
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
	} while (!lastRect && !s.eos());

	return rects;
}

Common::String Database::getRoomName(uint32 roomID) {
	if (roomID != 0 && roomID != _currentRoomID) {
		const RoomData *data = findRoomData(roomID);
		return data->name;
	} else if (_currentRoomData) {
		return _currentRoomData->name;
	}

	return "";
}

uint32 Database::getRoomId(const char *name) {
	for (uint i = 0; i < ARRAYSIZE(_ages); i++)
		for (uint j = 0; j < _ages[i].roomCount; j++) {
			if (scumm_stricmp(_ages[i].rooms[j].name, name) == 0) {
				return _ages[i].rooms[j].id;
			}
		}

	return 0;
}

uint32 Database::getAgeLabelId(uint32 ageID) {
	for (uint i = 0; i < ARRAYSIZE(_ages); i++)
		if (_ages[i].id == ageID)
			return _ages[i].labelId;

	return 0;
}

Common::String Database::getSoundName(uint32 id) {
	const Common::String result = _soundNames.getVal(id, "");
	if (result.empty())
		error("Unable to find a sound with id %d", id);

	return result;
}

void Database::loadAmbientCues(Common::ReadStream *s) {
	_ambientCues.clear();

	while (!s->eos()) {
		uint16 id = s->readUint16LE();

		if (!id)
			break;

		AmbientCue cue;
		cue.id = id;
		cue.minFrames = s->readUint16LE();
		cue.maxFrames = s->readUint16LE();

		while (1) {
			uint16 track = s->readUint16LE();

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

void Database::readScriptIndex(Common::SeekableReadStream *stream, bool load) {
	uint count = stream->readUint32LE();
	for (uint i = 0; i < count; i++) {

		RoomScripts roomScripts;

		char roomName[5];
		stream->read(roomName, sizeof(roomName));
		roomName[4] = '\0';

		roomScripts.room = Common::String(roomName);
		roomScripts.type = (ScriptType) stream->readUint32LE();
		roomScripts.offset = stream->readUint32LE();
		roomScripts.size = stream->readUint32LE();

		if (load) {
			_roomScriptsIndex.push_back(roomScripts);
		}
	}
}

void Database::readSoundNames(Common::SeekableReadStream *stream, bool load) {
	uint count = stream->readUint32LE();
	for (uint i = 0; i < count; i++) {

		uint id = stream->readUint32LE();

		char soundName[32];
		stream->read(soundName, sizeof(soundName));
		soundName[31] = '\0';

		if (load) {
			_soundNames[id] = Common::String(soundName);
		}
	}
}

Common::SeekableReadStream *Database::getRoomScriptStream(const char *room, ScriptType scriptType) const {
	for (uint i = 0; i < _roomScriptsIndex.size(); i++) {
		if (_roomScriptsIndex[i].room.equalsIgnoreCase(room)
		    && _roomScriptsIndex[i].type == scriptType) {
			uint32 startOffset = _roomScriptsStartOffset + _roomScriptsIndex[i].offset;
			uint32 size = _roomScriptsIndex[i].size;

			return new Common::SeekableSubReadStream(_datFile, startOffset, startOffset + size);
		}
	}

	return nullptr;
}

void Database::patchLanguageMenu() {
	// The menu scripts in 'myst3.dat" for the non English CD versions come from the French version
	// The scripts for the other languages only differ by the value set for AudioLanguage variable
	// when the language selection is not English.
	// This function patches the language selection script to set the appropriate value based
	// on the detected game langage.

	// Script disassembly:
	//	hotspot 5 > c[v1 != 0] (true)
	//	rect > pitch: 373 heading: 114 width: 209 height: 28
	//	op 206, soundPlayVolume ( 795 5 )
	//	op 53, varSetValue ( vLanguageAudio 2 ) // <= The second argument of this opcode is patched
	//	op 194, runPuzzle1 ( 18 )
	//	op 194, runPuzzle1 ( 19 )

	NodePtr languageMenu = getNodeData(530, 901, 9);
	languageMenu->hotspots[5].script[1].args[1] = _vm->getGameLanguageCode();
}

} // End of namespace Myst3
