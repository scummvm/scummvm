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

#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/main_shape_archive.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/container.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/glob_egg.h"
#include "ultima/ultima8/world/egg.h"
#include "ultima/ultima8/world/monster_egg.h"
#include "ultima/ultima8/world/teleport_egg.h"
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/filesys/idata_source.h"

namespace Ultima {
namespace Ultima8 {

Item *ItemFactory::createItem(uint32 shape, uint32 frame, uint16 quality,
                              uint16 flags, uint16 npcnum, uint16 mapnum,
                              uint32 extendedflags, bool _objId) {
	// check what class to create
	ShapeInfo *info = GameData::get_instance()->getMainShapes()->
	                  getShapeInfo(shape);
	if (info == 0) return 0;

	// New item, no lerping
	extendedflags |= Item::EXT_LERP_NOPREV;

	uint32 family = info->_family;

	switch (family) {
	case ShapeInfo::SF_GENERIC:
	case ShapeInfo::SF_QUALITY:
	case ShapeInfo::SF_QUANTITY:
	case ShapeInfo::SF_BREAKABLE:
	case ShapeInfo::SF_REAGENT: // reagents need special handling too
	case ShapeInfo::SF_15: { // what's this?
		// 'simple' item

		Item *item = new Item();
		item->setShape(shape);
		item->_frame = frame;
		item->_quality = quality;
		item->_flags = flags;
		item->_npcNum = npcnum;
		item->_mapNum = mapnum;
		item->_extendedFlags = extendedflags;
		if (_objId) item->assignObjId();
		return item;
	}

	case ShapeInfo::SF_CONTAINER: {
		// container

		Container *container = new Container();
		container->setShape(shape);
		container->_frame = frame;
		container->_quality = quality;
		container->_flags = flags;
		container->_npcNum = npcnum;
		container->_mapNum = mapnum;
		container->_extendedFlags = extendedflags;
		if (_objId) container->assignObjId();
		return container;
	}

	case ShapeInfo::SF_GLOBEGG: {
		// glob egg

		GlobEgg *globegg = new GlobEgg();
		globegg->setShape(shape);
		globegg->_frame = frame;
		globegg->_quality = quality;
		globegg->_flags = flags;
		globegg->_npcNum = npcnum;
		globegg->_mapNum = mapnum;
		globegg->_extendedFlags = extendedflags;
		if (_objId) globegg->assignObjId();
		return globegg;
	}

	case ShapeInfo::SF_UNKEGG: {
		Egg *egg = new Egg();
		egg->setShape(shape);
		egg->_frame = frame;
		egg->_quality = quality;
		egg->_flags = flags;
		egg->_npcNum = npcnum;
		egg->_mapNum = mapnum;
		egg->_extendedFlags = extendedflags;
		if (_objId) egg->assignObjId();
		return egg;
	}

	case ShapeInfo::SF_MONSTEREGG: {
		MonsterEgg *egg = new MonsterEgg();
		egg->setShape(shape);
		egg->_frame = frame;
		egg->_quality = quality;
		egg->_flags = flags;
		egg->_npcNum = npcnum;
		egg->_mapNum = mapnum;
		egg->_extendedFlags = extendedflags;
		if (_objId) egg->assignObjId();
		return egg;
	}

	case ShapeInfo::SF_TELEPORTEGG: {
		TeleportEgg *egg = new TeleportEgg();
		egg->setShape(shape);
		egg->_frame = frame;
		egg->_quality = quality;
		egg->_flags = flags;
		egg->_npcNum = npcnum;
		egg->_mapNum = mapnum;
		egg->_extendedFlags = extendedflags;
		if (_objId) egg->assignObjId();
		return egg;
	}

	default:
		return 0;
	}
}



Actor *ItemFactory::createActor(uint32 shape, uint32 frame, uint16 quality,
                                uint16 flags, uint16 npcnum, uint16 mapnum,
                                uint32 extendedflags, bool _objId) {
	// this function should probably differentiate between the Avatar,
	// NPCs, monsters?

	/*
	    // This makes it rather hard to create new NPCs...
	    if (npcnum == 0) // or do monsters have npcnum 0? we'll see...
	        return 0;
	*/
	// New actor, no lerping
	extendedflags |= Item::EXT_LERP_NOPREV;

	if (npcnum == 1) {
		// Main Actor

		MainActor *actor = new MainActor();
		actor->setShape(shape);
		actor->_frame = frame;
		actor->_quality = quality;
		actor->_flags = flags;
		actor->_npcNum = npcnum;
		actor->_mapNum = mapnum;
		actor->_objId = 1;
		actor->_extendedFlags = extendedflags;
		return actor;
	}

	// 'normal' Actor/NPC
	Actor *actor = new Actor();
	actor->setShape(shape);
	actor->_frame = frame;
	actor->_quality = quality;
	actor->_flags = flags;
	actor->_npcNum = npcnum;
	actor->_mapNum = mapnum;
	if (npcnum != 0) {
		actor->_objId = static_cast<uint16>(npcnum);
	} else if (_objId) {
		actor->assignObjId();
	}
	actor->_extendedFlags = extendedflags;

	return actor;
}

} // End of namespace Ultima8
} // End of namespace Ultima
