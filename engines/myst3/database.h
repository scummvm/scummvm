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

#ifndef DATABASE_H_
#define DATABASE_H_

#include "common/scummsys.h"
#include "engines/myst3/hotspot.h"
#include "common/str.h"
#include "common/ptr.h"
#include "common/array.h"
#include "common/stream.h"

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
	uint8 id;
	uint8 roomUnk1;
	uint8 roomUnk2;
	uint8 roomUnk3;
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
	uint32 ageUnk1;

	Common::Array<RoomData> rooms;
};

class Database
{
public:
	/**
	 * Initialize the database from an executable file
	 */
	Database(const Common::String & executable);

	/**
	 * Loads a room's nodes into the database
	 */
	void loadRoomScripts(uint8 roomID);

	/**
	 * Returns a node's hotspots and scripts from the currently loaded room
	 */
	NodePtr getNodeData(uint16 nodeID);

	void getRoomName(char name[8]);
private:
	struct GameVersion {
		const char *description;
		const char *md5;
		const uint32 ageTableOffset;
	};

	static const uint32 _baseOffset = 0x400000;
	Common::String _exePath;
	GameVersion *_gameVersion;

	Common::Array<AgeData> _ages;

	uint16 _currentRoomID;
	RoomData *_currentRoomData;
	Common::Array<NodePtr> _currentRoomNodes;

	Common::Array<AgeData> loadAges(Common::ReadStream &s);
	RoomData loadRoom(Common::ReadStream &s);

	Common::Array<CondScript> loadCondScripts(Common::ReadStream & s);
	Common::Array<Opcode> loadOpcodes(Common::ReadStream & s);
	Common::Array<HotSpot> loadHotspots(Common::ReadStream & s);
	Common::Array<PolarRect> loadRects(Common::ReadStream & s);
	CondScript loadCondScript(Common::ReadStream & s);
	HotSpot loadHotspot(Common::ReadStream & s);
};

} /* namespace Myst3 */
#endif /* DATABASE_H_ */
