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

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#include "timeline.h"
#include "dungeonman.h"


namespace DM {

Timeline::Timeline(DMEngine* vm) : _vm(vm) {
	_g369_eventMaxCount = 0;
	_g370_events = nullptr;
	_g372_eventCount = 0;
	_g371_timeline = nullptr;
	_g373_firstUnusedEventIndex = 0;
}

Timeline::~Timeline() {
	delete[] _g370_events;
	delete[] _g371_timeline;
}

void Timeline::f233_initTimeline() {
	_g370_events = new TimelineEvent[_g369_eventMaxCount];
	_g371_timeline = new uint16[_g369_eventMaxCount];
	if (_vm->_g298_newGame) {
		for (int16 i = 0; i < _g369_eventMaxCount; ++i)
			_g370_events->_type = k0_TMEventTypeNone;
		_g372_eventCount = 0;
		_g373_firstUnusedEventIndex = 0;
	}
}

void Timeline::f237_deleteEvent(uint16 eventIndex) {
	uint16 L0586_ui_TimelineIndex;
	uint16 L0587_ui_EventCount;


	_vm->_timeline->_g370_events[eventIndex]._type = k0_TMEventTypeNone;
	if (eventIndex < _vm->_timeline->_g373_firstUnusedEventIndex) {
		_vm->_timeline->_g373_firstUnusedEventIndex = eventIndex;
	}
	_vm->_timeline->_g372_eventCount--;
	if ((L0587_ui_EventCount = _vm->_timeline->_g372_eventCount) == 0) {
		return;
	}
	L0586_ui_TimelineIndex = f235_getIndex(eventIndex);
	if (L0586_ui_TimelineIndex == L0587_ui_EventCount) {
		return;
	}
	_vm->_timeline->_g371_timeline[L0586_ui_TimelineIndex] = _vm->_timeline->_g371_timeline[L0587_ui_EventCount];
	f236_fixChronology(L0586_ui_TimelineIndex);
}

void Timeline::f236_fixChronology(uint16 timelineIndex) {
	uint16 L0581_ui_TimelineIndex;
	uint16 L0582_ui_EventIndex;
	uint16 L0583_ui_EventCount;
	TimelineEvent* L0584_ps_Event;
	bool L0585_B_ChronologyFixed;


	if ((L0583_ui_EventCount = _vm->_timeline->_g372_eventCount) == 1) {
		return;
	}

	L0584_ps_Event = &_vm->_timeline->_g370_events[L0582_ui_EventIndex = _vm->_timeline->_g371_timeline[timelineIndex]];
	L0585_B_ChronologyFixed = false;
	while (timelineIndex > 0) { /* Check if the event should be moved earlier in the timeline */
		L0581_ui_TimelineIndex = (timelineIndex - 1) >> 1;
		if (f234_isEventABeforeB(L0584_ps_Event, &_vm->_timeline->_g370_events[_vm->_timeline->_g371_timeline[L0581_ui_TimelineIndex]])) {
			_vm->_timeline->_g371_timeline[timelineIndex] = _vm->_timeline->_g371_timeline[L0581_ui_TimelineIndex];
			timelineIndex = L0581_ui_TimelineIndex;
			L0585_B_ChronologyFixed = true;
		} else {
			break;
		}
	}
	if (L0585_B_ChronologyFixed)
		goto T0236011;
	L0583_ui_EventCount = ((L0583_ui_EventCount - 1) - 1) >> 1;
	while (timelineIndex <= L0583_ui_EventCount) { /* Check if the event should be moved later in the timeline */
		L0581_ui_TimelineIndex = (timelineIndex << 1) + 1;
		if (((L0581_ui_TimelineIndex + 1) < _vm->_timeline->_g372_eventCount) && (f234_isEventABeforeB(&_vm->_timeline->_g370_events[_vm->_timeline->_g371_timeline[L0581_ui_TimelineIndex + 1]], &_vm->_timeline->_g370_events[_vm->_timeline->_g371_timeline[L0581_ui_TimelineIndex]]))) {
			L0581_ui_TimelineIndex++;
		}
		if (f234_isEventABeforeB(&_vm->_timeline->_g370_events[_vm->_timeline->_g371_timeline[L0581_ui_TimelineIndex]], L0584_ps_Event)) {
			_vm->_timeline->_g371_timeline[timelineIndex] = _vm->_timeline->_g371_timeline[L0581_ui_TimelineIndex];
			timelineIndex = L0581_ui_TimelineIndex;
		} else {
			break;
		}
	}
T0236011:
	_vm->_timeline->_g371_timeline[timelineIndex] = L0582_ui_EventIndex;
}

bool Timeline::f234_isEventABeforeB(TimelineEvent* eventA, TimelineEvent* eventB) {
	bool L0578_B_Simultaneous;

	return (M30_time(eventA->_mapTime) < M30_time(eventB->_mapTime)) ||
		((L0578_B_Simultaneous = (M30_time(eventA->_mapTime) == M30_time(eventB->_mapTime))) && (eventA->getTypePriority() > eventB->getTypePriority())) ||
		(L0578_B_Simultaneous && (eventA->getTypePriority() == eventB->getTypePriority()) && (eventA <= eventB));
}

uint16 Timeline::f235_getIndex(uint16 eventIndex) {
	uint16 L0579_ui_TimelineIndex;
	uint16* L0580_pui_TimelineEntry;


	for (L0579_ui_TimelineIndex = 0, L0580_pui_TimelineEntry = _vm->_timeline->_g371_timeline; L0579_ui_TimelineIndex < _vm->_timeline->_g369_eventMaxCount; L0579_ui_TimelineIndex++) {
		if (*L0580_pui_TimelineEntry++ == eventIndex)
			break;
	}
	if (L0579_ui_TimelineIndex >= _vm->_timeline->_g369_eventMaxCount) { /* BUG0_00 Useless code. The function is always called with event indices that are in the timeline */
		L0579_ui_TimelineIndex = 0; /* BUG0_01 Coding error without consequence. Wrong return value. If the specified event index is not found in the timeline the function returns 0 which is the same value that is returned if the event index is found in the first timeline entry. No consequence because this code is never executed */
	}
	return L0579_ui_TimelineIndex;
}

uint16 Timeline::f238_addEventGetEventIndex(TimelineEvent* event) {
	uint16 L0588_ui_EventIndex;
	uint16 L0590_ui_NewEventIndex;
	TimelineEvent* L0591_ps_Event;


	if (_vm->_timeline->_g372_eventCount == _vm->_timeline->_g369_eventMaxCount) {
		_vm->f19_displayErrorAndStop(45);
	}
	if ((event->_type >= k5_TMEventTypeCorridor) && (event->_type <= k10_TMEventTypeDoor)) {
		for (L0588_ui_EventIndex = 0, L0591_ps_Event = _vm->_timeline->_g370_events; L0588_ui_EventIndex < _vm->_timeline->_g369_eventMaxCount; L0588_ui_EventIndex++, L0591_ps_Event++) {
			if ((L0591_ps_Event->_type >= k5_TMEventTypeCorridor) && (L0591_ps_Event->_type <= k10_TMEventTypeDoor)) {
				if ((event->_mapTime == L0591_ps_Event->_mapTime) && (event->getMapXY() == L0591_ps_Event->getMapXY()) && ((L0591_ps_Event->_type != k6_TMEventTypeWall) || (L0591_ps_Event->_C.A._cell == event->_C.A._cell))) {
					L0591_ps_Event->_C.A._effect = event->_C.A._effect;
					return L0588_ui_EventIndex;
				}
				continue;
			} else {
				if ((L0591_ps_Event->_type == k1_TMEventTypeDoorAnimation) && (event->_mapTime == L0591_ps_Event->_mapTime) && (event->getMapXY() == L0591_ps_Event->getMapXY())) {
					if (event->_C.A._effect == k2_SensorEffToggle) {
						event->_C.A._effect = 1 - L0591_ps_Event->_C.A._effect;
					}
					f237_deleteEvent(L0588_ui_EventIndex);
					break;
				}
			}
		}
	} else {
		if (event->_type == k1_TMEventTypeDoorAnimation) {
			for (L0588_ui_EventIndex = 0, L0591_ps_Event = _vm->_timeline->_g370_events; L0588_ui_EventIndex < _vm->_timeline->_g369_eventMaxCount; L0588_ui_EventIndex++, L0591_ps_Event++) {
				if ((event->_mapTime == L0591_ps_Event->_mapTime) && (event->getMapXY() == L0591_ps_Event->getMapXY())) {
					if (L0591_ps_Event->_type == k10_TMEventTypeDoor) {
						if (L0591_ps_Event->_C.A._effect == k2_SensorEffToggle) {
							L0591_ps_Event->_C.A._effect = 1 - event->_C.A._effect;
						}
						return L0588_ui_EventIndex;
					}
					if (L0591_ps_Event->_type == k1_TMEventTypeDoorAnimation) {
						L0591_ps_Event->_C.A._effect = event->_C.A._effect;
						return L0588_ui_EventIndex;
					}
				}
			}
		} else {
			if (event->_type == k2_TMEventTypeDoorDestruction) {
				for (L0588_ui_EventIndex = 0, L0591_ps_Event = _vm->_timeline->_g370_events; L0588_ui_EventIndex < _vm->_timeline->_g369_eventMaxCount; L0588_ui_EventIndex++, L0591_ps_Event++) {
					if ((event->getMapXY() == L0591_ps_Event->getMapXY()) && (M29_map(event->_mapTime) == M29_map(L0591_ps_Event->_mapTime))) {
						if ((L0591_ps_Event->_type == k1_TMEventTypeDoorAnimation) || (L0591_ps_Event->_type == k10_TMEventTypeDoor)) {
							f237_deleteEvent(L0588_ui_EventIndex);
						}
					}
				}
			}
		}
	}
	_vm->_timeline->_g370_events[L0590_ui_NewEventIndex = _vm->_timeline->_g373_firstUnusedEventIndex] = *event; /* Copy the event data (Megamax C can assign structures) */
	do {
		if (_vm->_timeline->_g373_firstUnusedEventIndex == _vm->_timeline->_g369_eventMaxCount)
			break;
		_vm->_timeline->_g373_firstUnusedEventIndex++;
	} while ((_vm->_timeline->_g370_events[_vm->_timeline->_g373_firstUnusedEventIndex])._type != k0_TMEventTypeNone);
	_vm->_timeline->_g371_timeline[_vm->_timeline->_g372_eventCount] = L0590_ui_NewEventIndex;
	f236_fixChronology(_vm->_timeline->_g372_eventCount++);
	return L0590_ui_NewEventIndex;
}

}
