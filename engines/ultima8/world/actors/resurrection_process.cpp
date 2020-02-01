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

#include "ultima8/misc/pent_include.h"

#include "resurrection_process.h"
#include "ultima8/world/actors/actor.h"
#include "ultima8/world/get_object.h"

#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(ResurrectionProcess, Process);

ResurrectionProcess::ResurrectionProcess() : Process() {

}

ResurrectionProcess::ResurrectionProcess(Actor *actor_) {
	assert(actor_);
	item_num = actor_->getObjId();

	type = 0x229; // CONSTANT !
}

void ResurrectionProcess::run() {
	Actor *a = getActor(item_num);

	if (!a) {
		// actor gone... too late for resurrection now :-)
		terminate();
		return;
	}

	if (!a->isDead()) {
		// not dead?
		terminate();
		return;
	}

	if (a->getFlags() & Item::FLG_GUMP_OPEN) {
		// first close gump in case player is still rummaging through us
		a->closeGump();
	}

	a->clearActorFlag(Actor::ACT_WITHSTANDDEATH);
	a->clearActorFlag(Actor::ACT_DEAD);

	// reload stats
	if (!a->loadMonsterStats()) {
		perr << "ResurrectionProcess::run failed to reset stats for actor ("
		     << a->getShape() << ")." << std::endl;
	}

	// go into combat mode
	a->setInCombat();

	// we should already be killed by going into combat mode.
	if (!(flags & PROC_TERMINATED))
		terminate();
}

void ResurrectionProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);
}

bool ResurrectionProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	return true;
}

} // End of namespace Ultima8
