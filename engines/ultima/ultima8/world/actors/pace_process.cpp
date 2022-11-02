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

#include "ultima/ultima8/world/actors/pace_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(PaceProcess)

PaceProcess::PaceProcess() : Process(), _counter(0) {
}

PaceProcess::PaceProcess(Actor *actor): _counter(0) {
	assert(actor);
	_itemNum = actor->getObjId();
	_type = 0x255;

	// Only pace with one process at a time.
	Process *previous = Kernel::get_instance()->findProcess(_itemNum, _type);
	if (previous)
		previous->terminate();
}


bool PaceProcess::maybeStartDefaultActivity1(Actor *actor) {
	uint16 activity = actor->getDefaultActivity(1);
	uint16 cur_activity = actor->getCurrentActivityNo();
	if (activity != 0 && cur_activity != activity && actor->canSeeControlledActor(false)) {
		actor->setActivity(activity);
		return true;
	}

	return false;
}

void PaceProcess::run() {
	Actor *a = getActor(_itemNum);
	Kernel *kernel = Kernel::get_instance();
	assert(kernel);

	if (!a || a->isDead()) {
		// dead?
		terminate();
		return;
	}

	if (!a->hasFlags(Item::FLG_FASTAREA))
		return;

	if (maybeStartDefaultActivity1(a))
		return;

	if (a->isBusy()) {
		return;
	}

	Animation::Result result = a->tryAnim(Animation::walk, a->getDir());
	if (result == Animation::SUCCESS) {
		_counter = 0;
		uint16 walkprocid = a->doAnim(Animation::walk, a->getDir());
		waitFor(walkprocid);
	} else {
		_counter++;
		if (_counter > 1) {
			uint32 shapeno = a->getShape();
			if (shapeno == 0x2f5 || shapeno == 0x2f7 || shapeno != 0x2f6 ||
				shapeno == 0x344 || shapeno == 0x597) {
				a->setActivity(7); // surrender
			} else {
				a->setActivity(5); // attack
			}
			return;
		}

		// Stand, turn around, and wait for 60.
		uint16 standprocid = a->doAnim(Animation::stand, a->getDir());
		//debug("PaceProcess: actor %d turning from %d to %d", a->getObjId(),
		//	  a->getDir(), Direction_Invert(a->getDir()));
		uint16 turnprocid = a->turnTowardDir(Direction_Invert(a->getDir()), standprocid);
		Process *waitproc = new DelayProcess(60);
		Kernel::get_instance()->addProcess(waitproc);
		waitproc->waitFor(turnprocid);
		waitFor(waitproc);
	}
}

void PaceProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);
	ws->writeByte(_counter);
}

bool PaceProcess::loadData(Common::ReadStream *rs, uint32 version) {
	bool result = Process::loadData(rs, version);
	if (result)
		_counter = rs->readByte();
	return result;
}

} // End of namespace Ultima8
} // End of namespace Ultima
