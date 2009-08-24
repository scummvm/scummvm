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
 * $URL$
 * $Id$
 */

#include "common/scummsys.h"
#include "backends/timer/default/default-timer.h"
#include "common/util.h"
#include "common/system.h"


struct TimerSlot {
	Common::TimerManager::TimerProc callback;
	void *refCon;
	uint32 interval;	// in microseconds

	uint32 nextFireTime;	// in milliseconds
	uint32 nextFireTimeMicro;	// mircoseconds part of nextFire

	TimerSlot *next;
};

void insertPrioQueue(TimerSlot *head, TimerSlot *newSlot) {
	// The head points to a fake anchor TimerSlot; this common
	// trick allows us to get rid of many special cases.

	const uint32 nextFireTime = newSlot->nextFireTime;
	TimerSlot *slot = head;
	newSlot->next = 0;

	// Insert the new slot into the sorted list of already scheduled
	// timers in such a way that the list stays sorted...
	while (true) {
		assert(slot);
		if (slot->next == 0 || nextFireTime < slot->next->nextFireTime) {
			newSlot->next = slot->next;
			slot->next = newSlot;
			return;
		}
		slot = slot->next;
	}
}


DefaultTimerManager::DefaultTimerManager() :
	_timerHandler(0),
	_head(0) {

	_head = new TimerSlot();
	memset(_head, 0, sizeof(TimerSlot));
}

DefaultTimerManager::~DefaultTimerManager() {
	Common::StackLock lock(_mutex);

	TimerSlot *slot = _head;
	while (slot) {
		TimerSlot *next = slot->next;
		delete slot;
		slot = next;
	}
	_head = 0;
}

void DefaultTimerManager::handler() {
	Common::StackLock lock(_mutex);

	const uint32 curTime = g_system->getMillis();

	// Repeat as long as there is a TimerSlot that is scheduled to fire.
	TimerSlot *slot = _head->next;
	while (slot && slot->nextFireTime < curTime) {
		// Remove the slot from the priority queue
		_head->next = slot->next;

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
		slot->callback(slot->refCon);

		// Look at the next scheduled timer
		slot = _head->next;
	}
}

bool DefaultTimerManager::installTimerProc(TimerProc callback, int32 interval, void *refCon) {
	assert(interval > 0);
	Common::StackLock lock(_mutex);

	TimerSlot *slot = new TimerSlot;
	slot->callback = callback;
	slot->refCon = refCon;
	slot->interval = interval;
	slot->nextFireTime = g_system->getMillis() + interval / 1000;
	slot->nextFireTimeMicro = interval % 1000;
	slot->next = 0;

	// FIXME: It seems we do allow the client to add one callback multiple times over here,
	// but "removeTimerProc" will remove *all* added instances. We should either prevent
	// multiple additions of a timer proc OR we should change removeTimerProc to only remove
	// a specific timer proc entry.
	// Probably we can safely just allow a single addition of a specific function once
	// and just update our Timer documentation accordingly.
	insertPrioQueue(_head, slot);

	return true;
}

void DefaultTimerManager::removeTimerProc(TimerProc callback) {
	Common::StackLock lock(_mutex);

	TimerSlot *slot = _head;

	while (slot->next) {
		if (slot->next->callback == callback) {
			TimerSlot *next = slot->next->next;
			delete slot->next;
			slot->next = next;
		} else {
			slot = slot->next;
		}
	}
}
