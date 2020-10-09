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

#include "common/archive.h"
#include "common/debug.h"
#include "common/substream.h"

namespace Myst3 {

/**
 * An abstract node transformation.
 *
 * Subclasses can read data related to a node from a stream in read,
 * and transform a node using that data in apply.
 */
class NodeTransform {
public :
	virtual ~NodeTransform() {};

	virtual void read(Common::SeekableReadStream *file) = 0;
	virtual void apply(NodePtr &node) = 0;
};

/**
 * A node transformation that reads hotspots and scripts,
 * and adds them to a node.
 */
class NodeTransformAddHotspots : public NodeTransform {
public :
	NodeTransformAddHotspots();

	void read(Common::SeekableReadStream *file);
	void apply(NodePtr &node);

private:
	int32 _zipBitIndex;
	Common::Array<CondScript> _scripts;
	Common::Array<HotSpot> _hotspots;
};

/**
 * A node transformation that reads scripts, and adds them to a node's
 * sound scripts.
 */
class NodeTransformAddSoundScripts : public NodeTransform {
public :
	void read(Common::SeekableReadStream *file) override;
	void apply(NodePtr &node) override;

private:
	Common::Array<CondScript> _scripts;
};

/**
 * A node transformation that reads scripts, and adds them to a node's
 * background sound scripts.
 */
class NodeTransformAddBackgroundSoundScripts : public NodeTransform {
public :
	void read(Common::SeekableReadStream *file) override;
	void apply(NodePtr &node) override;

private:
	Common::Array<CondScript> _scripts;
};

/**
 * Walks through a stream of nodes. For each encountered node, a NodeTransformer is called.
 */
class NodeWalker {
public :
	NodeWalker(NodeTransform *transform);
	~NodeWalker();

	void read(Common::SeekableReadStream *file, Common::Array<NodePtr> &allNodes, bool createMissingSharedNodes);

private:
	NodeTransform *_transform;
};

/**
 * A collection of functions used to read script related data
 */
class ScriptData {
public:
	static Common::Array<CondScript> readCondScripts(Common::SeekableReadStream &s);
	static Common::Array<Opcode> readOpcodes(Common::ReadStream &s);
	static Common::Array<HotSpot> readHotspots(Common::ReadStream &s);
	static Common::Array<PolarRect> readRects(Common::ReadStream &s);
	static CondScript readCondScript(Common::SeekableReadStream &s);
	static HotSpot readHotspot(Common::ReadStream &s);

private:
	ScriptData() {};
};

Common::Array<PolarRect> ScriptData::readRects(Common::ReadStream &s) {
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

Common::Array<Opcode> ScriptData::readOpcodes(Common::ReadStream &s) {
	Common::Array<Opcode> script;

	while (!s.eos()) {
		Opcode opcode;
		uint16 code = s.readUint16LE();

		opcode.op = code & 0xff;
		uint8 count = code >> 8;
		if (count == 0 && opcode.op == 0)
			break;

		for (int i = 0; i < count; i++) {
			int16 value = s.readSint16LE();
			opcode.args.push_back(value);
		}

		script.push_back(opcode);
	}

	return script;
}

CondScript ScriptData::readCondScript(Common::SeekableReadStream &s) {
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

	script.script = readOpcodes(s);

	return script;
}

Common::Array<CondScript> ScriptData::readCondScripts(Common::SeekableReadStream &s) {
	Common::Array<CondScript> scripts;

	while (!s.eos()) {
		CondScript script = readCondScript(s);

		if (!script.condition)
			break;

		scripts.push_back(script);
	}

	return scripts;
}

HotSpot ScriptData::readHotspot(Common::ReadStream &s) {
	HotSpot hotspot;

	hotspot.condition = s.readUint16LE();

	if (hotspot.condition == 0)
		return hotspot;

	if (hotspot.condition != -1) {
		hotspot.rects = readRects(s);
		hotspot.cursor = s.readUint16LE();
	}

	hotspot.script = readOpcodes(s);

	return hotspot;
}

Common::Array<HotSpot> ScriptData::readHotspots(Common::ReadStream &s) {
	Common::Array<HotSpot> scripts;

	while (!s.eos()) {
		HotSpot hotspot = readHotspot(s);

		if (!hotspot.condition)
			break;

		scripts.push_back(hotspot);
	}

	return scripts;
}

NodeTransformAddHotspots::NodeTransformAddHotspots() : _zipBitIndex(-1) {
}

void NodeTransformAddHotspots::read(Common::SeekableReadStream *file) {
	_zipBitIndex++;
	_scripts = ScriptData::readCondScripts(*file);
	_hotspots = ScriptData::readHotspots(*file);
}

void NodeTransformAddHotspots::apply(NodePtr &node) {
	node->zipBitIndex = _zipBitIndex;
	node->scripts.push_back(_scripts);
	node->hotspots.push_back(_hotspots);
}

void NodeTransformAddSoundScripts::read(Common::SeekableReadStream *file) {
	_scripts = ScriptData::readCondScripts(*file);
}

void NodeTransformAddSoundScripts::apply(NodePtr &node) {
	node->soundScripts.push_back(_scripts);
}

void NodeTransformAddBackgroundSoundScripts::read(Common::SeekableReadStream *file) {
	_scripts = ScriptData::readCondScripts(*file);
}

void NodeTransformAddBackgroundSoundScripts::apply(NodePtr &node) {
	node->backgroundSoundScripts.push_back(_scripts);
}

NodeWalker::NodeWalker(NodeTransform *transform) : _transform(transform) {
}

void NodeWalker::read(Common::SeekableReadStream *file, Common::Array<NodePtr> &allNodes, bool createMissingSharedNodes) {
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
			for (uint i = 0; i < allNodes.size(); i++)
				if (allNodes[i]->id == id) {
					node = allNodes[i];
					break;
				}

			// Node not found, create a new one
			if (!node) {
				node = NodePtr(new NodeData());
				node->id = id;
				allNodes.push_back(node);
			}

			_transform->read(file);
			_transform->apply(node);
		} else {
			// Several nodes sharing the same scripts
			// Find the node ids the script applies to
			Common::Array<int16> scriptNodeIds;

			if (id == -10)
				do {
					id = file->readUint16LE();
					if (id < 0) {
						uint16 end = file->readUint16LE();
						for (int i = -id; i <= end; i++)
							scriptNodeIds.push_back(i);

					} else if (id > 0) {
						scriptNodeIds.push_back(id);
					}
				} while (id);
			else
				for (int i = 0; i < -id; i++) {
					scriptNodeIds.push_back(file->readUint16LE());
				}

			// Load the script
			_transform->read(file);

			// Add the script to each matching node
			for (uint i = 0; i < scriptNodeIds.size(); i++) {
				NodePtr node;

				// Find the current node if existing
				for (uint j = 0; j < allNodes.size(); j++) {
					if (allNodes[j]->id == scriptNodeIds[i]) {
						node = allNodes[j];
						break;
					}
				}

				if (!node) {
					if (createMissingSharedNodes) {
						// Node not found, create a new one
						node = NodePtr(new NodeData());
						node->id = scriptNodeIds[i];
						allNodes.push_back(node);
					} else {
						// Node not found, skip it
						continue;
					}
				}

				_transform->apply(node);
			}
		}
	}
}

NodeWalker::~NodeWalker() {
	delete _transform;
}

static const RoomData roomsXXXX[] = {
		{ kRoomShared,        "XXXX" }
};

static const RoomData roomsINTR[] = {
		{ kRoomIntro,         "INTR" }
};

static const RoomData roomsTOHO[] = {
		{ kRoomTomahnaStart,  "TOHO" }
};

static const RoomData roomsTOHB[] = {
		{ kRoomTomahnaReturn, "TOHB" }
};

static const RoomData roomsLE[] = {
		{ kJnaninStart,       "LEIS" },
		{ kRoomLeos,          "LEOS" },
		{ kRoomLeet,          "LEET" },
		{ kRoomLelt,          "LELT" },
		{ kRoomLemt,          "LEMT" },
		{ kRoomLeof,          "LEOF" }
};

static const RoomData roomsLI[] = {
		{ kRoomEdannaStart,   "LIDR" },
		{ kRoomLisw,          "LISW" },
		{ kRoomLifo,          "LIFO" },
		{ kRoomLisp,          "LISP" },
		{ kRoomLine,          "LINE" }
};

static const RoomData roomsEN[] = {
		{ kRoomVoltaicStart,  "ENSI" },
		{ kRoomEnpp,          "ENPP" },
		{ kRoomEnem,          "ENEM" },
		{ kRoomEnlc,          "ENLC" },
		{ kRoomEndd,          "ENDD" },
		{ kRoomEnch,          "ENCH" },
		{ kRoomEnli,          "ENLI" }
};

static const RoomData roomsNA[] = {
		{ kRoomNarayan,       "NACH" }
};

static const RoomData roomsMENU[] = {
		{ kRoomMenu,          "MENU" },
		{ kRoomJournals,      "JRNL" },
		{ kRoomDemo,          "DEMO" },
		{ kRoomAtix,          "ATIX" }
};

static const RoomData roomsMA[] = {
		{ kRoomAmateriaStart, "MACA" },
		{ kRoomMais,          "MAIS" },
		{ kRoomMall,          "MALL" },
		{ kRoomMass,          "MASS" },
		{ kRoomMaww,          "MAWW" },
		{ kRoomMato,          "MATO" }
};

static const RoomData roomsLOGO[] = {
		{ kLogo,              "LOGO" }
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

Database::Database(const Common::Platform platform, const Common::Language language, const uint32 localizationType) :
		_platform(platform),
		_language(language),
		_localizationType(localizationType),
		_soundIdMin(0),
		_soundIdMax(0) {

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

	bool isWindowMacVersion = _platform == Common::kPlatformWindows || _platform == Common::kPlatformMacintosh;
	bool isXboxVersion = _platform == Common::kPlatformXbox;

	readScriptIndex(_datFile, isWindowMacVersion);                                          // Main scripts
	readScriptIndex(_datFile, isWindowMacVersion && _localizationType == kLocMulti6);      // Menu scripts 6 languages version
	readScriptIndex(_datFile, isWindowMacVersion && _localizationType == kLocMulti2);      // Menu scripts 2 languages CD version
	readScriptIndex(_datFile, isWindowMacVersion && _localizationType == kLocMonolingual); // Menu scripts english CD version
	readScriptIndex(_datFile, isXboxVersion);                                               // Main scripts Xbox version
	readScriptIndex(_datFile, isXboxVersion && _localizationType != kLocMonolingual);      // Menu scripts PAL Xbox version
	readScriptIndex(_datFile, isXboxVersion && _localizationType == kLocMonolingual);      // Menu scripts NTSC Xbox version
	readSoundNames(_datFile, isWindowMacVersion);                                           // Sound names
	readSoundNames(_datFile, isXboxVersion);                                                // Sound names Xbox

	_roomScriptsStartOffset = _datFile->pos();

	Common::SeekableReadStream *initScriptStream = getRoomScriptStream("INIT", kScriptTypeNodeInit);
	_nodeInitScript = ScriptData::readOpcodes(*initScriptStream);
	delete initScriptStream;

	Common::SeekableReadStream *cuesStream = getRoomScriptStream("INIT", kScriptTypeAmbientCue);
	loadAmbientCues(cuesStream);
	delete cuesStream;

	preloadCommonRooms();
	initializeZipBitIndexTable();

	if (isWindowMacVersion && _localizationType == kLocMulti2) {
		patchLanguageMenu();
	}
}

Database::~Database() {
	delete _datFile;
}

void Database::preloadCommonRooms() {
	for (uint i = 0; i < ARRAYSIZE(_ages); i++) {
		const AgeData &age = _ages[i];

		for (uint j = 0; j < age.roomCount; j++) {
			const RoomData &room = age.rooms[j];

			if (isCommonRoom(room.id, age.id)) {
				Common::Array<NodePtr> nodes = readRoomScripts(&room);
				_roomNodesCache.setVal(RoomKey(room.id, age.id), nodes);
			}
		}
	}
}

Common::Array<NodePtr> Database::getRoomNodes(uint32 roomID, uint32 ageID) const {
	Common::Array<NodePtr> nodes;

	if (_roomNodesCache.contains(RoomKey(roomID, ageID))) {
		nodes = _roomNodesCache.getVal(RoomKey(roomID, ageID));
	} else {
		const RoomData *data = findRoomData(roomID, ageID);
		nodes = readRoomScripts(data);
	}

	return nodes;
}

Common::Array<uint16> Database::listRoomNodes(uint32 roomID, uint32 ageID) {
	Common::Array<NodePtr> nodes;
	Common::Array<uint16> list;

	nodes = getRoomNodes(roomID, ageID);

	for (uint i = 0; i < nodes.size(); i++) {
		list.push_back(nodes[i]->id);
	}

	return list;
}

NodePtr Database::getNodeData(uint16 nodeID, uint32 roomID, uint32 ageID) {
	Common::Array<NodePtr> nodes = getRoomNodes(roomID, ageID);

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
			Common::Array<NodePtr> nodes = readRoomScripts(&_ages[i].rooms[j]);
			for (uint k = 0; k < nodes.size(); k++) {
				maxZipBitForRoom = MAX(maxZipBitForRoom, nodes[k]->zipBitIndex);
			}

			zipBit += maxZipBitForRoom + 1;
		}
	}
}

int32 Database::getNodeZipBitIndex(uint16 nodeID, uint32 roomID, uint32 ageID) {
	if (!_roomZipBitIndex.contains(roomID)) {
		error("Unable to find zip-bit index for room %d", roomID);
	}

	Common::Array<NodePtr> nodes = getRoomNodes(roomID, ageID);

	for (uint i = 0; i < nodes.size(); i++) {
		if (nodes[i]->id == nodeID) {
			return _roomZipBitIndex[roomID] + nodes[i]->zipBitIndex;
		}
	}

	error("Unable to find zip-bit index for node (%d, %d)", nodeID, roomID);
}

const RoomData *Database::findRoomData(uint32 roomID, uint32 ageID) const {
	for (uint i = 0; i < ARRAYSIZE(_ages); i++) {
		if (_ages[i].id == ageID) {
			for (uint j = 0; j < _ages[i].roomCount; j++) {
				if (_ages[i].rooms[j].id == roomID) {
					return &_ages[i].rooms[j];
				}
			}
		}
	}

	error("No room with ID %d in age %d", roomID, ageID);
}

Common::Array<NodePtr> Database::readRoomScripts(const RoomData *room) const {
	Common::Array<NodePtr> nodes;

	// Load the node scripts
	Common::SeekableReadStream *scriptsStream = getRoomScriptStream(room->name, kScriptTypeNode);
	if (scriptsStream) {
		NodeWalker scriptWalker = NodeWalker(new NodeTransformAddHotspots());
		scriptWalker.read(scriptsStream, nodes, true);

		delete scriptsStream;
	}

	// Load the ambient sound scripts, if any
	Common::SeekableReadStream *ambientSoundsStream = getRoomScriptStream(room->name, kScriptTypeAmbientSound);
	if (ambientSoundsStream) {
		NodeWalker scriptWalker = NodeWalker(new NodeTransformAddSoundScripts());
		scriptWalker.read(ambientSoundsStream, nodes, false);

		delete ambientSoundsStream;
	}

	Common::SeekableReadStream *backgroundSoundsStream = getRoomScriptStream(room->name, kScriptTypeBackgroundSound);
	if (backgroundSoundsStream) {
		NodeWalker scriptWalker = NodeWalker(new NodeTransformAddBackgroundSoundScripts());
		scriptWalker.read(backgroundSoundsStream, nodes, false);

		delete backgroundSoundsStream;
	}

	patchNodeScripts(room, nodes);

	return nodes;
}

void Database::patchNodeScripts(const RoomData *room, Common::Array<NodePtr> &nodes) const {
	if (strcmp(room->name, "LEOF") == 0) {
		// The room LEOF does not have a script to set default water effect
		// parameters when entering a node. As a result, the pool of water
		// mainly visible in LEOF 23 uses the last set water effect parameters.
		// If the player comes from the top of the tower, the water effect is
		// barely visible.
		// As a workaround we insert default water effect settings in node
		// 32765 which get applied for each node in the room.
		// The new script disassembles as follow:

		// node: LEOF 32765
		// init 0 > c[v1 != 0] (true)
		// op 33, waterEffectSetWave ( 100 100 )
		// op 32, waterEffectSetAttenuation ( 360 )
		// op 31, waterEffectSetSpeed ( 12 )

		Opcode waterEffectSetWave;
		waterEffectSetWave.op = 33;
		waterEffectSetWave.args.push_back(100);
		waterEffectSetWave.args.push_back(100);

		Opcode waterEffectSetAttenuation;
		waterEffectSetAttenuation.op = 32;
		waterEffectSetAttenuation.args.push_back(360);

		Opcode waterEffectSetSpeed;
		waterEffectSetSpeed.op = 31;
		waterEffectSetSpeed.args.push_back(12);

		CondScript waterEffectScript;
		waterEffectScript.condition = 1;
		waterEffectScript.script.push_back(waterEffectSetWave);
		waterEffectScript.script.push_back(waterEffectSetAttenuation);
		waterEffectScript.script.push_back(waterEffectSetSpeed);

		NodePtr node32765 = NodePtr(new NodeData());
		node32765->id = 32765;
		node32765->scripts.push_back(waterEffectScript);

		nodes.push_back(node32765);
	}
}

bool Database::isCommonRoom(uint32 roomID, uint32 ageID) const {
	return roomID == kRoomShared || roomID == kRoomMenu || roomID == kRoomJournals;
}

void Database::cacheRoom(uint32 roomID, uint32 ageID) {
	if (_roomNodesCache.contains(RoomKey(roomID, ageID))) {
		return;
	}

	// Remove old rooms from cache and add the new one
	for (NodesCache::iterator it = _roomNodesCache.begin(); it != _roomNodesCache.end(); it++) {
		if (!isCommonRoom(it->_key.roomID, it->_key.ageID)) {
			_roomNodesCache.erase(it);
		}
	}

	const RoomData *currentRoomData = findRoomData(roomID, ageID);

	if (!currentRoomData)
		return;

	_roomNodesCache.setVal(RoomKey(roomID, ageID), readRoomScripts(currentRoomData));
}

Common::String Database::getRoomName(uint32 roomID, uint32 ageID) const {
	const RoomData *data = findRoomData(roomID, ageID);
	return data->name;
}

RoomKey Database::getRoomKey(const char *name) {
	for (uint i = 0; i < ARRAYSIZE(_ages); i++)
		for (uint j = 0; j < _ages[i].roomCount; j++) {
			if (scumm_stricmp(_ages[i].rooms[j].name, name) == 0) {
				return RoomKey(_ages[i].rooms[j].id, _ages[i].id);
			}
		}

	return RoomKey(0, 0);
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

const AmbientCue &Database::getAmbientCue(uint16 id) {
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

			if (_soundIdMin == 0 || id < _soundIdMin) {
				_soundIdMin = id;
			}

			if (_soundIdMax == 0 || id > _soundIdMax) {
				_soundIdMax = id;
			}
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

bool Database::areRoomsScriptsEqual(uint32 roomID1, uint32 ageID1, uint32 roomID2, uint32 ageID2, ScriptType scriptType) {
	const RoomData *data1 = findRoomData(roomID1, ageID1);
	const RoomData *data2 = findRoomData(roomID2, ageID2);

	int32 startOffset1 = -1;
	int32 startOffset2 = -1;
	for (uint i = 0; i < _roomScriptsIndex.size(); i++) {
		if (_roomScriptsIndex[i].room.equalsIgnoreCase(data1->name)
		    && _roomScriptsIndex[i].type == scriptType) {
			startOffset1 = _roomScriptsStartOffset + _roomScriptsIndex[i].offset;
		}

		if (_roomScriptsIndex[i].room.equalsIgnoreCase(data2->name)
		    && _roomScriptsIndex[i].type == scriptType) {
			startOffset2 = _roomScriptsStartOffset + _roomScriptsIndex[i].offset;
		}
	}

	return startOffset1 == startOffset2;
}

int16 Database::getGameLanguageCode() const {
	// The monolingual versions of the game always use 0 as the language code
	if (_localizationType == kLocMonolingual) {
		return kEnglish;
	}

	switch (_language) {
		case Common::FR_FRA:
			return kFrench;
		case Common::DE_DEU:
			return kGerman;
		case Common::IT_ITA:
			return kItalian;
		case Common::ES_ESP:
			return kSpanish;
		case Common::EN_ANY:
			return kEnglish;
		default:
			return kOther;
	}
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

	NodePtr languageMenu = getNodeData(530, kRoomMenu, 9);
	languageMenu->hotspots[5].script[1].args[1] = getGameLanguageCode();
}

} // End of namespace Myst3
