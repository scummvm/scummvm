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


#include "ultima/ultima8/world/actors/scheduler_process.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(SchedulerProcess)

SchedulerProcess::SchedulerProcess() : Process() {
	_lastRun = 0;
	_nextActor = 0;
	_type = 0x245; // CONSTANT!
}

void SchedulerProcess::run() {
	if (_nextActor != 0) {
		// doing a scheduling run at the moment

		Actor *a = getActor(_nextActor);
		if (a) {
			// CHECKME: is this the right time to pass? CONSTANT
			uint32 stime = Ultima8Engine::get_instance()->getGameTimeInSeconds() / 60;
			ProcId schedpid = a->callUsecodeEvent_schedule(stime);
			if (schedpid) waitFor(schedpid);
		}

		_nextActor++;
		if (_nextActor == 256) { // CONSTANT
			_nextActor = 0; // done
#if 0
			debugC(kDebugActor, "Scheduler: finished run at %u",
				Kernel::get_instance()->getFrameNum());
#endif
		}

		return;
	}

	// CONSTANT!
	uint32 currenthour = Ultima8Engine::get_instance()->getGameTimeInSeconds() / 900;

	if (currenthour > _lastRun) {
		// schedule a new scheduling run
		_lastRun = currenthour;
		_nextActor = 1;
#if 0
		debugC(kDebugActor, "Scheduler: %u" , Kernel::get_instance()->getFrameNum());
#endif
	}
}

void SchedulerProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint32LE(_lastRun);
	ws->writeUint16LE(_nextActor);
}

bool SchedulerProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_lastRun = rs->readUint32LE();
	_nextActor = rs->readUint16LE();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
