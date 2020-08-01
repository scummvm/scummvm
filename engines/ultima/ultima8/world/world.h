/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#ifndef ULTIMA8_WORLD_WORLD_H
#define ULTIMA8_WORLD_WORLD_H

// the main world class.

// Current ideas on how to store the game world: (-wjp)

// World contains Map objects. All Map objects are initialized when
//  starting/loading a game.

// Each Map permanently contains the nonfixed objects in that map,
//  ( These objects don't have to be assigned object IDs here, I think )

// When a Map is loaded, it loads the fixed objects from disk.
// All objects in the active map will also have to be assigned object IDs.
//  NB: This has to happen in such a way that when a game is loaded the IDs can
//  be reproduced exactly. (Since running usecode scripts can contain objIDs)
// The first N objIDs will probably be reserved from NPCs (and inventories?)
// After that, for example, the fixed objects could be loaded (in disk-order),
// followed by assigning IDs to the (already loaded) dynamic items.
//  (in basic depth-first order, I would guess)

// NPCs will also have to be stored somewhere. We could keep them in Map #0
// like the original.
// Q: Is the avatar's inventory stored in npcdata?
// Q: Is the number of objIDs reserved for npcdata fixed?

// World also has to have the necessary save/load functions. (Which will
//  mostly consist of calls to the save/load functions of the Map objects.)

// Fixed objects could be kept cached in for better performance, or
// swapped out for memory.

// A clear/reset function would also be useful. (All singletons that store
//  game data need this, actually.)

#include "ultima/ultima8/misc/common_types.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima8 {

class Map;
class CurrentMap;
class Actor;
class MainActor;
class Flex;
class Item;

class World {
public:
	World();
	~World();

	static World *get_instance() {
		return _world;
	}

	//! clear the world (maps, currentmap, ethereal items)
	void clear();

	//! reset the world (clear everything and re-initialize maps)
	void reset();

	//! create (empty) maps, currentmap
	void initMaps();

	//! load U8's nonfixed.dat into the Maps
	void loadNonFixed(Common::SeekableReadStream *rs); // delete ds afterwards

	//! load U8's itemcach.dat, npcdata.dat into the world
	void loadItemCachNPCData(Common::SeekableReadStream *itemcach, Common::SeekableReadStream *npcdata);

	//! get the CurrentMap
	CurrentMap *getCurrentMap() const {
		return _currentMap;
	}

	//! switch map. This unloads the CurrentMap back into a Map, kills
	//! processes, and loads a new Map into the CurrentMap.
	//! \return true if successful
	bool switchMap(uint32 newmap);

	//! push an item onto the ethereal void
	void etherealPush(ObjId objid) {
		_ethereal.push_front(objid);
	}

	//! check if the the ethereal void is empty
	bool etherealEmpty() const {
		return _ethereal.empty();
	}

	//! return (but don't remove) the top item from the ethereal void
	ObjId etherealPeek() const {
		return _ethereal.front();
	}

	//! remove an item from the ethereal void
	void etherealRemove(ObjId objid) {
		_ethereal.remove(objid);
	}

	//! output some statistics about the world
	void worldStats() const;

	//! save the Maps in World.
	void saveMaps(Common::WriteStream *ws);

	//! load Maps
	bool loadMaps(Common::ReadStream *rs, uint32 version);

	//! save the rest of the World data (ethereal items, current map number).
	void save(Common::WriteStream *ws);

	//! load World data
	bool load(Common::ReadStream *rs, uint32 version);

	bool isAlertActive() const {
		return _alertActive;
	}

	void setAlertActive(bool active);

	uint8 getGameDifficulty() const {
		return _difficulty;
	}
	void setGameDifficulty(uint8 difficulty) {
		_difficulty = difficulty;
	}

	uint16 getControlledNPCNum() const {
		return _controlledNPCNum;
	}
	void setControlledNPCNum(uint16 num);

	INTRINSIC(I_getAlertActive); // for Crusader
	INTRINSIC(I_setAlertActive); // for Crusader
	INTRINSIC(I_clrAlertActive); // for Crusader
	INTRINSIC(I_gameDifficulty); // for Crusader
	INTRINSIC(I_getControlledNPCNum); // for Crusader
	INTRINSIC(I_setControlledNPCNum); // for Crusader

private:
	static World *_world;

	Std::vector<Map *> _maps;
	CurrentMap *_currentMap;

	Std::list<ObjId> _ethereal;

	bool _alertActive; //!< is intruder alert active (Crusader)
	uint8 _difficulty; //!< game difficulty level (Crusader)
	uint16 _controlledNPCNum; //!< Current controlled NPC (normally 1, the avatar)

};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
