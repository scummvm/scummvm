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

#include "m4/wscript/ws_timer.h"
#include "m4/vars.h"

namespace M4 {

static void dispose_timeRequest(onTimeReq *timeReq);

bool ws_InitWSTimer(void) {
	_GWS(firstTimeReq) = nullptr;
	return true;
}

void ws_KillTime() {
	onTimeReq *tempTime;

	tempTime = _GWS(firstTimeReq);
	while (tempTime) {
		_GWS(firstTimeReq) = _GWS(firstTimeReq)->next;
		dispose_timeRequest(tempTime);
		tempTime = _GWS(firstTimeReq);
	}
}

static onTimeReq *new_timeRequest() {
	return (onTimeReq *)mem_alloc(sizeof(onTimeReq), "onTimeReq");
}

static void dispose_timeRequest(onTimeReq *timeReq) {
	if (timeReq) {
		mem_free(timeReq);
	}
}

void ws_MakeOnTimeReq(int32 wakeUpTime, machine *myXM, int32 pcOffset, int32 pcCount) {
	onTimeReq *newTimeReq, *tempTimeReq;

	// Create a new time request struct and set it's fields
	newTimeReq = new_timeRequest();
	newTimeReq->myTime = wakeUpTime;
	newTimeReq->myXM = myXM;
	newTimeReq->pcOffset = pcOffset;
	newTimeReq->pcCount = pcCount;

	// Insert the new time req into the list in ascending order
	// If the list is empty...
	if (!_GWS(firstTimeReq)) {
		newTimeReq->next = nullptr;
		_GWS(firstTimeReq) = newTimeReq;
	} else if (newTimeReq->myTime <= _GWS(firstTimeReq)->myTime) {
		// Else if it belongs at the front of a non-empty list..
		newTimeReq->next = _GWS(firstTimeReq);
		_GWS(firstTimeReq) = newTimeReq;
	} else {
		// Else it belongs in the middle/end of a non-empty list...
		tempTimeReq = _GWS(firstTimeReq);
		while (tempTimeReq->next && (tempTimeReq->next->myTime < newTimeReq->myTime)) {
			tempTimeReq = tempTimeReq->next;
		}

		newTimeReq->next = tempTimeReq->next;
		tempTimeReq->next = newTimeReq;
	}
}

void ws_CancelOnTimeReqs(machine *m) {
	onTimeReq *tempTimeReq, *prevTimeReq;

	if (!m)
		return;

	// Pass through the linked list, removing any requests for machine* m.
	prevTimeReq = _GWS(firstTimeReq);
	tempTimeReq = _GWS(firstTimeReq);

	while (tempTimeReq) {
		// If tempTimeReq is one to be cancelled...
		if (tempTimeReq->myXM == m) {
			// If req is first in the list
			if (tempTimeReq == _GWS(firstTimeReq)) {
				_GWS(firstTimeReq) = _GWS(firstTimeReq)->next;
				prevTimeReq = _GWS(firstTimeReq);
				dispose_timeRequest(tempTimeReq);
				tempTimeReq = _GWS(firstTimeReq);
			} else {
				// Else prevTimeReq is the parent of tempTimeReq
				prevTimeReq->next = tempTimeReq->next;
				dispose_timeRequest(tempTimeReq);
				tempTimeReq = prevTimeReq->next;
			}
		} else {
			// Else check next req in list, maintain prevTimeReq is parent of tempTimeReq
			if (prevTimeReq != tempTimeReq) {
				prevTimeReq = prevTimeReq->next;
			}
			tempTimeReq = tempTimeReq->next;
		}
	}
}

void ws_CheckTimeReqs(int32 curTime) {
	onTimeReq *tempTimeReq;
	machine *myXM;
	int32 pcOffset, pcCount;

	// Loop through the list, answering all expired time requests
	tempTimeReq = _GWS(firstTimeReq);
	while (tempTimeReq && (tempTimeReq->myTime <= curTime)) {
		// Extract all important information from request
		myXM = tempTimeReq->myXM;
		pcOffset = tempTimeReq->pcOffset;
		pcCount = tempTimeReq->pcCount;

		// Remove it from the list and dispose
		_GWS(firstTimeReq) = _GWS(firstTimeReq)->next;
		dispose_timeRequest(tempTimeReq);

		// Execute machine instructions for onTimeReq
		ws_StepWhile(myXM, pcOffset, pcCount);

		tempTimeReq = _GWS(firstTimeReq);
	}
}

} // End of namespace M4
