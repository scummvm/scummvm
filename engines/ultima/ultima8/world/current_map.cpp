/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ultima/ultima.h"
#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/map.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/world_point.h"
#include "ultima/ultima8/world/coord_utils.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/world/teleport_egg.h"
#include "ultima/ultima8/world/egg_hatcher_process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/main_shape_archive.h"
#include "ultima/ultima8/gumps/game_map_gump.h"
#include "ultima/ultima8/misc/box.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/world/get_object.h"

// Uncomment to check that a single object doesn't appear in multiple chunks
// during updates
//#define VALIDATE_CHUNKS 1

namespace Ultima {
namespace Ultima8 {

typedef Std::list<Item *> item_list;

const int INT_MAX_VALUE = 0x7fffffff;
const int INT_MIN_VALUE = -INT_MAX_VALUE - 1;

CurrentMap::CurrentMap() : _currentMap(0), _eggHatcher(0),
	  _fastXMin(-1), _fastYMin(-1), _fastXMax(-1), _fastYMax(-1) {
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; i++) {
		memset(_fast[i], false, sizeof(uint32)*MAP_NUM_CHUNKS / 32);
	}

	if (GAME_IS_U8) {
		_mapChunkSize = 512;
	} else if (GAME_IS_CRUSADER) {
		_mapChunkSize = 1024;
	} else {
		warning("Unknown game type in CurrentMap constructor.");
	}

	for (unsigned int i = 0; i < MAP_NUM_TARGET_ITEMS; i++) {
		_targets[i] = 0;
	}
}


CurrentMap::~CurrentMap() {
//	clear();
}

void CurrentMap::clear() {
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; i++) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS; j++) {
			for (auto *item : _items[i][j])
				delete item;
			_items[i][j].clear();
		}
		memset(_fast[i], false, sizeof(uint32)*MAP_NUM_CHUNKS / 32);
	}

	_fastXMin =  _fastYMin = _fastXMax = _fastYMax = -1;
	_currentMap = nullptr;

	Process *ehp = Kernel::get_instance()->getProcess(_eggHatcher);
	if (ehp)
		ehp->terminate();
	_eggHatcher = 0;
}

uint32 CurrentMap::getNum() const {
	if (_currentMap == nullptr)
		return 0;

	return _currentMap->_mapNum;
}

void CurrentMap::createEggHatcher() {
	// get rid of old one, if any
	Process *ehp = Kernel::get_instance()->getProcess(_eggHatcher);
	if (ehp)
		ehp->terminate();

	ehp = new EggHatcherProcess();
	_eggHatcher = Kernel::get_instance()->addProcess(ehp);
}

void CurrentMap::writeback() {
	if (!_currentMap)
		return;

	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; i++) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS; j++) {
			for (auto *item : _items[i][j]) {
				// item is being removed from the CurrentMap item lists
				item->clearExtFlag(Item::EXT_INCURMAP);

				// delete all fast only and disposable _items
				if (item->hasFlags(Item::FLG_FAST_ONLY | Item::FLG_DISPOSABLE)) {
					delete item;
					continue;
				}

				// Reset the egg
				Egg *egg = dynamic_cast<Egg *>(item);
				if (egg) {
					egg->reset();
				}

				// this item isn't from the Map. (like NPCs)
				if (item->hasFlags(Item::FLG_IN_NPC_LIST))
					continue;

				item->clearObjId();
				if (item->hasExtFlags(Item::EXT_FIXED)) {
					// item came from fixed
					_currentMap->_fixedItems.push_back(item);
				} else {
					_currentMap->_dynamicItems.push_back(item);
				}
			}
			_items[i][j].clear();
		}
	}

	// delete _eggHatcher
	Process *ehp = Kernel::get_instance()->getProcess(_eggHatcher);
	if (ehp)
		ehp->terminate();
	_eggHatcher = 0;
}

void CurrentMap::loadItems(const Std::list<Item *> &itemlist, bool callCacheIn) {
	for (auto *item : itemlist) {
		item->assignObjId();

		// No fast area for you!
		item->clearFlag(Item::FLG_FASTAREA);

		// add item to internal object list
		addItemToEnd(item);

		if (callCacheIn)
			item->callUsecodeEvent_cachein();
	}
}

void CurrentMap::loadMap(Map *map) {
	// Don't call the cachein events at startup or when loading a savegame
	// in u8.  Always call them in Crusader.
	// TODO: This may not work for loading games in Crusader - need to check.
	bool callCacheIn = (_currentMap != nullptr || GAME_IS_CRUSADER);

	_currentMap = map;

	createEggHatcher();

	// Clear fast area
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; i++) {
		memset(_fast[i], false, sizeof(uint32)*MAP_NUM_CHUNKS / 32);
	}
	_fastXMin = -1;
	_fastYMin = -1;
	_fastXMax = -1;
	_fastYMax = -1;

	// Clear target items
	for (unsigned int i = 0; i < MAP_NUM_TARGET_ITEMS; i++) {
		_targets[i] = 0;
	}

	loadItems(map->_fixedItems, callCacheIn);
	loadItems(map->_dynamicItems, callCacheIn);

	// we take control of the items in map, so clear the pointers
	map->_fixedItems.clear();
	map->_dynamicItems.clear();

	// load relevant NPCs to the item lists
	// !constant
	for (uint16 i = 0; i < 256; ++i) {
		Actor *actor = getActor(i);
		if (!actor) continue;

		// Schedule
		// CHECKME: is this the right time to pass?
		if (callCacheIn)
			actor->schedule(Ultima8Engine::get_instance()->getGameTimeInSeconds() / 60);

		if (actor->getMapNum() == getNum()) {
			addItemToEnd(actor);

			// the avatar's cachein function is very strange in U8; disabled for now
			if (callCacheIn && GAME_IS_CRUSADER)
				actor->callUsecodeEvent_cachein();
		}
	}
}

void CurrentMap::addItem(Item *item) {
	Point3 pt = item->getLocation();

	if (pt.x < 0 || pt.x >= _mapChunkSize * MAP_NUM_CHUNKS ||
	        pt.y < 0 || pt.y >= _mapChunkSize * MAP_NUM_CHUNKS) {
		//warning("Skipping item %u: out of range (%d, %d)", item->getObjId(), pt.x, pt.y);
		return;
	}

	int32 cx = pt.x / _mapChunkSize;
	int32 cy = pt.y / _mapChunkSize;

#ifdef VALIDATE_CHUNKS
	for (int32 ccy = 0; ccy < MAP_NUM_CHUNKS; ccy++) {
		for (int32 ccx = 0; ccx < MAP_NUM_CHUNKS; ccx++) {
			for (const auto *existing : _items[ccx][ccy])
				if (existing == item) {
					warning("item %d already exists in map chunk (%d, %d)", item->getObjId(), ccx, ccy);
				}
			}
		}
	}
#endif

	_items[cx][cy].push_front(item);
	item->setExtFlag(Item::EXT_INCURMAP);

	Egg *egg = dynamic_cast<Egg *>(item);
	if (egg) {
		EggHatcherProcess *ehp = dynamic_cast<EggHatcherProcess *>(Kernel::get_instance()->getProcess(_eggHatcher));
		assert(ehp);
		ehp->addEgg(egg);
	}
}

void CurrentMap::addItemToEnd(Item *item) {
	Point3 pt = item->getLocation();

	if (pt.x < 0 || pt.x >= _mapChunkSize * MAP_NUM_CHUNKS ||
	        pt.y < 0 || pt.y >= _mapChunkSize * MAP_NUM_CHUNKS) {
		//warning("Skipping item %u: out of range (%d, %d)", item->getObjId(), pt.x, pt.y);
		return;
	}

	int32 cx = pt.x / _mapChunkSize;
	int32 cy = pt.y / _mapChunkSize;

#ifdef VALIDATE_CHUNKS
	for (int32 ccy = 0; ccy < MAP_NUM_CHUNKS; ccy++) {
		for (int32 ccx = 0; ccx < MAP_NUM_CHUNKS; ccx++) {
			for (const auto *existing : _items[ccx][ccy])
				if (existing == item) {
					warning("item %d already exists in map chunk (%d, %d)", item->getObjId(), ccx, ccy);
				}
			}
		}
	}
#endif

	_items[cx][cy].push_back(item);
	item->setExtFlag(Item::EXT_INCURMAP);

	Egg *egg = dynamic_cast<Egg *>(item);
	if (egg) {
		EggHatcherProcess *ehp = dynamic_cast<EggHatcherProcess *>(Kernel::get_instance()->getProcess(_eggHatcher));
		assert(ehp);
		ehp->addEgg(egg);
	}
}

void CurrentMap::removeItem(Item *item) {
	Point3 pt = item->getLocation();

	removeItemFromList(item, pt.x, pt.y);
}

void CurrentMap::addTargetItem(const Item *item) {
	assert(item);
	// The game also maintains a count of non-zero targets, but it
	// seems to serve little purpose so we just update.
	ObjId id = item->getObjId();
	for (int i = 0; i < MAP_NUM_TARGET_ITEMS; i++) {
		if (_targets[i] == 0) {
			_targets[i] = id;
			return;
		}
	}
}

void CurrentMap::removeTargetItem(const Item *item) {
	assert(item);
	ObjId id = item->getObjId();
	for (int i = 0; i < MAP_NUM_TARGET_ITEMS; i++) {
		if (_targets[i] == id) {
			_targets[i] = 0;
			return;
		}
	}
}


Item *CurrentMap::findBestTargetItem(int32 x, int32 y, int32 z, Direction dir, DirectionMode dirmode) {
	// "best" means:
	// Shape info SI_OCCL
	// isNPC
	// Closest
	// in that order.
	bool bestisnpc = false;
	bool bestisoccl = false;
	Item *bestitem = nullptr;
	int bestdist = 0xffff;
	const uint16 controllednpc = World::get_instance()->getControlledNPCNum();

	for (int i = 0; i < MAP_NUM_TARGET_ITEMS; i++) {
		if (_targets[i] == 0 || _targets[i] == controllednpc)
			continue;
		Item *item = getItem(_targets[i]);
		// FIXME: this should probably always be non-null,
		// but if it's not the item disappeared - remove it from the list.
		// If we fix this, this function can be const.
		if (!item) {
			_targets[i] = 0;
			continue;
		}

		if (item->hasFlags(Item::FLG_BROKEN))
			continue;

		const ShapeInfo *si = item->getShapeInfo();
		bool isoccl = si->_flags & ShapeInfo::SI_OCCL;

		Point3 pt = item->getLocation();
		Direction itemdir = Direction_GetWorldDir(pt.y - y, pt.x - x, dirmode);
		if (itemdir != dir)
			continue;

		const Actor *actor = dynamic_cast<const Actor *>(item);
		if ((bestisoccl && !isoccl) || (bestisnpc && !actor) || !item->isPartlyOnScreen())
			continue;

		int xdiff = abs(x - pt.x);
		int ydiff = abs(y - pt.y);
		int zdiff = abs(z - pt.z);
		int dist = MAX(MAX(xdiff, ydiff), zdiff);

		if (dist < bestdist) {
			bestitem = item;
			bestdist = dist;
			bestisoccl = isoccl;
			bestisnpc = (actor != nullptr);
		}
	}
	return bestitem;
}


void CurrentMap::removeItemFromList(Item *item, int32 oldx, int32 oldy) {
	//! This might a bit too inefficient
	// if it's really a problem we could change the item lists into sets
	// or something, but let's see how it turns out

	if (oldx < 0 || oldx >= _mapChunkSize * MAP_NUM_CHUNKS ||
	        oldy < 0 || oldy >= _mapChunkSize * MAP_NUM_CHUNKS) {
		//warning("Skipping item %u: out of range (%d, %d)", item->getObjId(), oldx, oldy);
		return;
	}

	int32 cx = oldx / _mapChunkSize;
	int32 cy = oldy / _mapChunkSize;

	_items[cx][cy].remove(item);
	item->clearExtFlag(Item::EXT_INCURMAP);
}

// Check to see if the chunk is on the screen
static inline bool ChunkOnScreen(int32 cx, int32 cy, int32 sleft, int32 stop, int32 sright, int32 sbot, int mapChunkSize) {
	int32 scx = (cx * mapChunkSize - cy * mapChunkSize) / 4;
	int32 scy = (cx * mapChunkSize + cy * mapChunkSize) / 8;

	// Screenspace bounding box left extent    (LNT x coord)
	int32 cxleft = scx - mapChunkSize / 4;
	// Screenspace bounding box right extent   (RFT x coord)
	int32 cxright = scx + mapChunkSize / 4;

	// Screenspace bounding box top extent     (LFT y coord)
	int32 cytop = scy - mapChunkSize / 2;
	// Screenspace bounding box bottom extent  (RNB y coord)
	int32 cybot = scy + mapChunkSize / 4;

	const bool right_clear = cxright <= sleft;
	const bool left_clear = cxleft >= sright;
	const bool top_clear = cytop >= sbot;
	const bool bot_clear = cybot <= stop;

	const bool clear = right_clear || left_clear || top_clear || bot_clear;

	return !clear;
}

static inline void CalcFastAreaLimits(int32 &sx_limit,
									  int32 &sy_limit,
									  int32 &xy_limit,
									  const Common::Rect32 &dims,
									  int mapChunkSize) {
	// By default the fastArea is the screensize rounded down to the nearest
	// map chunk, plus 3 wide and 7 high.

	// In the original games, the fast area is +/- 3,7 in U8
	// map chunks and +/- 3,5 for Cruasder.  We have to do it a
	// bit differently because the screen size is adjustable.
	sx_limit = dims.width() / (mapChunkSize / 2) + 3;
	sy_limit = dims.height() / (mapChunkSize / 4) + 7;
	xy_limit = (sy_limit + sx_limit) / 2;
}

void CurrentMap::updateFastArea(const Point3 &from, const Point3 &to) {
	int x_min = MIN(from.x, to.x);
	int x_max = MAX(from.x, to.x);

	int y_min = MIN(from.y, to.y);
	int y_max = MAX(from.y, to.y);

	int z_min = MIN(from.z, to.z);
	int z_max = MAX(from.z, to.z);

	// Work out Fine (screenspace) Limits of chunks with half chunk border
	Common::Rect32 dims = Ultima8Engine::get_instance()->getGameMapGump()->getDims();

	int32 sleft  = ((x_min - y_min) / 4)         - (dims.width() / 2 + _mapChunkSize / 4);
	int32 stop   = ((x_min + y_min) / 8 - z_max) - (dims.height() / 2 + _mapChunkSize / 8);
	int32 sright = ((x_max - y_max) / 4)         + (dims.width() / 2 + _mapChunkSize / 4);
	int32 sbot   = ((x_max + y_max) / 8 - z_min) + (dims.height() / 2 + _mapChunkSize / 8);

	// Don't do anything IF the regions are the same
	if (_fastXMin == sleft && _fastYMin == stop &&
	        _fastXMax == sright && _fastYMax == sbot)
		return;

	// Update the saved region
	_fastXMin = sleft;
	_fastYMin = stop;
	_fastXMax = sright;
	_fastYMax = sbot;

	// Get Coarse Limits
	int32 sx_limit;
	int32 sy_limit;
	int32 xy_limit;

	CalcFastAreaLimits(sx_limit, sy_limit, xy_limit, dims, _mapChunkSize);

	x_min = x_min / _mapChunkSize - xy_limit;
	x_max = x_max / _mapChunkSize + xy_limit;
	y_min = y_min / _mapChunkSize - xy_limit;
	y_max = y_max / _mapChunkSize + xy_limit;

	for (int32 cy = 0; cy < MAP_NUM_CHUNKS; cy++) {
		for (int32 cx = 0; cx < MAP_NUM_CHUNKS; cx++) {

			// Coarse
			bool want_fast = cx >= x_min && cx <= x_max && cy >= y_min && cy <= y_max;

			// Fine
			if (want_fast) want_fast = ChunkOnScreen(cx, cy, sleft, stop, sright, sbot, _mapChunkSize);

			bool currently_fast = isChunkFast(cx, cy);

			// Don't do anything, they are the same
			if (want_fast == currently_fast)
				continue;

			// leave fast area
			if (!want_fast) unsetChunkFast(cx, cy);
			// Enter fast area
			else setChunkFast(cx, cy);
		}
	}
}

void CurrentMap::setChunkFast(int32 cx, int32 cy) {
	_fast[cy][cx / 32] |= 1 << (cx & 31);

	for (auto *item : _items[cx][cy]) {
		item->enterFastArea();
	}
}

void CurrentMap::unsetChunkFast(int32 cx, int32 cy) {
	_fast[cy][cx / 32] &= ~(1 << (cx & 31));

	item_list::iterator iter = _items[cx][cy].begin();
	while (iter != _items[cx][cy].end()) {
		Item *item = *iter;
		++iter;
#ifdef VALIDATE_CHUNKS
		int32 x, y, z;
		item->getLocation(x, y, z);
		if (x / _mapChunkSize != cx || y / _mapChunkSize != cy) {
			warning("Item leaving fast area in chunk (%d, %d), should be (%d, %d)",
					cx, cy, x / _mapChunkSize, y / _mapChunkSize);
		}
#endif
		item->leaveFastArea();  // Can destroy the item
	}
}

inline void CurrentMap::clipMapChunks(int &minx, int &maxx, int &miny, int &maxy) {
	minx = CLIP(minx, 0, MAP_NUM_CHUNKS - 1);
	maxx = CLIP(maxx, 0, MAP_NUM_CHUNKS - 1);
	miny = CLIP(miny, 0, MAP_NUM_CHUNKS - 1);
	maxy = CLIP(maxy, 0, MAP_NUM_CHUNKS - 1);
}

void CurrentMap::areaSearch(UCList *itemlist, const uint8 *loopscript,
							uint32 scriptsize, const Item *check, uint16 range,
							bool recurse, int32 x, int32 y) const {
	int32 xd = 0, yd = 0;

	// if item != 0, search an area around item. Otherwise, search an area
	// around (x,y)
	if (check) {
		int32 zd;
		Point3 pt = check->getLocationAbsolute();
		x = pt.x;
		y = pt.y;
		check->getFootpadWorld(xd, yd, zd);
	}

	//
	// Original games consider items at <= range to match. Add a pixel
	// to range to match that behavior (eg, KEYPAD in Crusader : No Remorse
	// Mission 12 (map 19) at (17118,34878) uses range 6400 and a VALBOX
	// exactly 6400 from the KEYPAD.
	//
	// box size is negative in x and y, so range is from x+range to x-range.
	//
	const Box searchrange(x + range, y + range, 0, xd + range * 2 + 1, yd + range * 2 + 1, INT_MAX_VALUE);

	int minx = ((x - xd - range) / _mapChunkSize) - 1;
	int maxx = ((x + range) / _mapChunkSize) + 1;
	int miny = ((y - yd - range) / _mapChunkSize) - 1;
	int maxy = ((y + range) / _mapChunkSize) + 1;
	clipMapChunks(minx, maxx, miny, maxy);

	//
	// NOTE: Iteration order of chunks here is important for
	// usecode compatibility!
	//
	// eg, No Remorse Mission 2 has a camera which searches for
	// trigger with qlo = 5, but there are 2 of those on the map.
	// We need to return in the correct order or it triggers
	// the wrong one and breaks the game.
	//
	for (int cy = miny; cy <= maxy; cy++) {
		for (int cx = minx; cx <= maxx; cx++) {
			for (const auto *item : _items[cx][cy]) {
				if (item->hasExtFlags(Item::EXT_SPRITE))
					continue;

				// check if item is in range
				Point3 pt = item->getLocation();
				if (searchrange.containsXY(pt.x, pt.y)) {
					// check item against loopscript
					if (item->checkLoopScript(loopscript, scriptsize)) {
						assert(itemlist->getElementSize() == 2);
						itemlist->appenduint16(item->getObjId());
					}

					if (recurse) {
						// recurse into child-containers
						const Container *container = dynamic_cast<const Container *>(item);
						if (container)
							container->containerSearch(itemlist, loopscript,
													   scriptsize, recurse);
					}
				}
			}
		}
	}
}

void CurrentMap::surfaceSearch(UCList *itemlist, const uint8 *loopscript,
							   uint32 scriptsize, const Item *check,
							   bool above, bool below, bool recurse) const {
	int32 xd, yd, zd;
	Point3 pt = check->getLocationAbsolute();
	check->getFootpadWorld(xd, yd, zd);
	const Box searchrange(pt.x, pt.y, pt.z, xd, yd, zd);

	int minx = ((pt.x - xd) / _mapChunkSize) - 1;
	int maxx = (pt.x / _mapChunkSize) + 1;
	int miny = ((pt.y - yd) / _mapChunkSize) - 1;
	int maxy = (pt.y / _mapChunkSize) + 1;
	clipMapChunks(minx, maxx, miny, maxy);

	for (int cy = miny; cy <= maxy; cy++) {
		for (int cx = minx; cx <= maxx; cx++) {
			for (const auto *item : _items[cx][cy]) {
				if (item->getObjId() == check->getObjId())
					continue;
				if (item->hasExtFlags(Item::EXT_SPRITE))
					continue;

				// check if item is in range?
				const Box ib = item->getWorldBox();
				if (searchrange.overlapsXY(ib)) {
					bool ok = false;

					if (above && ib._z == (searchrange._z + searchrange._zd)) {
						ok = true;
						// Only recursive if tops aren't same (i.e. NOT flat)
						if (recurse && (ib._zd + ib._z != searchrange._z + searchrange._zd))
							surfaceSearch(itemlist, loopscript, scriptsize, item, true, false, true);
					}

					if (below && searchrange._z == (ib._z + ib._zd)) {
						ok = true;
						// Only recursive if bottoms aren't same (i.e. NOT flat)
						if (recurse && (ib._z != searchrange._z))
							surfaceSearch(itemlist, loopscript, scriptsize, item, false, true, true);
					}

					if (ok) {
						// check item against loopscript
						if (item->checkLoopScript(loopscript, scriptsize)) {
							assert(itemlist->getElementSize() == 2);
							itemlist->appenduint16(item->getObjId());
						}
					}
				}
			}
		}
	}
}

TeleportEgg *CurrentMap::findDestination(uint16 id) {
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; i++) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS; j++) {
			for (auto *item : _items[i][j]) {
				TeleportEgg *egg = dynamic_cast<TeleportEgg *>(item);
				if (egg) {
					if (!egg->isTeleporter() && egg->getTeleportId() == id)
						return egg;
				}
			}
		}
	}
	return nullptr;
}

const Std::list<Item *> *CurrentMap::getItemList(int32 gx, int32 gy) const {
	if (gx < 0 || gy < 0 || gx >= MAP_NUM_CHUNKS || gy >= MAP_NUM_CHUNKS)
		return nullptr;
	return &_items[gx][gy];
}

PositionInfo CurrentMap::getPositionInfo(int32 x, int32 y, int32 z, uint32 shape, ObjId id) const {
	const ShapeInfo *si = GameData::get_instance()->getMainShapes()->getShapeInfo(shape);
	int32 xd, yd, zd;
	// Note: this assumes the shape to be placed is not flipped
	si->getFootpadWorld(xd, yd, zd, 0);
	Box target(x, y, z, xd, yd, zd);
	Box empty;

	return getPositionInfo(target, empty, si->_flags, id);
}

PositionInfo CurrentMap::getPositionInfo(const Box &target, const Box &start, uint32 shapeflags, ObjId id) const {
	PositionInfo info;
	static const uint32 flagmask = (ShapeInfo::SI_SOLID | ShapeInfo::SI_DAMAGING | ShapeInfo::SI_LAND | ShapeInfo::SI_ROOF);
	static const uint32 supportmask = (ShapeInfo::SI_SOLID | ShapeInfo::SI_LAND | ShapeInfo::SI_ROOF);
	static const uint32 landmask = (ShapeInfo::SI_LAND | ShapeInfo::SI_ROOF);
	static const uint32 blockmask = (ShapeInfo::SI_SOLID | ShapeInfo::SI_DAMAGING);

	int32 supportz = INT_MIN_VALUE;
	int32 landz = INT_MIN_VALUE;
	int32 roofz = INT_MAX_VALUE;

	int32 midx = target._x - target._xd / 2;
	int32 midy = target._y - target._yd / 2;

	int minx = ((target._x - target._xd) / _mapChunkSize) - 1;
	int maxx = (target._x / _mapChunkSize) + 1;
	int miny = ((target._y - target._yd) / _mapChunkSize) - 1;
	int maxy = (target._y / _mapChunkSize) + 1;
	clipMapChunks(minx, maxx, miny, maxy);

	for (int cx = minx; cx <= maxx; cx++) {
		for (int cy = miny; cy <= maxy; cy++) {
			for (const auto *item : _items[cx][cy]) {
				if (item->getObjId() == id)
					continue;
				if (item->hasExtFlags(Item::EXT_SPRITE))
					continue;

				const ShapeInfo *si = item->getShapeInfo();
				if (!(si->_flags & flagmask))
					continue; // not an interesting item

				Box ib = item->getWorldBox();

				// check overlap
				if ((si->_flags & shapeflags & blockmask) &&
					target.overlaps(ib) && !start.overlaps(ib)) {
					// overlapping an item. Invalid position
#if 0
					debugC(kDebugObject, "%s", item->dumpInfo().c_str());
#endif
					if (info.blocker == nullptr) {
						info.blocker = item;
					}
				}

				if (target.overlapsXY(ib)) {
					// check support
					if (si->_flags & supportmask && ib._z + ib._zd > supportz && ib._z + ib._zd <= target._z) {
						supportz = ib._z + ib._zd;
					}

					// check roof
					if (si->is_roof() && ib._z < roofz && ib._z >= target._z + target._zd) {
						info.roof = item;
						roofz = ib._z;
					}
				}

				// check bottom center
				if (ib.isBelow(midx, midy, target._z)) {
					// check land
					if (si->_flags & landmask && ib._z + ib._zd > landz) {
						info.land = item;
						landz = ib._z + ib._zd;
					}
				}
			}
		}
	}

	info.valid = info.blocker == nullptr;
	// Partial support allowed if land is close. Allow up to 8 to match the
	// position adjustments in scanForValidPosition.  In Crusader, we don't
	// require land - just support.
	if (supportz == target._z && (landz + 8 >= target._z || GAME_IS_CRUSADER))
		info.supported = true;

	// Mark supported at minimum z
	if (target._z <= 0)
		info.supported = true;

	return info;
}

bool CurrentMap::scanForValidPosition(int32 x, int32 y, int32 z, const Item *item,
									  Direction movedir, bool wantsupport,
									  int32 &tx, int32 &ty, int32 &tz) {
	// TODO: clean this up. Currently the mask arrays are filled with more
	// data than is actually used.
	const uint32 blockflagmask = (ShapeInfo::SI_SOLID | ShapeInfo::SI_DAMAGING) & item->getShapeInfo()->_flags;
	if (!blockflagmask)
		warning("scanForValidPosition non-solid object, should be valid anywhere");

	Direction searchdir = static_cast<Direction>(((int)movedir + 4) % 8);

	bool xdir = (Direction_XFactor(searchdir) != 0);
	bool ydir = (Direction_YFactor(searchdir) != 0);

	int searchtype = ((int)searchdir / 2);

	// The number of grid points on each side of x/y/z to scan.
	const int scansize = GAME_IS_CRUSADER ? 10 : 8;

	// Mark everything as valid, but without support.  We only use SCANSIZE * 2 + 1 bits of the mask
	// but fill them all for simplicity.  Mask arrays are bigger than needed for either game.
	uint32 validmask[24];
	uint32 supportmask[24];
	for (int i = 0; i < scansize * 2 + 1; ++i) {
		validmask[i] = 0xFFFFFFFF;
		supportmask[i] = 0;
	}

	int32 xd, yd, zd;
	item->getFootpadWorld(xd, yd, zd);

	// Note on scan direction:
	// The 'horiz' variable below will always mean a direction in
	// the positive  x/y directions, with the exception of searchtype 1,
	// in which case positive horiz points in the (positive x, negative y)
	// direction.

	// next, we'll loop over all objects in the area, and mark the areas
	// overlapped and supported by each object

	int minx = ((x - xd) / _mapChunkSize) - 1;
	int maxx = (x / _mapChunkSize) + 1;
	int miny = ((y - yd) / _mapChunkSize) - 1;
	int maxy = (y / _mapChunkSize) + 1;
	clipMapChunks(minx, maxx, miny, maxy);

	for (int cx = minx; cx <= maxx; cx++) {
		for (int cy = miny; cy <= maxy; cy++) {
			for (const auto *citem : _items[cx][cy]) {
				if (citem->getObjId() == item->getObjId())
					continue;
				if (citem->hasExtFlags(Item::EXT_SPRITE))
					continue;

				const ShapeInfo *si = citem->getShapeInfo();
				//!! need to check is_sea() and is_land() maybe?
				if (!(si->_flags & blockflagmask))
					continue; // not an interesting item

				int32 ixd, iyd, izd;
				Point3 pt = citem->getLocation();
				citem->getFootpadWorld(ixd, iyd, izd);

				int minv = pt.z - z - zd + 1;
				int maxv = pt.z + izd - z - 1;
				if (minv < -scansize) minv = -scansize;
				if (maxv > scansize) maxv = scansize;

				int sminx = pt.x - ixd + 1 - x;
				int smaxx = pt.x + xd - 1  - x;
				int sminy = pt.y - iyd + 1 - y;
				int smaxy = pt.y + yd - 1  - y;

				int minh = -100;
				int maxh = 100;
				if (!xdir && (sminx > 0 || smaxx < 0))
					continue;
				if (!ydir && (sminy > 0 || smaxy < 0))
					continue;

				if (xdir && minh < sminx)
					minh = sminx;
				if (xdir && maxh > smaxx)
					maxh = smaxx;
				if ((ydir && searchtype != 1) && minh < sminy)
					minh = sminy;
				if ((ydir && searchtype != 1) && maxh > smaxy)
					maxh = smaxy;
				if (searchtype == 1 && minh < -smaxy)
					minh = -smaxy;
				if (searchtype == 1 && maxh > -sminy)
					maxh = -sminy;

				if (minh < -scansize) minh = -scansize;
				if (maxh > scansize) maxh = scansize;

				for (int j = minv; j <= maxv; ++j)
					for (int i = minh; i <= maxh; ++i)
						validmask[j + scansize] &= ~(1 << (i + scansize));

				if (wantsupport && si->is_solid() &&
				        pt.z + izd >= z - scansize && pt.z + izd <= z + scansize) {
					for (int i = minh; i <= maxh; ++i)
						supportmask[pt.z + izd - z + scansize] |= (1 << (i + scansize));

				}
			}
		}
	}

	bool foundunsupported = false;

#if 0
	debugC(kDebugCollision, "valid | support")
	for (unsigned int i = 0; i < SCANSIZE * 2 + 1; ++i) {
		debugC(kDebugCollision, "%05x | %05x", validmask[SCANSIZE * 2 - i], supportmask[SCANSIZE * 2 - i]);
	}
	debugC(kDebugCollision, ("-----------");
#endif

	//
	// Crusader also has some floor pieces and elevators which are 1 z-pixel offset,
	// presumably to fix render-order issues.  We need to be able to step up/down
	// to them smoothly, so check 1 px either side of the step sizes as well.  Also
	// check +/- 2 px which can happen on walkways
	//
	static const int SEARCH_OFFSETS_U8[] = {0, -4, 4, -8, 8};
	static const int SEARCH_OFFSETS_CRU[] = {0, -1, 1, -2, 2, -3, 3, -4, 4, -5, 5, -7, 7, -8, 8, -9, 9};

	const int *search_offsets = GAME_IS_CRUSADER ? SEARCH_OFFSETS_CRU : SEARCH_OFFSETS_U8;
	const unsigned int nhoriz = GAME_IS_CRUSADER ? 11 : 3;
	const unsigned int nvert = GAME_IS_CRUSADER ?
		ARRAYSIZE(SEARCH_OFFSETS_CRU) : ARRAYSIZE(SEARCH_OFFSETS_U8);
	for (unsigned int i = 0; i < nhoriz; ++i) {
		const int horiz = search_offsets[i];

		for (unsigned int j = 0; j < nvert; ++j) {
			const int vert = search_offsets[j];
			const uint32 checkbit = (1 << (horiz + scansize));

			if (validmask[vert + scansize] & checkbit) {
				if (!wantsupport || !foundunsupported ||
				        (supportmask[vert + scansize] & checkbit)) {
					tz = z + vert;
					tx = x;
					if (searchtype != 0)
						tx += horiz;
					ty = y;
					if (searchtype == 1)
						ty -= horiz;
					else if (searchtype != 2)
						ty += horiz;
				}
				if (!wantsupport || (supportmask[vert + scansize] & checkbit))
					return true;
				foundunsupported = true;
			}
		}
	}

	// no supported location found, so return unsupported unblocked one
	if (foundunsupported)
		return true;

	return false;
}


// Do a sweepTest of an item from start to end point.
// dims is the bounding box size.
// item is the item that we are checking to move
// blocking_only forces us to check against blocking items only.
// Returns item hit or 0 if no hit.
// end is set to the colision point
bool CurrentMap::sweepTest(const Point3 &start, const Point3 &end,
						   const int32 dims[3], uint32 shapeflags,
						   ObjId item, bool blocking_only,
						   Std::list<SweepItem> *hit) const {
	const uint32 blockflagmask = (ShapeInfo::SI_SOLID | ShapeInfo::SI_DAMAGING | ShapeInfo::SI_LAND);

	int minx = ((start.x - dims[0]) / _mapChunkSize) - 1;
	int maxx = (start.x / _mapChunkSize) + 1;
	int miny = ((start.y - dims[1]) / _mapChunkSize) - 1;
	int maxy = (start.y / _mapChunkSize) + 1;

	{
		int dminx = ((end.x - dims[0]) / _mapChunkSize) - 1;
		int dmaxx = (end.x / _mapChunkSize) + 1;
		int dminy = ((end.y - dims[1]) / _mapChunkSize) - 1;
		int dmaxy = (end.y / _mapChunkSize) + 1;
		if (dminx < minx)
			minx = dminx;
		if (dmaxx > maxx)
			maxx = dmaxx;
		if (dminy < miny)
			miny = dminy;
		if (dmaxy > maxy)
			maxy = dmaxy;
	}

	clipMapChunks(minx, maxx, miny, maxy);

	// Get velocity, extents, and centre of item
	int32 vel[3];
	int32 ext[3];
	Point3 centre;
	vel[0] = end.x - start.x;
	vel[1] = end.y - start.y;
	vel[2] = end.z - start.z;

	ext[0] = dims[0] / 2;
	ext[1] = dims[1] / 2;
	ext[2] = dims[2] / 2;

	centre.x = start.x - ext[0];
	centre.y = start.y - ext[1];
	// Z is opposite direction to x and y..
	centre.z = start.z + ext[2];

	debugC(kDebugCollision, "Sweeping from (%d, %d, %d) - (%d, %d, %d) to (%d, %d, %d) - (%d, %d, %d)",
		   -ext[0], -ext[1], -ext[2],
		   ext[0], ext[1], ext[2],
		   vel[0] - ext[0], vel[1] - ext[1], vel[2] - ext[2],
		   vel[0] + ext[0], vel[1] + ext[1], vel[2] + ext[2]);

	Std::list<SweepItem>::iterator sw_it;
	if (hit) sw_it = hit->end();

	for (int cx = minx; cx <= maxx; cx++) {
		for (int cy = miny; cy <= maxy; cy++) {
			for (const auto *other_item : _items[cx][cy]) {
				if (other_item->getObjId() == item)
					continue;
				if (other_item->hasExtFlags(Item::EXT_SPRITE))
					continue;

				uint32 othershapeflags = other_item->getShapeInfo()->_flags;
				bool blocking = (othershapeflags & shapeflags &
				                 blockflagmask) != 0;

				// This WILL hit everything and return them unless
				// blocking_only is set
				if (blocking_only && !blocking)
					continue;

				int32 other[3], oext[3];
				Point3 opt = other_item->getLocation();
				other[0] = opt.x;
				other[1] = opt.y;
				other[2] = opt.z;
				other_item->getFootpadWorld(oext[0], oext[1], oext[2]);

				// If the objects overlapped at the start, ignore collision.
				// The -1 and +1 portions are to still consider collisions
				// for items which were merely touching at the start for all
				// intents and purposes, but partially overlapped due to an
				// off-by-one error (hypothetically, but they do happen so
				// protect against it).
				if ( /* not non-overlapping start position */
				    !(start.x <= other[0] - (oext[0] - 1) ||
				      start.x - dims[0] >= other[0] - 1 ||
				      start.y <= other[1] - (oext[1] - 1) ||
				      start.y - dims[1] >= other[1] - 1 ||
				      start.z + dims[2] <= other[2] + 1 ||
				      start.z >= other[2] + (oext[2] - 1))) {
					// Overlapped at the start, and not just touching so
					// ignore collision
					continue;
				}

				// Make oext the distance to midpoint in each dim
				oext[0] /= 2;
				oext[1] /= 2;
				oext[2] /= 2;

				// Put other into our coord frame
				other[0] -= oext[0] + centre.x;
				other[1] -= oext[1] + centre.y;
				other[2] += oext[2] - centre.z;

				//first times of overlap along each axis
				int32 u_1[3] = {0, 0, 0};

				//last times of overlap along each axis
				int32 u_0[3] = {0x4000, 0x4000, 0x4000}; // CONSTANTS

				bool touch = false;
				bool touch_floor = false;

				//find the possible first and last times
				//of overlap along each axis
				for (int i = 0 ; i < 3; i++) {
					int32 A_max = ext[i];
					int32 A_min = -ext[i];
					int32 B_max = other[i] + oext[i];
					int32 B_min = other[i] - oext[i];

					if (vel[i] < 0 && A_max >= B_min) {      // A_max>=B_min not required
						// Special case: if moving item has zero height and
						// other item is a 128x128 flat, then moving item is
						// considered blocked by the flat
						// FIXME: it might be better to make this extra check
						// identical to the 'flats' special case in ItemSorter
						if (A_max == B_min && !
						        (i == 2 && ext[i] == 0 && oext[i] == 0 &&
						         oext[0] == 64 && oext[1] == 64))
							touch = true; // touch at start
						if (A_min + vel[i] == B_max) {
							touch = true; // touch at end
							if (i == 2)
								touch_floor = true;
						}

						// - want to know when rear of A passes front of B
						u_0[i] = ((B_max - A_min) * 0x4000) / vel[i];
						// - want to know when front of A passes rear of B
						u_1[i] = ((B_min - A_max) * 0x4000) / vel[i];
					} else if (vel[i] > 0 && A_min <= B_max) { // A_min<=B_max not required
						if (A_min == B_max)
							touch = true; // touch at start
						if (A_max + vel[i] == B_min)
							touch = true; // touch at end

						// + want to know when front of A passes rear of B
						u_0[i] = ((B_min - A_max) * 0x4000) / vel[i];
						// + want to know when rear of A passes front of B
						u_1[i] = ((B_max - A_min) * 0x4000) / vel[i];
					} else if (vel[i] == 0 && A_max >= B_min && A_min <= B_max) {
						if (A_min == B_max || A_max == B_min)
							touch = true;
						if (i == 2 && A_min == B_max)
							touch_floor = true;

						u_0[i] = -1;
						u_1[i] = 0x4000;
					} else {
						u_0[i] = 0x4001;
						u_1[i] = -1;
					}

					if (u_1[i] >= u_0[i] && (u_0[i] > 0x4000 || u_1[i] < 0)) {
						u_0[i] = 0x4001;
						u_1[i] = -1;
					}
				}

				//possible first time of overlap
				int32 first = u_0[0];
				if (u_0[1] > first) first = u_0[1];
				if (u_0[2] > first) first = u_0[2];

				//possible last time of overlap
				int32 last = u_1[0];
				if (u_1[1] < last) last = u_1[1];
				if (u_1[2] < last) last = u_1[2];

				// store directions in which we're being blocked
				uint8 dirs = 0;
				for (int i = 0; i <= 2; ++i) {
					if (first == u_0[i])
						dirs |= (1 << i);
				}

				//they could have only collided if
				//the first time of overlap occurred
				//before the last time of overlap
				if (first <= last) {
					if (!hit)
						return true;

					// Clamp
					if (first < -1) first = -1;
					if (last > 0x4000) last = 0x4000;

					// Ok, what we want to do here is add to the list.
					// Sorted by _hitTime.

					// Small speed up.
					if (sw_it != hit->end()) {
						if (sw_it->_hitTime > first)
							sw_it = hit->begin();
					} else
						sw_it = hit->begin();

					for (; sw_it != hit->end(); ++sw_it) {
						if (sw_it->_hitTime > first ||
							(sw_it->_hitTime == first && sw_it->_endTime > last))
							break;
					}

					// Now add it
					hit->insert(sw_it, SweepItem(other_item->getObjId(), first, last, touch, touch_floor, blocking, dirs));

					//debugC(kDebugCollision, "Hit item %u (%d, %d, %d) at first: %d, last: %d",
					//	   other_item->getObjId(), other[0], other[1], other[2], first, last);
					//debugC(kDebugCollision, "hit item time (%d-%d) (%d-%d) (%d-%d)",
					//	u_0[0], u_1[0], u_0[1], u_1[1], u_0[2], u_1[2]);
					//debugC(kDebugCollision, "touch: %d, floor: %d, block: %d", touch, touch_floor, blocking);
				}
			}
		}
	}

	return hit && hit->size();
}

void CurrentMap::setFastAtPoint(const Point3 &pt) {
	int32 cx = pt.x / _mapChunkSize;
	int32 cy = pt.y / _mapChunkSize;

	if (!isChunkFast(cx, cy))
		setChunkFast(cx, cy);
}

void CurrentMap::setWholeMapFast() {
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; ++i) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS; ++j) {
			if (!isChunkFast(j, i))
				setChunkFast(j, i);
		}
	}

	_fastXMin = -1;
	_fastYMin = -1;
	_fastXMax = -1;
	_fastYMax = -1;
}

void CurrentMap::save(Common::WriteStream *ws) {
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; ++i) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS / 32; ++j) {
			ws->writeUint32LE(_fast[i][j]);
		}
	}

	if (GAME_IS_CRUSADER) {
		for (int i = 0; i < MAP_NUM_TARGET_ITEMS; i++)
			ws->writeUint16LE(_targets[i]);
	}
}

bool CurrentMap::load(Common::ReadStream *rs, uint32 version) {
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; ++i) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS / 32; ++j) {
			_fast[i][j] = rs->readUint32LE();
		}
	}

	_fastXMin = -1;
	_fastYMin = -1;
	_fastXMax = -1;
	_fastYMax = -1;

	if (GAME_IS_CRUSADER) {
		for (int i = 0; i < MAP_NUM_TARGET_ITEMS; i++)
			_targets[i] = rs->readUint16LE();
	}

	return true;
}

uint32 CurrentMap::I_canExistAt(const uint8 *args, unsigned int argsize) {
	ARG_UINT16(shape);
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT8(z);
	if (argsize > 8) {
		//!! TODO: figure these out
		ARG_NULL16(); // is either 1 or 4. moves?
		ARG_NULL16(); // some objid?
		ARG_NULL16(); // always zero
	}

	World_FromUsecodeXY(x, y);

	//
	// TODO: The crusader version of this function actually checks by
	// changing the shapeinfo of shape 0x31A to match the target
	// shape.  For a first level approximation, this is the same.
	//

	const CurrentMap *cm = World::get_instance()->getCurrentMap();
	PositionInfo info = cm->getPositionInfo(x, y, z, shape, 0);

	if (info.valid)
		return 1;
	else
		return 0;
}

uint32 CurrentMap::I_canExistAtPoint(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(other);
	ARG_UINT16(shape);
	ARG_WORLDPOINT(pt);

	if (other) {
		debugC(kDebugObject, "I_canExistAtPoint other object: %s", other->dumpInfo().c_str());
	} else {
		debugC(kDebugObject, "I_canExistAtPoint other object null.");
	}
	if (shape > 0x800)
		return 0;

	int32 x = pt.getX();
	int32 y = pt.getY();
	int32 z = pt.getZ();

	World_FromUsecodeXY(x, y);

	const CurrentMap *cm = World::get_instance()->getCurrentMap();
	PositionInfo info = cm->getPositionInfo(x, y, z, shape, 0);

	if (info.valid)
		return 1;
	else
		return 0;
}


} // End of namespace Ultima8
} // End of namespace Ultima
