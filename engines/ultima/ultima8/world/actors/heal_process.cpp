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

#include "ultima/ultima8/world/actors/heal_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/get_object.h"

#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(HealProcess, Process)

HealProcess::HealProcess() : Process() {
	hungerCounter = 0;
	healCounter = 0;
	item_num = 0;
	type = 0x222; // CONSTANT!
}

void HealProcess::run() {
	MainActor *avatar = getMainActor();

	if (!avatar || avatar->isDead()) {
		// dead?
		terminate();
		return;
	}

	// heal one hitpoint and one manapoint every minute (1800 frames)

	healCounter++;

	if (healCounter == 900) {
		int16 mana = avatar->getMana();
		if (mana < avatar->getMaxMana()) {
			mana++;
			avatar->setMana(mana);
		}
	}

	if (healCounter == 1800) {
		uint16 hp = avatar->getHP();
		if (hp < avatar->getMaxHP()) {
			hp++;
			avatar->setHP(hp);
		}
		healCounter = 0;

		if (hungerCounter < 200)
			hungerCounter++;
	}
}

void HealProcess::feedAvatar(uint16 food) {
	MainActor *avatar = getMainActor();

	if (!avatar || avatar->isDead()) {
		// dead?
		terminate();
		return;
	}

	if (food > hungerCounter)
		food = hungerCounter;

	if (food == 0) return;

	uint16 oldCounter = hungerCounter;
	hungerCounter -= food;

	uint16 hp = avatar->getHP() - (hungerCounter / 4) + (oldCounter / 4);
	if (hp > avatar->getMaxHP()) hp = avatar->getMaxHP();

	avatar->setHP(hp);
}

uint32 HealProcess::I_feedAvatar(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UINT16(food);

	Process *p = Kernel::get_instance()->findProcess(0, 0x222); // CONSTANT!
	HealProcess *hp = p_dynamic_cast<HealProcess *>(p);
	if (!hp) {
		perr << "I_feedAvatar: unable to find HealProcess!" << std::endl;
		return 0;
	}

	hp->feedAvatar(food);

	return 0;
}


void HealProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write2(healCounter);
	ods->write2(hungerCounter);
}

bool HealProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	healCounter = ids->read2();
	hungerCounter = ids->read2();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
