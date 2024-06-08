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

#include "ultima/ultima8/world/actors/loiter_process.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/actors/pathfinder_process.h"
#include "ultima/ultima8/world/actors/cru_pathfinder_process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(LoiterProcess)

LoiterProcess::LoiterProcess() : Process(), _count(0) {
}

LoiterProcess::LoiterProcess(Actor *actor, int32 c) : _count(c) {
	assert(actor);
	_itemNum = actor->getObjId();

	if (GAME_IS_U8)
		_type = 0x205; // CONSTANT!
	else
		_type = 599;

	// Only loiter with one process at a time.
	Process *previous = Kernel::get_instance()->findProcess(_itemNum, _type);
	if (previous)
		previous->terminate();
	Process *prevpf = Kernel::get_instance()->findProcess(_itemNum, PathfinderProcess::PATHFINDER_PROC_TYPE);
	if (prevpf)
		prevpf->terminate();
}

void LoiterProcess::run() {
	if (!_count) {
		terminate();
		return;
	}
	if (_count > 0)
		_count--;

	Actor *a = getActor(_itemNum);

	if (!a || a->isDead()) {
		// dead?
		terminate();
		return;
	}

	Point3 pt = a->getLocation();

	Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();

	pt.x += 32 * rs.getRandomNumberRngSigned(-10, 10);
	pt.y += 32 * rs.getRandomNumberRngSigned(-10, 10);

	Process *pfp;
	if (GAME_IS_U8)
		pfp = new PathfinderProcess(a, pt);
	else
		pfp = new CruPathfinderProcess(a, pt, 0xc, 0x80, false);

	Kernel::get_instance()->addProcess(pfp);

	bool hasidle1 = a->hasAnim(Animation::idle1);
	bool hasidle2 = a->hasAnim(Animation::idle2);

	if ((hasidle1 || hasidle2) && (rs.getRandomNumber(2) == 0)) {
		Animation::Sequence idleanim;

		if (!hasidle1) {
			idleanim = Animation::idle2;
		} else if (!hasidle2) {
			idleanim = Animation::idle1;
		} else {
			if (rs.getRandomBit())
				idleanim = Animation::idle1;
			else
				idleanim = Animation::idle2;
		}
		uint16 idlepid = a->doAnim(idleanim, dir_current);
		Process *idlep = Kernel::get_instance()->getProcess(idlepid);
		idlep->waitFor(pfp);

		waitFor(idlep);

	} else {
		// wait 4-7 sec
		DelayProcess *dp = new DelayProcess(30 * rs.getRandomNumberRng(4, 7));
		Kernel::get_instance()->addProcess(dp);
		dp->waitFor(pfp);

		waitFor(dp);
	}
}

Common::String LoiterProcess::dumpInfo() const {
	return Process::dumpInfo() +
		Common::String::format(", frames left: %d", _count);
}

void LoiterProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint32LE(_count);
}

bool LoiterProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	if (version >= 3)
		_count = rs->readUint32LE();
	else
		_count = 0; // default to loitering indefinitely

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
