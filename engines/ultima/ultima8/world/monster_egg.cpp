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

#include "ultima/ultima8/world/monster_egg.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/world/actors/monster_info.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(MonsterEgg)

MonsterEgg::MonsterEgg() {
}


MonsterEgg::~MonsterEgg() {
}

uint16 MonsterEgg::hatch() {
	//!! do we need to check probability here?
	//!! monster activity? combat? anything?

	int shapeNum = getMonsterShape();

	// CHECKME: why does this happen? (in the plane of Earth near the end)
	if (shapeNum == 0)
		return 0;

	Actor *newactor = ItemFactory::createActor(shapeNum, 0, 0,
	                  FLG_FAST_ONLY | FLG_DISPOSABLE | FLG_IN_NPC_LIST,
	                  0, 0, 0, true);
	if (!newactor) {
		perr << "MonsterEgg::hatch failed to create actor (" << shapeNum
		     << ")." << Std::endl;
		return 0;
	}
	uint16 objID = newactor->getObjId();

	// set stats
	if (!newactor->loadMonsterStats()) {
		perr << "MonsterEgg::hatch failed to set stats for actor (" << shapeNum
		     << ")." << Std::endl;
	}

	if (!newactor->canExistAt(_x, _y, _z)) {
		newactor->destroy();
		return 0;
	}

	// mapnum has to be set to the current map. Reason: Beren teleports to
	// newactor->getMapNum() when newactor is assaulted.
	newactor->setMapNum(World::get_instance()->getCurrentMap()->getNum());
	newactor->setNpcNum(objID);
	newactor->move(_x, _y, _z);
	newactor->setActivity(getActivity());

	return objID;
}

void MonsterEgg::saveData(Common::WriteStream *ws) {
	Item::saveData(ws);
}

bool MonsterEgg::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Item::loadData(rs, version)) return false;

	return true;
}

uint32 MonsterEgg::I_monsterEggHatch(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	MonsterEgg *megg = dynamic_cast<MonsterEgg *>(item);
	if (!megg) return 0;

	return megg->hatch();
}

uint32 MonsterEgg::I_getMonId(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	MonsterEgg *megg = dynamic_cast<MonsterEgg *>(item);
	if (!megg) return 0;

	return megg->getMapNum() >> 3;
}

} // End of namespace Ultima8
} // End of namespace Ultima
