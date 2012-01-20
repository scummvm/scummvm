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

#ifndef DATABASE_H_
#define DATABASE_H_

#include "common/scummsys.h"
#include "engines/myst3/hotspot.h"
#include "common/str.h"
#include "common/ptr.h"
#include "common/array.h"
#include "common/hashmap.h"
#include "common/stream.h"

namespace Common {
class SeekableSubReadStreamEndian;
}

namespace Myst3 {

struct NodeData
{
	int16 id;
	Common::Array<CondScript> scripts;
	Common::Array<HotSpot> hotspots;
};

// Nodes are using ref counting pointers since they can be
// deleted by a script they own
typedef Common::SharedPtr<NodeData> NodePtr;

struct RoomData
{
	uint32 id;
	char name[8];
	uint32 scriptsOffset;
	uint32 ambSoundsOffset;
	uint32 unkOffset;
	uint32 roomUnk4;
	uint32 roomUnk5;
};

struct AgeData
{
	uint32 id;
	uint32 disk;
	uint32 roomCount;
	uint32 roomsOffset;
	uint32 labelId;

	Common::Array<RoomData> rooms;
};

class Database
{
public:
	/**
	 * Initialize the database from an executable file
	 */
	Database();

	/**
	 * Loads a room's nodes into the database
	 */
	void setCurrentRoom(uint32 roomID);

	/**
	 * Returns a node's hotspots and scripts from the currently loaded room
	 */
	NodePtr getNodeData(uint16 nodeID, uint32 roomID = 0, uint32 ageID = 0);

	/**
	 * Returns the generic node init script
	 */
	const Common::Array<Opcode>& getNodeInitScript() { return _nodeInitScript; }

	/**
	 * Returns the name of the currently loaded room
	 */
	void getRoomName(char name[8], uint32 roomID = 0);

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
private:
	struct GameVersion {
		const char *description;
		Common::Platform platform;
		const char *md5;
		uint32 baseOffset;
		uint32 ageTableOffset;
		uint32 nodeInitScriptOffset;
		uint32 soundNamesOffset;
	};

	Common::String _exePath;
	GameVersion *_gameVersion;

	Common::Array<AgeData> _ages;

	uint32 _currentRoomID;
	RoomData *_currentRoomData;
	Common::HashMap< uint16, Common::Array<NodePtr> > _roomNodesCache;

	Common::Array<Opcode> _nodeInitScript;

	Common::HashMap< uint32, Common::String> _soundNames;

	RoomData *findRoomData(const uint32 &roomID);
	Common::Array<NodePtr> loadRoomScripts(RoomData *room);
	void preloadCommonRooms();

	Common::Array<AgeData> loadAges(Common::ReadStreamEndian &s);
	RoomData loadRoomDescription(Common::ReadStreamEndian &s);

	Common::Array<CondScript> loadCondScripts(Common::ReadStreamEndian & s);
	Common::Array<Opcode> loadOpcodes(Common::ReadStreamEndian & s);
	Common::Array<HotSpot> loadHotspots(Common::ReadStreamEndian & s);
	Common::Array<PolarRect> loadRects(Common::ReadStreamEndian & s);
	CondScript loadCondScript(Common::ReadStreamEndian & s);
	HotSpot loadHotspot(Common::ReadStreamEndian & s);

	void loadSoundNames(Common::ReadStreamEndian *s);

	Common::SeekableSubReadStreamEndian *openDatabaseFile() const;
	Common::SeekableReadStream *decompressPEFDataSegment(Common::SeekableReadStream *stream, uint segmentID) const;
};

} /* namespace Myst3 */
#endif /* DATABASE_H_ */
