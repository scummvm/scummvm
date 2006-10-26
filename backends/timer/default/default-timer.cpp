/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "backends/timer/default/default-timer.h"
#include "common/util.h"
#include "common/system.h"


struct TimerSlot {
	Common::TimerManager::TimerProc callback;
	void *refCon;
	int32 interval;
	int32 nextFireTime;
	
	TimerSlot *next;
};

void insertPrioQueue(TimerSlot *head, TimerSlot *newSlot) {
	// The head points to a fake anchor TimerSlot; this common
	// trick allows us to get rid of many special cases.
	
	const int32 nextFireTime = newSlot->nextFireTime;
	TimerSlot *slot = head;
	newSlot->next = 0;

	// Insert the new slot into the sorted list of already scheduled
	// timers in such a way that the list stays sorted...
	while (true) {
		assert(slot);
		if (slot->next == 0 || slot->next->nextFireTime > nextFireTime) {
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

	const int32 curTime = g_system->getMillis() * 1000;
	
	// Repeat as long as there is a TimerSlot that is scheduled to fire.
	TimerSlot *slot = _head->next;
	while (slot && slot->nextFireTime < curTime) {
		// Remove the slot from the priority queue
		_head->next = slot->next;
		
		// Update the fire time and reinsert the TimerSlot into the priority
		// queue. Has to be done before the timer callback is invoked, in case
		// the callback wants to remove itself.
		assert(slot->interval > 0);
		slot->nextFireTime += slot->interval;
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
	slot->nextFireTime = g_system->getMillis() * 1000 + interval;
	slot->next = 0;
	
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
