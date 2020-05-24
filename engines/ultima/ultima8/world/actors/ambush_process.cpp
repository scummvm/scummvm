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

#include "ultima/ultima8/world/actors/ambush_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/actors/combat_process.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AmbushProcess)

AmbushProcess::AmbushProcess() : Process(), _delayCount(0) {

}

AmbushProcess::AmbushProcess(Actor *actor) : _delayCount(0) {
	assert(actor);
	_itemNum = actor->getObjId();
	_type = 0x21E; // CONSTANT !
}

void AmbushProcess::run() {
	if (_delayCount > 0) {
		_delayCount--;
		return;
	}
	_delayCount = 10;

	Actor *a = getActor(_itemNum);
	if (!a) {
	   // this shouldn't happen
	   terminate();
	   return;
	}

	CombatProcess *cp = a->getCombatProcess();
	if (!cp) {
		// this shouldn't have happened
		terminate();
		return;
	}

	ObjId targetid = cp->seekTarget();
	Item *target = getItem(targetid);

	// no target in range yet, continue waiting
	if (!target || a->getRange(*target) > 192)
		return;

	// target in range, so terminate and let parent take over
	terminate();
}

void AmbushProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint32LE(_delayCount);
}

bool AmbushProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_delayCount = rs->readUint32LE();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
