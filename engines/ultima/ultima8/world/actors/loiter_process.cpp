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
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/world/actors/loiter_process.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/actors/pathfinder_process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
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

	int32 x, y, z;
	a->getLocation(x, y, z);

	x += 32 * ((getRandom() % 20) - 10);
	y += 32 * ((getRandom() % 20) - 10);

	PathfinderProcess *pfp = new PathfinderProcess(a, x, y, z);
	Kernel::get_instance()->addProcess(pfp);

	bool hasidle1 = a->hasAnim(Animation::idle1);
	bool hasidle2 = a->hasAnim(Animation::idle2);

	if ((hasidle1 || hasidle2) && ((getRandom() % 3) == 0)) {
		Animation::Sequence idleanim;

		if (!hasidle1) {
			idleanim = Animation::idle2;
		} else if (!hasidle2) {
			idleanim = Animation::idle1;
		} else {
			if (getRandom() % 2)
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
		DelayProcess *dp = new DelayProcess(30 * (4 + (getRandom() % 3)));
		Kernel::get_instance()->addProcess(dp);
		dp->waitFor(pfp);

		waitFor(dp);
	}
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
