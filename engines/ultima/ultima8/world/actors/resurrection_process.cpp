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
#include "ultima/ultima8/world/actors/resurrection_process.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(ResurrectionProcess)

ResurrectionProcess::ResurrectionProcess() : Process() {

}

ResurrectionProcess::ResurrectionProcess(Actor *actor) {
	assert(actor);
	_itemNum = actor->getObjId();

	_type = 0x229; // CONSTANT !
}

void ResurrectionProcess::run() {
	Actor *a = getActor(_itemNum);

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

	if (a->hasFlags(Item::FLG_GUMP_OPEN)) {
		// first close gump in case player is still rummaging through us
		a->closeGump();
	}

	a->clearActorFlag(Actor::ACT_WITHSTANDDEATH);
	a->clearActorFlag(Actor::ACT_DEAD);

	// reload stats
	if (!a->loadMonsterStats()) {
		perr << "ResurrectionProcess::run failed to reset stats for actor ("
		     << a->getShape() << ")." << Std::endl;
	}

	// go into combat mode
	// Note: only happens in U8, so activity num is not important.
	a->setInCombat(0);

	// we should already be killed by going into combat mode.
	if (!(_flags & PROC_TERMINATED))
		terminate();
}

void ResurrectionProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);
}

bool ResurrectionProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
