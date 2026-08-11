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

#include "backends/timer/emscripten/emscripten-timer.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "common/util.h"

EmscriptenTimerManager::EmscriptenTimerManager() : DefaultTimerManager() {
}

EmscriptenTimerManager::~EmscriptenTimerManager() {
}

void EmscriptenTimerManager::handler() {
	Common::StackLock lock(_mutex);

	uint32 curTime = g_system->getMillis(true);
	TimerSlot *slot = _head->next;
	TimerSlot *prev = _head;

	// Repeat as long as there is a TimerSlot that is scheduled to fire.
	while (slot && slot->nextFireTime < curTime) {
		// avoid a recursion loop if a timer callback decides to call OSystem::delayMillis()
		// timers should not be triggering timers, but many timers trigger I/O - which in case
		// of virtual FS will trigger delayMillis for the network timer to keep running.
		// We know the network timer itself does not have this recursion issue, so we let it pass.
		static bool inTimer = false;
		bool isConnManTimer = (slot->id == "Networking::ConnectionManager's Timer");
		if (!inTimer || isConnManTimer) {
			// Remove the slot from the priority queue
			prev->next = slot->next;
			// Update the fire time and reinsert the TimerSlot into the priority
			// queue.
			assert(slot->interval > 0);
			slot->nextFireTime += (slot->interval / 1000);
			slot->nextFireTimeMicro += (slot->interval % 1000);
			if (slot->nextFireTimeMicro > 1000) {
				slot->nextFireTime += slot->nextFireTimeMicro / 1000;
				slot->nextFireTimeMicro %= 1000;
			}
			insertPrioQueue(_head, slot);

			// Invoke the timer callback
			assert(slot->callback);
			if (!isConnManTimer)
				inTimer = true;
			slot->callback(slot->refCon);
			if (!isConnManTimer)
				inTimer = false;
		} else {
			// Skip to the next timer, leaving this one in place to be checked again later
			prev = slot;
		}
		slot = prev->next;
	}
}
