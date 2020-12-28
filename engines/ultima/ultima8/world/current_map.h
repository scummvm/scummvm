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

#ifndef ULTIMA8_WORLD_CURRENTMAP_H
#define ULTIMA8_WORLD_CURRENTMAP_H

#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/misc/direction.h"

namespace Ultima {
namespace Ultima8 {

class Map;
class Item;
class UCList;
class TeleportEgg;
class EggHatcherProcess;

#define MAP_NUM_CHUNKS  64
#define MAP_NUM_TARGET_ITEMS 200

class CurrentMap {
	friend class World;
public:
	CurrentMap();
	~CurrentMap();

	void clear();
	void writeback();
	void loadMap(Map *map);

	//! sets the currently loaded map, without any processing.
	//! (Should only be used for loading.)
	void setMap(Map *map) {
		_currentMap = map;
	}

	//! Get the map number of the CurrentMap
	uint32 getNum() const;

	unsigned int getChunkSize() const {
		return _mapChunkSize;
	}

	//! Add an item to the beginning of the item list
	void addItem(Item *item);

	//! Add an item to the end of the item list
	void addItemToEnd(Item *item);

	void removeItemFromList(Item *item, int32 oldx, int32 oldy);
	void removeItem(Item *item);

	//! Add an item to the list of possible targets (in Crusader)
	void addTargetItem(const Item *item);
	//! Remove an item from the list of possible targets (in Crusader)
	void removeTargetItem(const Item *item);
	//! Find the best target item in the given direction
	Item *findBestTargetItem(int32 x, int32 y, Direction dir, DirectionMode dirmode);

	//! Update the fast area for the cameras position
	void updateFastArea(int32 from_x, int32 from_y, int32 from_z, int32 to_x, int32 to_y, int32 to_z);

	//! search an area for items matching a loopscript
	//! \param itemlist the list to return objids in
	//! \param loopscript the script to check items against
	//! \param scriptsize the size (in bytes) of the loopscript
	//! \param item the item around which you want to search, or 0.
	//!             if item is 0, search around (x,y)
	//! \param range the (square) range to search
	//! \param recurse if true, search in containers too
	//! \param x x coordinate of search center if item is 0.
	//! \param y y coordinate of search center if item is 0.
	void areaSearch(UCList *itemlist, const uint8 *loopscript,
	                uint32 scriptsize, const Item *item, uint16 range,
					bool recurse, int32 x = 0, int32 y = 0) const;

	// Surface search: Search above and below an item.
	void surfaceSearch(UCList *itemlist, const uint8 *loopscript,
	                   uint32 scriptsize, const Item *item, bool above,
					   bool below, bool recurse = false) const;

	// Surface search: Search above and below an item.
	void surfaceSearch(UCList *itemlist, const uint8 *loopscript,
	                   uint32 scriptsize, ObjId id,
	                   int32 origin[3], int32 dims[2],
	                   bool above, bool below, bool recurse = false) const;

	// Collision detection. Returns true if the box [x,y,z]-[x-xd,y-yd,z+zd]
	// does not collide with any solid items.
	// Additionally:
	// * If support is not NULL, *support is set to the item supporting
	//   the given box, or 0 if it isn't supported.
	// * If roof is not NULL, *roof is set to the roof item with the lowest
	//   z coordinate that's over the box, or 0 if there is no roof above box.
	// * If blocker is not NULL, *blocker will be set to an item blocking
	//   the whole box if there is one, or 0 if there is no such item.
	// Ignores collisions which were already occurring at the start position.
	// NB: isValidPosition doesn't consider item 'item'.
	bool isValidPosition(int32 x, int32 y, int32 z,
	                     int32 startx, int32 starty, int32 startz,
	                     int xd, int yd, int zd, uint32 shapeflags,
	                     ObjId item, const Item **support = 0,
	                     ObjId *roof = 0, const Item **blocker = 0) const;

	// Note that this version of isValidPosition does not look for start
	// position collisions.
	bool isValidPosition(int32 x, int32 y, int32 z,
	                     int xd, int yd, int zd, uint32 shapeflags,
	                     ObjId item, const Item **support = 0,
						 ObjId *roof = 0, const Item **blocker = 0) const;

	// Note that this version of isValidPosition can not take 'flipped'
	// into account!
	bool isValidPosition(int32 x, int32 y, int32 z, uint32 shape,
	                     ObjId item, const Item **support = 0,
                         ObjId *roof = 0, const Item **blocker = 0) const;

	//! Scan for a valid position for item in directions orthogonal to movedir
	bool scanForValidPosition(int32 x, int32 y, int32 z, const Item *item,
	                          Direction movedir, bool wantsupport,
	                          int32 &tx, int32 &ty, int32 &tz);

	struct SweepItem {
		SweepItem(ObjId it, int32 ht, int32 et, bool touch,
		          bool touchfloor, bool block, uint8 dir)
			: _item(it), _hitTime(ht), _endTime(et), _touching(touch),
			  _touchingFloor(touchfloor), _blocking(block), _dirs(dir) { }

		ObjId   _item;       // Item that was hit

		//
		// The time values here are 'normalized' fixed point values
		// They range from 0 for the start of the move to 0x4000 for the end of
		// The move.
		//
		// Linear interpolate between the start and end positions using
		// hit_time to find where the moving item was when the hit occurs
		//

		int32   _hitTime;   // if -1, already hitting when sweep started.
		int32   _endTime;   // if 0x4000, still hitting when sweep finished

		bool    _touching;   // We are only touching (don't actually overlap)
		bool    _touchingFloor; // touching and directly below the moving item

		bool    _blocking;   // This item blocks the moving item

		uint8   _dirs; // Directions in which the item is being hit.
		// Bitmask. Bit 0 is x, 1 is y, 2 is z.

		// Use this func to get the interpolated location of the hit
		void GetInterpolatedCoords(int32 out[3], const int32 start[3], const int32 end[3]) const {
			for (int i = 0; i < 3; i++)
				out[i] = start[i] + ((end[i] - start[i]) * (_hitTime >= 0 ? _hitTime : 0) + (end[i] > start[i] ? 0x2000 : -0x2000)) / 0x4000;
		}
	};

	//! Perform a sweepTest for an item move
	//! \param start Start point to sweep from.
	//! \param end End point to sweep to.
	//! \param dims Bounding size of item to check.
	//! \param shapeflags shapeflags of item to check.
	//! \param item ObjId of the item being checked. This will allow item to
	//!             be skipped from being tested against. Use 0 for no item.
	//! \param solid_only If true, only test solid items.
	//! \param hit Pointer to a list to fill with items hit. Items are sorted
	//!            by SweepItem::hit_time
	//! \return false if no items were hit.
	//!         true if any items were hit.
	bool sweepTest(const int32 start[3], const int32 end[3],
	               const int32 dims[3], uint32 shapeflags,
	               ObjId item, bool solid_only, Std::list<SweepItem> *hit) const;

	TeleportEgg *findDestination(uint16 id);

	// Not allowed to modify the list. Remember to use const_iterator
	const Std::list<Item *> *getItemList(int32 gx, int32 gy) const;

	bool isChunkFast(int32 cx, int32 cy) const {
		// CONSTANTS!
		if (cx < 0 || cy < 0 || cx >= MAP_NUM_CHUNKS || cy >= MAP_NUM_CHUNKS)
			return false;
		return (_fast[cy][cx / 32] & (1 << (cx & 31))) != 0;
	}

	// A simple trace to find the top item at a specific xy point
	const Item *traceTopItem(int32 x, int32 y, int32 ztop, int32 zbot, ObjId ignore, uint32 shflags);

	// Set the entire map as being 'fast'
	void setWholeMapFast();

	void save(Common::WriteStream *ws);
	bool load(Common::ReadStream *rs, uint32 version);

	INTRINSIC(I_canExistAt);
	INTRINSIC(I_canExistAtPoint);

private:
	void loadItems(const Std::list<Item *> &itemlist, bool callCacheIn);
	void createEggHatcher();

	//! clip the given map chunk numbers to iterate over them safely
	void clipMapChunks(int &minx, int &maxx, int &miny, int &maxy) const;

	Map *_currentMap;

	// item lists. Lots of them :-)
	// items[x][y]
	Std::list<Item *> _items[MAP_NUM_CHUNKS][MAP_NUM_CHUNKS];

	ProcId _eggHatcher;

	// Fast area bit masks -> fast[ry][rx/32]&(1<<(rx&31));
	uint32 _fast[MAP_NUM_CHUNKS][MAP_NUM_CHUNKS / 32];
	int32 _fastXMin, _fastYMin, _fastXMax, _fastYMax;

	int _mapChunkSize;

	//! Items that are "targetable" in Crusader. It might be faster to store
	//! this in a more fancy data structure, but this works fine.
	ObjId _targets[MAP_NUM_TARGET_ITEMS];

	void setChunkFast(int32 cx, int32 cy);
	void unsetChunkFast(int32 cx, int32 cy);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
