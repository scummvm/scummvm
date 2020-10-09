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

#include "engines/myst3/hotspot.h"
#include "engines/myst3/detection.h"

#include "common/scummsys.h"
#include "common/str.h"
#include "common/language.h"
#include "common/platform.h"
#include "common/ptr.h"
#include "common/array.h"
#include "common/hashmap.h"
#include "common/stream.h"

namespace Myst3 {

enum MystLanguage {
	kEnglish = 0,
	kOther   = 1, // Dutch, Japanese or Polish
	kDutch   = 1,
	kFrench  = 2,
	kGerman  = 3,
	kItalian = 4,
	kSpanish = 5
};

enum NodeID {
	kNodeSharedInit   = 1,
	kNodeLogoPlay     = 1,
	kNodeMenuNewGame  = 98,
	kNodeMenuMain     = 100,
	kNodeMenuLoadGame = 200,
	kNodeMenuSaveGame = 300
};

enum RoomID {
	kRoomShared        = 101,
	kRoomIntro         = 201,
	kRoomTomahnaStart  = 301,
	kRoomTomahnaReturn = 401,
	kJnaninStart       = 501,
	kRoomLeos          = 502,
	kRoomLeet          = 503,
	kRoomLelt          = 504,
	kRoomLemt          = 505,
	kRoomLeof          = 506,
	kRoomEdannaStart   = 601,
	kRoomLisw          = 602,
	kRoomLifo          = 603,
	kRoomLisp          = 604,
	kRoomLine          = 605,
	kRoomVoltaicStart  = 701,
	kRoomEnpp          = 703,
	kRoomEnem          = 704,
	kRoomEnlc          = 705,
	kRoomEndd          = 706,
	kRoomEnch          = 707,
	kRoomEnli          = 708,
	kRoomNarayan       = 801,
	kRoomMenu          = 901,
	kRoomJournals      = 902,
	kRoomDemo          = 903,
	kRoomAtix          = 904,
	kRoomAmateriaStart = 1001,
	kRoomMais          = 1002,
	kRoomMall          = 1003,
	kRoomMass          = 1004,
	kRoomMaww          = 1005,
	kRoomMato          = 1006,
	kLogo              = 1101
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

	/** Get the sound variable id range */
	uint32 getSoundIdMin() const { return _soundIdMin; }
	uint32 getSoundIdMax() const { return _soundIdMax; }

	/**
	 * Retrieve an ambient cue from its id
	 */
	const AmbientCue &getAmbientCue(uint16 id);

	int16 getGameLanguageCode() const;

	/** Check if the scripts for two rooms are identical */
	bool areRoomsScriptsEqual(uint32 roomID1, uint32 ageID1, uint32 roomID2, uint32 ageID2, ScriptType scriptType);

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

	uint32 _soundIdMin;
	uint32 _soundIdMax;
	Common::HashMap<uint32, Common::String> _soundNames;
	Common::HashMap<uint16, AmbientCue> _ambientCues;
	Common::HashMap<uint32, int16> _roomZipBitIndex;

	// 'myst3.dat' cached data
	static const uint kDatVersion = 3;
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
