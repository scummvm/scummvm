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

#ifndef ICB_EVENTLIST_H_INCLUDED
#define ICB_EVENTLIST_H_INCLUDED

#include "engines/icb/string_vest.h"
#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_string.h"
#include "engines/icb/common/px_assert.h"
#include "engines/icb/string_vest.h"

namespace ICB {

// Use globals to save on rdata storage and having repeated strings therein
extern const char *global_event_deleted_placeholder;
extern const char *global_event_generic;
extern const char *global_event_line_of_sight;
extern const char *global_event_sound;
extern const char *global_event_on_camera;
extern const char *global_event_off_camera;
extern const char *global_event_logic_rerun;
extern const char *global_event_out_of_sight;
extern const char *global_event_lift_ascend;
extern const char *global_event_lift_descend;

// This is used to mark deleted objects to save reallocating all the time.
#define EVENT_DELETED_PLACEHOLDER global_event_deleted_placeholder

// This string is used to identify a generic, anonymous event, which all objects respond to all the time.
#define EVENT_LINE_OF_SIGHT global_event_line_of_sight
#define EVENT_OUT_OF_SIGHT global_event_out_of_sight

#define EVENT_ON_CAMERA global_event_on_camera
#define EVENT_OFF_CAMERA global_event_off_camera

#define EVENT_LIFT_ASCEND global_event_lift_ascend
#define EVENT_LIFT_DESCEND global_event_lift_descend

#define EVENT_LOGIC_RERUN global_event_logic_rerun

// Primarily for PSX speed variable data structures are history.
#define EVENT_MAX_EVENTS_REGISTERED 20

// This marks an invalid object ID as the sender of an event.
#define EVENT_INVALID_SENDER_ID (-1)

struct _event {
	const char *s_pcEventName;
	int32 s_nLastSenderID;
	bool8 s_bPending;
	uint8 s_nPadding[3];

	// Initialisation.
	_event() {
		s_pcEventName = EVENT_DELETED_PLACEHOLDER;
		s_nLastSenderID = EVENT_INVALID_SENDER_ID;
		s_bPending = FALSE8;
	}

	// Copy constructor and assignment.
	_event(const _event &oX) {
		s_pcEventName = oX.s_pcEventName;
		s_nLastSenderID = oX.s_nLastSenderID;
		s_bPending = oX.s_bPending;
	}

	const _event &operator=(const _event &oOpB) {
		s_pcEventName = oOpB.s_pcEventName;
		s_nLastSenderID = oOpB.s_nLastSenderID;
		s_bPending = oOpB.s_bPending;
		return (*this);
	}
};

// class _event_list
// Holds the list of events an object is currently interested in and counts of which ones are pending.  Events
// with handlers are held in a separate list and take precedence over generic events, which simply cause the
// logic context for the object to be rerun.
class _event_list {
public:
	// Default constructor and destructor.
	inline _event_list();
	~_event_list() {}

	// Copy constructor.
	_event_list(const _event_list &oX);

	// Operator '='.
	const _event_list &operator=(const _event_list &oOpB);

	// This sets a new object name for the event list, and resets everything else.
	void SetNewObjectName(const char *pcObjectName);

	// This gets the name of the object that the event list is for.
	const char *GetObjectName() const { return (m_pcObjectName); }

	// This determines whether or not an object requires any event processing.
	inline bool8 HasEventPending();

	// Checks if the named event is waiting for the object.
	bool8 CheckEventWaiting(const char *pcEventName);

	// This determines whether or not the named event is in the list of registered events.
	bool8 IsEventInList(const char *pcEventName) const;

	// These functions have direct script equivalents, but may also get used directly from the engine.
	void AddEventForObject(const char *pcEventName);
	void RemoveEventForObject(const char *pcEventName);
	inline void RemoveAllEventsForObject();
	bool8 DidObjectSendLastNamedEvent(uint32 nObjectID, const char *pcEventName) const;
	int32 GetIDOfLastObjectToPostEvent(const char *pcEventName) const;
	bool8 PostNamedEvent(const char *pcEventName, int32 nSenderID);
	void ClearAllOutstandingEvents();

private:
	const char *m_pcObjectName;                            // Name of the object.
	_event m_pNamedEventList[EVENT_MAX_EVENTS_REGISTERED]; // Named events this object is interested in.
	uint8 m_nNumNamedEventsPending;                        // Number of named events pending.
	uint8 m_nNumRegisteredEvents;                          // Number of events object is currently interested in.
	bool8 m_bEventPending;                                 // If true, at least one event is pending.
	uint8 m_nPad1;

	// Private functions used only inside this class.
	void Initialise();
};

inline _event_list::_event_list() {
	Initialise();
	m_pcObjectName = EVENT_DELETED_PLACEHOLDER;
}

inline void _event_list::SetNewObjectName(const char *pcObjectName) {
	Initialise();
	m_pcObjectName = pcObjectName;
}

inline bool8 _event_list::HasEventPending() {
	bool8 bRetVal;

	// This function has no choice but to clear this flag, for two reasons: firstly, line-of-sight events don't
	// get explicitly handled by a particular piece of code and so there is nowhere to clear the event if we
	// don't do it here; secondly, even if named events are outstanding, I can't leave the event flag set because
	// the script writer might not be making any calls to CheckEventWaiting(), in which case the event flag again
	// would never get cleared.
	bRetVal = m_bEventPending;
	m_bEventPending = FALSE8;
	return (bRetVal);
}

inline void _event_list::RemoveAllEventsForObject() { Initialise(); }

} // End of namespace ICB

#endif // #ifndef EVENTLIST_H_INCLUDED
