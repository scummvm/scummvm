/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_EVENTMANAGER_H_INCLUDED
#define ICB_EVENTMANAGER_H_INCLUDED

// Include files with relative paths
#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_string.h"
#include "engines/icb/common/px_assert.h"
#include "engines/icb/p4.h"
#include "engines/icb/event_timer.h"
#include "engines/icb/event_list.h"
#include "engines/icb/session.h"

namespace ICB {

#define EVENT_MANAGER_LOG "event_manager_log.txt"

// Maximum number of event timers active at one time.
#define EVENT_MANAGER_MAX_TIMERS 4

// Keeps track of which object is interested in hearing about events from which other objects and stores
// events which have occured until they can be handled.
class _event_manager {
public:
	// Default constructor and destructor.
	_event_manager() {}
	~_event_manager() {}

	// This initializes the whole object and must be called at the start of each session.
	void Initialise();

	// This says whether a particular object has events pending.
	bool8 HasEventPending(int32 nObjectID);

	// This checks if an object has the named event waiting for it.
	bool8 CheckEventWaitingForObject(int32 nObjectID, const char *pcEventName);

	// This shuts down an object in the event manager.
	void ShutDownEventProcessingForObject(int32 nObjectID);

	// This processes the event timers if there are any, posting events as required.
	void CycleEventManager();

	// Determines whether or not the named object is registered for the named event.
	bool8 IsObjectRegisteredForEvent(int32 nCallerID, const char *pcEventName);

	// These functions save and restore necessary parts of the event manager.
	void Save(Common::WriteStream *stream) const;
	void Restore(Common::SeekableReadStream *stream);

	// These functions will probably have direct script counterparts.
	void RegisterForEvent(int32 nObjectID, const char *pcEventName);
	void UnregisterForEvent(int32 nObjectID, const char *pcEventName);
	void PostNamedEvent(const char *pcEventName, int32 nSenderID);
	void PostNamedEventToObject(const char *pcEventName, int32 nTargetID, int32 nSenderID);
	void PostRepeatingEvent(const char *pcEventName, uint32 nStart, uint32 nInterval, uint32 nEnd);
	void ClearAllEventsForObject(int32 nObjectID);
	bool8 DidObjectSendLastNamedEvent(int32 nCallerID, int32 nObjectID, const char *pcEventName) const;
	int32 GetIDOfLastObjectToPostEvent(int32 nCallerID, const char *pcEventName) const;
	void SetSuspendFlagForObject(int32 nObjectID, bool8 bState);

private:
	_event_list m_pEventLists[MAX_session_objects];        // List of objects and the events they are currently interested in.
	_event_timer m_pEventTimers[EVENT_MANAGER_MAX_TIMERS]; // Housekeeping for future and repeating events.
	uint8 m_nNumObjects;                                   // Number of objects in the event manager.
	bool8 m_pbActiveTimers[EVENT_MANAGER_MAX_TIMERS];      // Housekeeping for future and repeating events.
	bool8 m_pbRunning[MAX_session_objects];                // Set true for currently-running objects, false if they have been shut down.
	bool8 m_pbSuspended[MAX_session_objects];              // Allows objects to be suspended then reinstated.

	// Here I block the use of the default '='.
	_event_manager(const _event_manager &) {}
	void operator=(const _event_manager &) {}
};

extern _event_manager *g_oEventManager; // Instantiated in global_objects.cpp.

} // End of namespace ICB

#endif // #if !defined( EVENTMANAGER_H_INCLUDED )
