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

#include "ultima/ultima8/world/actors/guard_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/actors/actor_anim_process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(GuardProcess)

GuardProcess::GuardProcess() : Process() {
}

GuardProcess::GuardProcess(Actor *actor) {
	assert(actor);
	_itemNum = actor->getObjId();
	_type = 0x25e;
}

void GuardProcess::run() {
	Actor *a = getActor(_itemNum);
	if (!a || a->isDead()) {
		// dead?
		terminate();
		return;
	}

	// Do nothing if busy
	if (a->isBusy())
		return;

	Actor *mainactor = getMainActor();
	if (!mainactor)
		return;

	if (!a->canSeeControlledActor(false)) {
		Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();
		if (rs.getRandomBit()) {
			DelayProcess *dp = new DelayProcess(30 * rs.getRandomNumberRng(1, 3));
			Kernel::get_instance()->addProcess(dp);
			waitFor(dp);
		} else {
			Animation::Sequence anim = Animation::absAnim(rs.getRandomBit() ? Animation::lookLeftCru : Animation::lookRightCru);
			uint16 animproc = a->doAnim(anim, dir_current);
			a->doAnimAfter(Animation::stand, dir_current, animproc);
		}
		return;
	}

	// Saw the silencer, go to combat.
	a->setActivity(5);
}

void GuardProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);
}

bool GuardProcess::loadData(Common::ReadStream *rs, uint32 version) {
	return Process::loadData(rs, version);
}

} // End of namespace Ultima8
} // End of namespace Ultima
