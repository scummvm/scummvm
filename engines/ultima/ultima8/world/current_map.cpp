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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/map.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/glob_egg.h"
#include "ultima/ultima8/world/egg.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/world_point.h"
#include "ultima/ultima8/world/container.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/world/teleport_egg.h"
#include "ultima/ultima8/world/egg_hatcher_process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/main_shape_archive.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/gumps/game_map_gump.h"
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/misc/rect.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

typedef Std::list<Item *> item_list;

static const int INT_MAX_VALUE = 0x7fffffff;

CurrentMap::CurrentMap() : _currentMap(0), _eggHatcher(0),
	  _fastXMin(-1), _fastYMin(-1), _fastXMax(-1), _fastYMax(-1) {
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; i++) {
		Std::memset(_fast[i], false, sizeof(uint32)*MAP_NUM_CHUNKS / 32);
	}

	if (GAME_IS_U8) {
		_mapChunkSize = 512;
	} else if (GAME_IS_CRUSADER) {
		_mapChunkSize = 1024;
	} else {
		CANT_HAPPEN_MSG("Unknown game type in CurrentMap constructor.");
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
			item_list::iterator iter;
			for (iter = _items[i][j].begin(); iter != _items[i][j].end(); ++iter)
				delete *iter;
			_items[i][j].clear();
		}
		Std::memset(_fast[i], false, sizeof(uint32)*MAP_NUM_CHUNKS / 32);
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
			item_list::iterator iter;
			for (iter = _items[i][j].begin(); iter != _items[i][j].end(); ++iter) {
				Item *item = *iter;

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
	item_list::const_iterator iter;
	for (iter = itemlist.begin(); iter != itemlist.end(); ++iter) {
		Item *item = *iter;

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
		Std::memset(_fast[i], false, sizeof(uint32)*MAP_NUM_CHUNKS / 32);
	}
	_fastXMin = -1;
	_fastYMin = -1;
	_fastXMax = -1;
	_fastYMax = -1;

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
	int32 ix, iy, iz;

	item->getLocation(ix, iy, iz);

	if (ix < 0 || ix >= _mapChunkSize * MAP_NUM_CHUNKS ||
	        iy < 0 || iy >= _mapChunkSize * MAP_NUM_CHUNKS) {
		perr << "Skipping item " << item->getObjId() << ": out of range ("
		     << ix << "," << iy << ")" << Std::endl;
		return;
	}

	int32 cx = ix / _mapChunkSize;
	int32 cy = iy / _mapChunkSize;

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
	int32 ix, iy, iz;

	item->getLocation(ix, iy, iz);

	if (ix < 0 || ix >= _mapChunkSize * MAP_NUM_CHUNKS ||
	        iy < 0 || iy >= _mapChunkSize * MAP_NUM_CHUNKS) {
		perr << "Skipping item " << item->getObjId() << ": out of range ("
		     << ix << "," << iy << ")" << Std::endl;
		return;
	}

	int32 cx = ix / _mapChunkSize;
	int32 cy = iy / _mapChunkSize;

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
	int32 ix, iy, iz;

	item->getLocation(ix, iy, iz);

	removeItemFromList(item, ix, iy);
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


Item *CurrentMap::findBestTargetItem(int32 x, int32 y, Direction dir, DirectionMode dirmode) {
	// "best" means:
	// Shape info SI_OCCL
	// isNPC
	// Closest
	// in that order.
	bool bestisnpc = false;
	bool bestisoccl = false;
	Item *bestitem = nullptr;
	int bestdist = 0xffff;

	for (int i = 0; i < MAP_NUM_TARGET_ITEMS; i++) {
		if (_targets[i] == 0)
			continue;
		Item *item = getItem(_targets[i]);
		// FIXME: this should probably always be non-null,
		// but if it's not the item disappeared - remove it from the list.
		// If we fix this, this function can be const.
		if (!item) {
			_targets[i] = 0;
			continue;
		}
		const ShapeInfo *si = item->getShapeInfo();
		bool isoccl = si->_flags & ShapeInfo::SI_OCCL;

		int32 ix, iy, iz;
		item->getLocation(ix, iy, iz);
		Direction itemdir = Direction_GetWorldDir(iy - y, ix - x, dirmode);
		if (itemdir != dir)
			continue;

		const Actor *actor = dynamic_cast<const Actor *>(item);
		if ((bestisoccl && !isoccl) || (bestisnpc && !actor) || !item->isOnScreen())
			continue;

		int xdiff = abs(x - ix);
		int ydiff = abs(y - iy);
		int dist = MAX(xdiff, ydiff);

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
		perr << "Skipping item " << item->getObjId() << ": out of range ("
		     << oldx << "," << oldy << ")" << Std::endl;
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
	int32 scy = ((cx * mapChunkSize + cy * mapChunkSize) / 8);

	// Screenspace bounding box left extent    (LNT x coord)
	int32 cxleft = scx - mapChunkSize / 4;
	// Screenspace bounding box right extent   (RFT x coord)
	int32 cxright = scx + mapChunkSize / 4;

	// Screenspace bounding box top extent     (LFT y coord)
	int32 cytop = scy - 256;
	// Screenspace bounding box bottom extent  (RNB y coord)
	int32 cybot = scy + 128;

	const bool right_clear = cxright <= sleft;
	const bool left_clear = cxleft >= sright;
	const bool top_clear = cytop >= sbot;
	const bool bot_clear = cybot <= stop;

	const bool clear = right_clear | left_clear | top_clear | bot_clear;

	return !clear;
}

static inline void CalcFastAreaLimits(int32 &sx_limit,
                                      int32 &sy_limit,
                                      int32 &xy_limit,
                                      const Rect &dims) {
	// By default the fastArea is the screensize plus a border of no more
	// than 256 pixels wide and 384 pixels high
	// dims.w and dims.h need to be divided by 2 for crusader
	sx_limit = dims.width() / 256 + 3;
	sy_limit = dims.height() / 128 + 7;
	xy_limit = (sy_limit + sx_limit) / 2;
}

void CurrentMap::updateFastArea(int32 from_x, int32 from_y, int32 from_z, int32 to_x, int32 to_y, int32 to_z) {
	int x_min = MIN(from_x, to_x);
	int x_max = MAX(from_x, to_x);

	int y_min = MIN(from_y, to_y);
	int y_max = MAX(from_y, to_y);

	int z_min = MIN(from_z, to_z);
	int z_max = MAX(from_z, to_z);

	// Work out Fine (screenspace) Limits of chunks with half chunk border
	Rect dims;
	Ultima8Engine::get_instance()->getGameMapGump()->GetDims(dims);

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

	CalcFastAreaLimits(sx_limit, sy_limit, xy_limit, dims);

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

	item_list::iterator iter;
	for (iter = _items[cx][cy].begin();
	        iter != _items[cx][cy].end(); ++iter) {
		(*iter)->enterFastArea();
	}
}

void CurrentMap::unsetChunkFast(int32 cx, int32 cy) {
	_fast[cy][cx / 32] &= ~(1 << (cx & 31));

	item_list::iterator iter = _items[cx][cy].begin();
	while (iter != _items[cx][cy].end()) {
		Item *item = *iter;
		++iter;
		item->leaveFastArea();  // Can destroy the item
	}
}

void CurrentMap::clipMapChunks(int &minx, int &maxx, int &miny, int &maxy) const {
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
		int32 z, zd;
		check->getLocationAbsolute(x, y, z);
		check->getFootpadWorld(xd, yd, zd);
	}

	const Rect searchrange(x - xd - range, y - yd - range, x + range, y + range);

	int minx = ((x - xd - range) / _mapChunkSize) - 1;
	int maxx = ((x + range) / _mapChunkSize) + 1;
	int miny = ((y - yd - range) / _mapChunkSize) - 1;
	int maxy = ((y + range) / _mapChunkSize) + 1;
	clipMapChunks(minx, maxx, miny, maxy);

	for (int cx = minx; cx <= maxx; cx++) {
		for (int cy = miny; cy <= maxy; cy++) {
			item_list::const_iterator iter;
			for (iter = _items[cx][cy].begin();
			        iter != _items[cx][cy].end(); ++iter) {

				const Item *item = *iter;

				if (item->hasExtFlags(Item::EXT_SPRITE))
					continue;

				// check if item is in range?
				int32 ix, iy, iz;
				item->getLocation(ix, iy, iz);

				int32 ixd, iyd, izd;
				item->getFootpadWorld(ixd, iyd, izd);

				const Rect itemrect(ix - ixd, iy - iyd, ix, iy);

				if (!itemrect.intersects(searchrange))
					continue;

				// check item against loopscript
				if (item->checkLoopScript(loopscript, scriptsize)) {
					assert(itemlist->getElementSize() == 2);
					uint16 objid = item->getObjId();
					uint8 buf[2];
					buf[0] = static_cast<uint8>(objid);
					buf[1] = static_cast<uint8>(objid >> 8);
					itemlist->append(buf);
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

void CurrentMap::surfaceSearch(UCList *itemlist, const uint8 *loopscript,
                               uint32 scriptsize, const Item *check,
							   bool above, bool below, bool recurse) const {
	int32 origin[3];
	int32 dims[3];
	check->getLocationAbsolute(origin[0], origin[1], origin[2]);
	check->getFootpadWorld(dims[0], dims[1], dims[2]);
	surfaceSearch(itemlist, loopscript, scriptsize, check->getObjId(),
	              origin, dims, above, below, recurse);
}

void CurrentMap::surfaceSearch(UCList *itemlist, const uint8 *loopscript,
                               uint32 scriptsize, ObjId check,
                               int32 origin[3], int32 dims[3],
                               bool above, bool below, bool recurse) const {
	const Rect searchrange(origin[0] - dims[0], origin[1] - dims[1],
	                       origin[0], origin[1]);

	int minx = ((origin[0] - dims[0]) / _mapChunkSize) - 1;
	int maxx = ((origin[0]) / _mapChunkSize) + 1;
	int miny = ((origin[1] - dims[1]) / _mapChunkSize) - 1;
	int maxy = ((origin[1]) / _mapChunkSize) + 1;
	clipMapChunks(minx, maxx, miny, maxy);

	for (int cx = minx; cx <= maxx; cx++) {
		for (int cy = miny; cy <= maxy; cy++) {
			item_list::const_iterator iter;
			for (iter = _items[cx][cy].begin();
			        iter != _items[cx][cy].end(); ++iter) {

				const Item *item = *iter;

				if (item->getObjId() == check)
					continue;
				if (item->hasExtFlags(Item::EXT_SPRITE))
					continue;

				// check if item is in range?
				int32 ix, iy, iz;
				item->getLocation(ix, iy, iz);
				int32 ixd, iyd, izd;
				item->getFootpadWorld(ixd, iyd, izd);

				const Rect itemrect(ix - ixd, iy - iyd, ix, iy);

				if (!itemrect.intersects(searchrange))
					continue;

				bool ok = false;

				if (above && iz == (origin[2] + dims[2])) {
					ok = true;
					// Only recursive if tops aren't same (i.e. NOT flat)
					if (recurse && (izd + iz != origin[2] + dims[2]))
						surfaceSearch(itemlist, loopscript, scriptsize, item, true, false, true);
				}

				if (below && origin[2] == (iz + izd)) {
					ok = true;
					// Only recursive if bottoms aren't same (i.e. NOT flat)
					if (recurse && (izd != dims[2]))
						surfaceSearch(itemlist, loopscript, scriptsize, item, false, true, true);
				}

				if (!ok)
					continue;

				// check item against loopscript
				if (item->checkLoopScript(loopscript, scriptsize)) {
					assert(itemlist->getElementSize() == 2);
					uint16 objid = item->getObjId();
					uint8 buf[2];
					buf[0] = static_cast<uint8>(objid);
					buf[1] = static_cast<uint8>(objid >> 8);
					itemlist->append(buf);
				}
			}
		}
	}
}

TeleportEgg *CurrentMap::findDestination(uint16 id) {
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; i++) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS; j++) {
			item_list::iterator iter;
			for (iter = _items[i][j].begin();
			        iter != _items[i][j].end(); ++iter) {
				TeleportEgg *egg = dynamic_cast<TeleportEgg *>(*iter);
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


bool CurrentMap::isValidPosition(int32 x, int32 y, int32 z,
                                 uint32 shape,
                                 ObjId item, const Item **support,
                                 ObjId *roof, const Item **blocker) const {
	const ShapeInfo *si = GameData::get_instance()->
	                getMainShapes()->getShapeInfo(shape);
	int32 xd, yd, zd;
	// Note: this assumes the shape to be placed is not flipped
	si->getFootpadWorld(xd, yd, zd, 0);

	return isValidPosition(x, y, z,
	                       INT_MAX_VALUE / 2, INT_MAX_VALUE / 2, INT_MAX_VALUE / 2,
	                       xd, yd, zd,
	                       si->_flags, item, support, roof, blocker);
}

bool CurrentMap::isValidPosition(int32 x, int32 y, int32 z,
                                 int xd, int yd, int zd,
                                 uint32 shapeflags,
                                 ObjId item, const Item **support,
                                 ObjId *roof, const Item **blocker) const {
	return isValidPosition(x, y, z,
	                       INT_MAX_VALUE / 2, INT_MAX_VALUE / 2, INT_MAX_VALUE / 2,
	                       xd, yd, zd,
	                       shapeflags, item, support, roof, blocker);
}


bool CurrentMap::isValidPosition(int32 x, int32 y, int32 z,
                                 int32 startx, int32 starty, int32 startz,
                                 int xd, int yd, int zd,
                                 uint32 shapeflags,
                                 ObjId item_, const Item **support_,
                                 ObjId *roof_, const Item **blocker_) const {
	const uint32 flagmask = (ShapeInfo::SI_SOLID | ShapeInfo::SI_DAMAGING |
	                         ShapeInfo::SI_ROOF);
	const uint32 blockflagmask = (ShapeInfo::SI_SOLID | ShapeInfo::SI_DAMAGING);

	bool valid = true;
	const Item *support = nullptr;
	const Item *blocker = nullptr;
	ObjId roof = 0;
	int32 roofz = INT_MAX_VALUE;

	int minx = ((x - xd) / _mapChunkSize) - 1;
	int maxx = (x / _mapChunkSize) + 1;
	int miny = ((y - yd) / _mapChunkSize) - 1;
	int maxy = (y / _mapChunkSize) + 1;
	clipMapChunks(minx, maxx, miny, maxy);

	for (int cx = minx; cx <= maxx; cx++) {
		for (int cy = miny; cy <= maxy; cy++) {
			item_list::const_iterator iter;
			for (iter = _items[cx][cy].begin();
				 iter != _items[cx][cy].end(); ++iter) {
				const Item *item = *iter;
				if (item->getObjId() == item_)
					continue;
				if (item->hasExtFlags(Item::EXT_SPRITE))
					continue;

				const ShapeInfo *si = item->getShapeInfo();
				//!! need to check is_sea() and is_land() maybe?
				if (!(si->_flags & flagmask))
					continue; // not an interesting item

				int32 ix, iy, iz, ixd, iyd, izd;
				item->getFootpadWorld(ixd, iyd, izd);
				item->getLocation(ix, iy, iz);

#if 0
				if (item->getShape() == 145) {
					perr << "Shape 145: (" << ix - ixd << "," << iy - iyd << ","
					     << iz << ")-(" << ix << "," << iy << "," << iz + izd
					     << ")" << Std::endl;
					if (!si->is_solid()) perr << "not solid" << Std::endl;
				}
#endif

				// check overlap
				if ((si->_flags & shapeflags & blockflagmask) &&
				        /* not non-overlapping */
				        !(x <= ix - ixd || x - xd >= ix ||
				          y <= iy - iyd || y - yd >= iy ||
				          z + zd <= iz || z >= iz + izd) &&
				        /* non-overlapping start position */
				        (startx <= ix - ixd || startx - xd >= ix ||
				         starty <= iy - iyd || starty - yd >= iy ||
				         startz + zd <= iz || startz >= iz + izd)) {
					// overlapping an item. Invalid position
#if 0
					item->dumpInfo();
#endif
					if (blocker == nullptr) {
						blocker = item;
					}
					valid = false;
				}

				// check xy overlap
				if (!(x <= ix - ixd || x - xd >= ix ||
				      y <= iy - iyd || y - yd >= iy)) {
					// check support
					if (support == nullptr && si->is_solid() &&
					        iz + izd == z) {
						support = item;
					}

					// check roof
					if (si->is_roof() && iz < roofz && iz >= z + zd) {
						roof = item->getObjId();
						roofz = iz;
					}
				}
			}
		}
	}

	if (support_)
		*support_ = support;
	if (blocker_)
		*blocker_ = blocker;
	if (roof_)
		*roof_ = roof;

	return valid;
}

bool CurrentMap::scanForValidPosition(int32 x, int32 y, int32 z, const Item *item,
                                      Direction movedir, bool wantsupport,
                                      int32 &tx, int32 &ty, int32 &tz) {
	// TODO: clean this up. Currently the mask arrays are filled with more
	// data than is actually used.
	const uint32 blockflagmask = (ShapeInfo::SI_SOLID | ShapeInfo::SI_DAMAGING) & item->getShapeInfo()->_flags;

	Direction searchdir = static_cast<Direction>(((int)movedir + 2) % 4);

	bool xdir = (Direction_XFactor(searchdir) != 0);
	bool ydir = (Direction_YFactor(searchdir) != 0);

	// mark everything as valid, but without support
	uint32 validmask[17];
	uint32 supportmask[17];
	for (int i = 0; i < 17; ++i) {
		validmask[i] = 0x1FFFF;
		supportmask[i] = 0;
	}

	int32 xd, yd, zd;
	item->getFootpadWorld(xd, yd, zd);

	// Note on scan direction:
	// The 'horiz' variable below will always mean a direction in
	// the positive  x/y directions, with the exception of searchdir 1,
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
			item_list::const_iterator iter;
			for (iter = _items[cx][cy].begin();
			        iter != _items[cx][cy].end(); ++iter) {
				const Item *citem = *iter;
				if (citem->getObjId() == item->getObjId())
					continue;
				if (citem->hasExtFlags(Item::EXT_SPRITE))
					continue;

				const ShapeInfo *si = citem->getShapeInfo();
				//!! need to check is_sea() and is_land() maybe?
				if (!(si->_flags & blockflagmask))
					continue; // not an interesting item

				int32 ix, iy, iz, ixd, iyd, izd;
				citem->getLocation(ix, iy, iz);
				citem->getFootpadWorld(ixd, iyd, izd);

				int minv = iz - z - zd + 1;
				int maxv = iz + izd - z - 1;
				if (minv < -8) minv = -8;
				if (maxv > 8) maxv = 8;

				int sminx, smaxx, sminy, smaxy;
				sminx = ix - ixd + 1 - x;
				smaxx = ix + xd - 1  - x;
				sminy = iy - iyd + 1 - y;
				smaxy = iy + yd - 1  - y;

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
				if ((ydir && searchdir != 1) && minh < sminy)
					minh = sminy;
				if ((ydir && searchdir != 1) && maxh > smaxy)
					maxh = smaxy;
				if (searchdir == 1 && minh < -smaxy)
					minh = -smaxy;
				if (searchdir == 1 && maxh > -sminy)
					maxh = -sminy;

				if (minh < -8) minh = -8;
				if (maxh > 8) maxh = 8;

				for (int j = minv; j <= maxv; ++j)
					for (int i = minh; i <= maxh; ++i)
						validmask[j + 8] &= ~(1 << (i + 8));

				if (wantsupport && si->is_solid() &&
				        iz + izd >= z - 8 && iz + izd <= z + 8) {
					for (int i = minh; i <= maxh; ++i)
						supportmask[iz + izd - z + 8] |= (1 << (i + 8));

				}
			}
		}
	}

	bool foundunsupported = false;

#if 0
	for (unsigned int i = 0; i < 17; ++i) {
		pout.printf("%05x | %05x\n", validmask[16 - i], supportmask[16 - i]);
	}
	pout.printf("-----------\n");
#endif

	for (unsigned int i = 0; i < 3; ++i) {
		int horiz;
		if (i % 2 == 0)
			horiz = 4 * (i / 2);
		else
			horiz = -4 - 4 * (i / 2);

		for (unsigned int j = 0; j < 5; ++j) {
			int vert;
			if (j % 2 == 0)
				vert = 4 * (j / 2);
			else
				vert = -4 - 4 * (j / 2);

			if (validmask[vert + 8] & (1 << (horiz + 8))) {
				if (!wantsupport || !foundunsupported ||
				        (supportmask[vert + 8] & (1 << (horiz + 8)))) {
					tz = z + vert;
					tx = x;
					if (searchdir != 0)
						tx += horiz;
					ty = y;
					if (searchdir == 1)
						ty -= horiz;
					else if (searchdir != 2)
						ty += horiz;
				}
				if (!wantsupport || (supportmask[vert + 8] & (1 << (horiz + 8))))
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
// skip will skip all items until item num skip is reached
// Returns item hit or 0 if no hit.
// end is set to the colision point
bool CurrentMap::sweepTest(const int32 start[3], const int32 end[3],
                           const int32 dims[3], uint32 shapeflags,
                           ObjId item, bool blocking_only,
                           Std::list<SweepItem> *hit) const {
	const uint32 blockflagmask = (ShapeInfo::SI_SOLID | ShapeInfo::SI_DAMAGING);

	int minx = ((start[0] - dims[0]) / _mapChunkSize) - 1;
	int maxx = (start[0] / _mapChunkSize) + 1;
	int miny = ((start[1] - dims[1]) / _mapChunkSize) - 1;
	int maxy = (start[1] / _mapChunkSize) + 1;

	{
		int dminx = ((end[0] - dims[0]) / _mapChunkSize) - 1;
		int dmaxx = (end[0] / _mapChunkSize) + 1;
		int dminy = ((end[1] - dims[1]) / _mapChunkSize) - 1;
		int dmaxy = (end[1] / _mapChunkSize) + 1;
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
	int32 centre[3];
	for (int i = 0; i < 3; i++) {
		vel[i] = end[i] - start[i];
		ext[i] = dims[i] / 2;
		centre[i] = start[i] - ext[i];
	}
	// Z is opposite direction to x and y..
	centre[2] = start[2] + ext[2];

//	pout << "Sweeping from (" << -ext[0] << ", " << -ext[1] << ", " << -ext[2] << ")" << Std::endl;
//	pout << "              (" << ext[0] << ", " << ext[1] << ", " << ext[2] << ")" << Std::endl;
//	pout << "Sweeping to   (" << vel[0]-ext[0] << ", " << vel[1]-ext[1] << ", " << vel[2]-ext[2] << ")" << Std::endl;
//	pout << "              (" << vel[0]+ext[0] << ", " << vel[1]+ext[1] << ", " << vel[2]+ext[2] << ")" << Std::endl;

	Std::list<SweepItem>::iterator sw_it;
	if (hit) sw_it = hit->end();

	for (int cx = minx; cx <= maxx; cx++) {
		for (int cy = miny; cy <= maxy; cy++) {
			item_list::const_iterator iter;
			for (iter = _items[cx][cy].begin();
			        iter != _items[cx][cy].end(); ++iter) {
				const Item *other_item = *iter;
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
				other_item->getLocation(other[0], other[1], other[2]);
				other_item->getFootpadWorld(oext[0], oext[1], oext[2]);

				// If the objects overlapped at the start, ignore collision.
				// The -1 and +1 portions are to still consider collisions
				// for items which were merely touching at the start for all
				// intents and purposes, but partially overlapped due to an
				// off-by-one error (hypothetically, but they do happen so
				// protect against it).
				if ( /* not non-overlapping start position */
				    !(start[0] <= other[0] - (oext[0] - 1) ||
				      start[0] - dims[0] >= other[0] - 1 ||
				      start[1] <= other[1] - (oext[1] - 1) ||
				      start[1] - dims[1] >= other[1] - 1 ||
				      start[2] + dims[2] <= other[2] + 1 ||
				      start[2] >= other[2] + (oext[2] - 1))) {
					// Overlapped at the start, and not just touching so
					// ignore collision
					continue;
				}

				// Make oext the distance to midpoint in each dim
				oext[0] /= 2;
				oext[1] /= 2;
				oext[2] /= 2;

				// Put other into our coord frame
				other[0] -= oext[0] + centre[0];
				other[1] -= oext[1] + centre[1];
				other[2] += oext[2] - centre[2];

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
						if (A_min + vel[i] == B_max)
							touch = true; // touch at end

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
					//pout << "Hit item " << other_item->getObjId() << " at first: " << first << "  last: " << last << Std::endl;

					if (!hit)
						return true;

					// Clamp
					if (first < -1) first = -1;
					if (last > 0x4000) last = 0x4000;

					// Ok, what we want to do here is add to the list.
					// Sorted by _hitTime.

					// Small speed up.
					if (sw_it != hit->end()) {
						const SweepItem &si = *sw_it;
						if (si._hitTime > first) sw_it = hit->begin();
					} else
						sw_it = hit->begin();

					for (; sw_it != hit->end(); ++sw_it)
						if ((*sw_it)._hitTime > first)
							break;

					// Now add it
					sw_it = hit->insert(sw_it, SweepItem(other_item->getObjId(), first, last, touch, touch_floor, blocking, dirs));
//					pout << "Hit item " << other_item->getObjId() << " at (" << first << "," << last << ")" << Std::endl;
//					pout << "hit item      (" << other[0] << ", " << other[1] << ", " << other[2] << ")" << Std::endl;
//					pout << "hit item time (" << u_0[0] << "-" << u_1[0] << ") (" << u_0[1] << "-" << u_1[1] << ") ("
//						 << u_0[2] << "-" << u_1[2] << ")" << Std::endl;
//					pout << "touch: " << touch << ", floor: " << touch_floor << ", block: " << blocking << Std::endl;
				}
			}
		}
	}

	return hit && hit->size();
}


const Item *CurrentMap::traceTopItem(int32 x, int32 y, int32 ztop, int32 zbot, ObjId ignore, uint32 shflags) {
	const Item *top = nullptr;

	if (ztop < zbot) {
		int32 temp = ztop;
		ztop = zbot;
		zbot = temp;
	}

	int minx = (x / _mapChunkSize);
	int maxx = (x / _mapChunkSize) + 1;
	int miny = (y / _mapChunkSize);
	int maxy = (y / _mapChunkSize) + 1;
	clipMapChunks(minx, maxx, miny, maxy);

	for (int cx = minx; cx <= maxx; cx++) {
		for (int cy = miny; cy <= maxy; cy++) {
			item_list::const_iterator iter;
			for (iter = _items[cx][cy].begin();
			        iter != _items[cx][cy].end(); ++iter) {
				const Item *item = *iter;
				if (item->getObjId() == ignore)
					continue;
				if (item->hasExtFlags(Item::EXT_SPRITE))
					continue;

				const ShapeInfo *si = item->getShapeInfo();
				if (!(si->_flags & shflags) || si->is_editor() || si->is_translucent()) continue;

				int32 ix, iy, iz, ixd, iyd, izd;
				item->getLocation(ix, iy, iz);
				item->getFootpadWorld(ixd, iyd, izd);

				if ((ix - ixd) >= x || ix <= x)
					continue;
				if ((iy - iyd) >= y || iy <= y)
					continue;
				if (iz >= ztop || (iz + izd) <= zbot)
					continue;

				if (top) {
					int32 tix, tiy, tiz, tixd, tiyd, tizd;
					top->getLocation(tix, tiy, tiz);
					top->getFootpadWorld(tixd, tiyd, tizd);

					if ((tiz + tizd) < (iz + izd)) top = nullptr;
				}

				if (!top) top = item;
			}
		}
	}
	return top;
}

void CurrentMap::setWholeMapFast() {
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; ++i) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS; ++j) {
			if (!isChunkFast(j, i))
				setChunkFast(j, i);
		}
	}
}

void CurrentMap::save(Common::WriteStream *ws) {
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; ++i) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS / 32; ++j) {
			ws->writeUint32LE(_fast[i][j]);
		}
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

	return true;
}

uint32 CurrentMap::I_canExistAt(const uint8 *args, unsigned int argsize) {
	ARG_UINT16(shape);
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT16(z);
	if (argsize > 8) {
		//!! TODO: figure these out
		ARG_UINT16(unk1); // is either 1 or 4
		ARG_UINT16(unk2); // looks like it could be an objid
		ARG_UINT16(unk3); // always zero
	}

	if (GAME_IS_CRUSADER) {
		x *= 2;
		y *= 2;
	}

	//
	// TODO: The crusader version of this function actually checks by
	// changing the shapeinfo of shape 0x31A to match the target
	// shape.  For a first level approximation, this is the same.
	//

	const CurrentMap *cm = World::get_instance()->getCurrentMap();
	bool valid = cm->isValidPosition(x, y, z, shape, 0, 0, 0);

	if (valid)
		return 1;
	else
		return 0;
}

uint32 CurrentMap::I_canExistAtPoint(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UINT16(unk1);
	ARG_UINT16(unk2);
	ARG_UINT16(shape);
	ARG_WORLDPOINT(pt);

	if (shape > 0x800)
		return 0;

	int32 x = pt.getX();
	int32 y = pt.getY();
	int32 z = pt.getZ();

	if (GAME_IS_CRUSADER) {
		x *= 2;
		y *= 2;
	}

	const CurrentMap *cm = World::get_instance()->getCurrentMap();
	bool valid = cm->isValidPosition(x, y, z, shape, 0, 0, 0);

	if (valid)
		return 1;
	else
		return 0;
}


} // End of namespace Ultima8
} // End of namespace Ultima
