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

#include "ultima/ultima8/world/actors/rolling_thunder_process.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(RollingThunderProcess)

RollingThunderProcess::RollingThunderProcess() : Process() {
}

RollingThunderProcess::RollingThunderProcess(Actor *actor) {
	assert(actor);
	_itemNum = actor->getObjId();

	_type = 0x263; // CONSTANT!
}

void RollingThunderProcess::run() {
	Actor *actor = getActor(_itemNum);

	if (!actor) {
		// gone! maybe dead..
		terminate();
		return;
	}

	if (actor->isBusy()) {
		Process *wait = new DelayProcess(60);
		Kernel::get_instance()->addProcess(wait);
		waitFor(wait);
		return;
	}

	warning("TODO: Implement rolling thunder");
	terminate();
}

void RollingThunderProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);
}

bool RollingThunderProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
