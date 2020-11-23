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

static Item *getItemForFamily(uint32 family) {
	switch (family) {
	case ShapeInfo::SF_GENERIC:
	case ShapeInfo::SF_QUALITY:
	case ShapeInfo::SF_QUANTITY:
	case ShapeInfo::SF_BREAKABLE:
	case ShapeInfo::SF_REAGENT: // reagents need special handling too
	case ShapeInfo::SF_CRUWEAPON:
	case ShapeInfo::SF_CRUAMMO:
	case ShapeInfo::SF_CRUBOMB:
	case ShapeInfo::SF_CRUINVITEM:
	case ShapeInfo::SF_15: // what's this?
		// 'simple' item
		return new Item();

	case ShapeInfo::SF_CONTAINER:
		return new Container();

	case ShapeInfo::SF_GLOBEGG:
		return new GlobEgg();

	case ShapeInfo::SF_UNKEGG:
		return new Egg();

	case ShapeInfo::SF_MONSTEREGG:
		// FIXME: For crusader, item family 7 is also treated like a container
		// (it can have contents, and the contents can be destroyed.
		// Is it not the same as monster egg?
		return new MonsterEgg();

	case ShapeInfo::SF_TELEPORTEGG:
		return new TeleportEgg();
	}

	return nullptr;
}

Item *ItemFactory::createItem(uint32 shape, uint32 frame, uint16 quality,
                              uint16 flags, uint16 npcnum, uint16 mapnum,
                              uint32 extendedflags, bool objId) {
	// check what class to create
	const ShapeInfo *info = GameData::get_instance()->getMainShapes()->
	                  getShapeInfo(shape);
	if (info == nullptr)
		return nullptr;

	// New item, no lerping
	extendedflags |= Item::EXT_LERP_NOPREV;

	uint32 family = info->_family;

	Item *item = getItemForFamily(family);

	if (item) {
		item->setShape(shape);
		item->_frame = frame;
		item->_quality = quality;
		item->_flags = flags;
		item->_npcNum = npcnum;
		item->_mapNum = mapnum;
		item->_extendedFlags = extendedflags;
		if (objId)
			item->assignObjId();
		if (GAME_IS_CRUSADER) {
			if (info->_damageInfo && info->_damageInfo->takesDamage()) {
				item->setDamagePoints(info->_damageInfo->damagePoints());
			}
			if (info->_family == ShapeInfo::SF_CRUWEAPON && info->_weaponInfo &&
				info->_weaponInfo->_defaultAmmo) {
				item->setQuality(info->_weaponInfo->_defaultAmmo);
			}
			if (info->_family == ShapeInfo::SF_CRUAMMO ||
					 info->_family == ShapeInfo::SF_CRUBOMB) {
				item->setQuality(1);
			}
		}
	}

	return item;
}

static Actor *getActorForNpcNum(uint32 npcnum) {
	if (npcnum == 1)
		return new MainActor();

	// 'normal' Actor/NPC
	return new Actor();
}

Actor *ItemFactory::createActor(uint32 shape, uint32 frame, uint16 quality,
                                uint16 flags, uint16 npcnum, uint16 mapnum,
                                uint32 extendedflags, bool objId) {
	/*
	    // This makes it rather hard to create new NPCs...
	    if (npcnum == 0) // or do monsters have npcnum 0? we'll see...
	        return nullptr;
	*/
	// New actor, no lerping
	extendedflags |= Item::EXT_LERP_NOPREV;

	Actor *actor = getActorForNpcNum(npcnum);

	actor->setShape(shape);
	actor->_frame = frame;
	actor->_quality = quality;
	actor->_flags = flags;
	actor->_npcNum = npcnum;
	actor->_mapNum = mapnum;
	if (npcnum != 0) {
		actor->_objId = static_cast<uint16>(npcnum);
	} else if (objId) {
		actor->assignObjId();
	}
	actor->_extendedFlags = extendedflags;

	return actor;
}

} // End of namespace Ultima8
} // End of namespace Ultima
