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
#include "common/language.h"
#include "common/platform.h"
#include "common/ptr.h"
#include "common/array.h"
#include "common/hashmap.h"
#include "common/stream.h"

namespace Myst3 {

enum GameLocalizationType {
	kLocMonolingual,
	kLocMulti2,
	kLocMulti6
};

enum MystLanguage {
	kEnglish = 0,
	kOther   = 1, // Dutch, Japanese or Polish
	kDutch   = 1,
	kFrench  = 2,
	kGerman  = 3,
	kItalian = 4,
	kSpanish = 5
};

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

struct RoomKey {
	uint16 ageID;
	uint16 roomID;

	RoomKey(uint16 room, uint16 age) : roomID(room), ageID(age) {};

	bool operator==(const RoomKey &k) const {
		return ageID == k.ageID && roomID == k.roomID;
	}
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
	Database(const Common::Platform platform, const Common::Language language, const uint32 localizationType);
	~Database();

	/**
	 * Loads a room's nodes into the database cache
	 */
	void cacheRoom(uint32 roomID, uint32 ageID);

	/**
	 * Tells if a room is a common room
	 *
	 * Common rooms are always in the cache
	 */
	bool isCommonRoom(uint32 roomID, uint32 ageID) const;

	/**
	 * Returns a node's hotspots and scripts from the currently loaded room
	 */
	NodePtr getNodeData(uint16 nodeID, uint32 roomID, uint32 ageID);

	/**
	 * Returns a node's zip id, as used by savestates
	 */
	int32 getNodeZipBitIndex(uint16 nodeID, uint32 roomID, uint32 ageID);

	/**
	 * Returns the generic node init script
	 */
	const Common::Array<Opcode>& getNodeInitScript() { return _nodeInitScript; }

	/**
	 * Returns the name of the currently loaded room
	 */
	Common::String getRoomName(uint32 roomID, uint32 ageID) const;

	/**
	 * Returns the id of a room from its name
	 */
	RoomKey getRoomKey(const char *name);

	/**
	 * Returns the list of the nodes of a room
	 */
	Common::Array<uint16> listRoomNodes(uint32 roomID, uint32 ageID);

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
	const AmbientCue &getAmbientCue(uint16 id);

	int16 getGameLanguageCode() const;

private:
	struct RoomKeyHash {
		uint operator()(const RoomKey &v) const {
			return v.ageID + (v.roomID << 16);
		}
	};

	typedef Common::HashMap<RoomKey, Common::Array<NodePtr>, RoomKeyHash> NodesCache;

	const Common::Platform _platform;
	const Common::Language _language;
	const uint32 _localizationType;

	static const AgeData _ages[];

	NodesCache _roomNodesCache;

	Common::Array<Opcode> _nodeInitScript;

	Common::HashMap<uint32, Common::String> _soundNames;
	Common::HashMap<uint16, AmbientCue> _ambientCues;
	Common::HashMap<uint32, int16> _roomZipBitIndex;

	// 'myst3.dat' cached data
	static const uint kDatVersion = 2;
	Common::SeekableReadStream *_datFile;
	Common::Array<RoomScripts> _roomScriptsIndex;
	int32 _roomScriptsStartOffset;

	const RoomData *findRoomData(uint32 roomID, uint32 ageID) const;
	Common::Array<NodePtr> getRoomNodes(uint32 roomID, uint32 ageID) const;

	Common::Array<NodePtr> readRoomScripts(const RoomData *room) const;
	void preloadCommonRooms();
	void initializeZipBitIndexTable();
	void patchLanguageMenu();
	void patchNodeScripts(const RoomData *room, Common::Array<NodePtr> &nodes) const;

	// 'myst3.dat' read methods
	void readScriptIndex(Common::SeekableReadStream *stream, bool load);
	void readSoundNames(Common::SeekableReadStream *stream, bool load);
	void loadAmbientCues(Common::ReadStream *s);
	Common::SeekableReadStream *getRoomScriptStream(const char *room, ScriptType scriptType) const;
};

} // End of namespace Myst3

#endif // DATABASE_H_
