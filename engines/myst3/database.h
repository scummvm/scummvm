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

#ifndef DATABASE_H_
#define DATABASE_H_

#include "common/scummsys.h"
#include "engines/myst3/hotspot.h"
#include "common/str.h"
#include "common/ptr.h"
#include "common/array.h"
#include "common/hashmap.h"
#include "common/stream.h"

namespace Myst3 {

struct NodeData {
	int16 id;
	int16 zipBitIndex;
	Common::Array<CondScript> scripts;
	Common::Array<HotSpot> hotspots;
	Common::Array<CondScript> soundScripts;
	Common::Array<CondScript> backgroundSoundScripts;
};

// Nodes are using ref counting pointers since they can be
// deleted by a script they own
typedef Common::SharedPtr<NodeData> NodePtr;

struct RoomData {
	uint32 id;
	const char *name;
};

struct AgeData {
	uint32 id;
	uint32 disk;
	uint32 roomCount;
	const RoomData *rooms;
	uint32 labelId;
};

struct AmbientCue {
	uint16 id;
	uint16 minFrames;
	uint16 maxFrames;
	Common::Array<uint16> tracks;
};

/**
 * Script types stored in 'myst3.dat'
 */
enum ScriptType {
	kScriptTypeNode,
	kScriptTypeAmbientSound,
	kScriptTypeBackgroundSound,
	kScriptTypeNodeInit,
	kScriptTypeAmbientCue
};

/**
 * A script index entry in the 'myst3.dat' file
 */
struct RoomScripts {
	Common::String room;
	ScriptType type;
	uint offset;
	uint size;
};

class Myst3Engine;

class Database {
public:
	Database(Myst3Engine *vm);
	~Database();

	/**
	 * Loads a room's nodes into the database
	 */
	void setCurrentRoom(uint32 roomID);

	/**
	 * Returns a node's hotspots and scripts from the currently loaded room
	 */
	NodePtr getNodeData(uint16 nodeID, uint32 roomID = 0, uint32 ageID = 0);

	/**
	 * Returns a node's zip id, as used by savestates
	 */
	int32 getNodeZipBitIndex(uint16 nodeID, uint32 roomID);

	/**
	 * Returns the generic node init script
	 */
	const Common::Array<Opcode>& getNodeInitScript() { return _nodeInitScript; }

	/**
	 * Returns the name of the currently loaded room
	 */
	Common::String getRoomName(uint32 roomID = 0);

	/**
	 * Returns the id of a room from its name
	 */
	uint32 getRoomId(const char *name);

	/**
	 * Returns the list of the nodes of a room
	 */
	Common::Array<uint16> listRoomNodes(uint32 roomID = 0, uint32 ageID = 0);

	/**
	 * Returns an age's label id, to be used with AGES 1000 metadata
	 */
	uint32 getAgeLabelId(uint32 ageID);

	/**
	 * Retrieve the file name of a sound from its id
	 */
	Common::String getSoundName(uint32 id);

	/**
	 * Retrieve an ambient cue from its id
	 */
	const AmbientCue& getAmbientCue(uint16 id);
private:
	Myst3Engine *_vm;

	static const AgeData _ages[];

	uint32 _currentRoomID;
	const RoomData *_currentRoomData;
	Common::HashMap< uint16, Common::Array<NodePtr> > _roomNodesCache;

	Common::Array<Opcode> _nodeInitScript;

	Common::HashMap<uint32, Common::String> _soundNames;
	Common::HashMap<uint16, AmbientCue> _ambientCues;
	Common::HashMap<uint32, int16> _roomZipBitIndex;

	// 'myst3.dat' cached data
	static const uint kDatVersion = 1;
	Common::SeekableReadStream *_datFile;
	Common::Array<RoomScripts> _roomScriptsIndex;
	int32 _roomScriptsStartOffset;

	const RoomData *findRoomData(uint32 roomID);
	Common::Array<NodePtr> getRoomNodes(uint32 roomID);

	Common::Array<NodePtr> loadRoomScripts(const RoomData *room);
	void loadRoomNodeScripts(Common::SeekableReadStream *file, Common::Array<NodePtr> &nodes);
	void loadRoomSoundScripts(Common::SeekableReadStream *file, Common::Array<NodePtr> &nodes, bool background);
	void preloadCommonRooms();
	void initializeZipBitIndexTable();
	void patchLanguageMenu();

	Common::Array<CondScript> loadCondScripts(Common::SeekableReadStream & s);
	Common::Array<Opcode> loadOpcodes(Common::SeekableReadStream & s);
	Common::Array<HotSpot> loadHotspots(Common::SeekableReadStream & s);
	Common::Array<PolarRect> loadRects(Common::SeekableReadStream & s);
	CondScript loadCondScript(Common::SeekableReadStream & s);
	HotSpot loadHotspot(Common::SeekableReadStream & s);

	// 'myst3.dat' read methods
	void readScriptIndex(Common::SeekableReadStream *stream, bool load);
	void readSoundNames(Common::SeekableReadStream *stream, bool load);
	void loadAmbientCues(Common::ReadStream *s);
	Common::SeekableReadStream *getRoomScriptStream(const char *room, ScriptType scriptType) const;
};

} // End of namespace Myst3

#endif // DATABASE_H_
