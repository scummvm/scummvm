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

#include "engines/icb/event_list.h"
#include "engines/icb/debug.h"

namespace ICB {

// Use globals to save on rdata storage and having repeated strings therein
const char *global_event_deleted_placeholder = "**";
const char *global_event_generic = "generic";
const char *global_event_line_of_sight = "line_of_sight";
const char *global_event_sound = "sound";
const char *global_event_on_camera = "on_camera";
const char *global_event_off_camera = "off_camera";
const char *global_event_logic_rerun = "logic_rerun";
const char *global_event_out_of_sight = "out_of_sight";
const char *global_event_lift_ascend = "lift_ascend";
const char *global_event_lift_descend = "lift_descend";

_event_list::_event_list(const _event_list &oX) {
	uint32 i;

	m_pcObjectName = oX.m_pcObjectName;
	m_nNumNamedEventsPending = oX.m_nNumNamedEventsPending;
	m_nNumRegisteredEvents = oX.m_nNumRegisteredEvents;
	m_bEventPending = oX.m_bEventPending;
	m_nPad1 = 0;

	for (i = 0; i < m_nNumRegisteredEvents; ++i)
		m_pNamedEventList[i] = oX.m_pNamedEventList[i];

	for (i = m_nNumRegisteredEvents; i < EVENT_MAX_EVENTS_REGISTERED; ++i)
		m_pNamedEventList[i].s_pcEventName = EVENT_DELETED_PLACEHOLDER;
}

const _event_list &_event_list::operator=(const _event_list &oOpB) {
	uint32 i;

	m_pcObjectName = oOpB.m_pcObjectName;
	m_nNumNamedEventsPending = oOpB.m_nNumNamedEventsPending;
	m_nNumRegisteredEvents = oOpB.m_nNumRegisteredEvents;
	m_bEventPending = oOpB.m_bEventPending;

	for (i = 0; i < m_nNumRegisteredEvents; ++i)
		m_pNamedEventList[i] = oOpB.m_pNamedEventList[i];

	for (i = m_nNumRegisteredEvents; i < EVENT_MAX_EVENTS_REGISTERED; ++i)
		m_pNamedEventList[i].s_pcEventName = EVENT_DELETED_PLACEHOLDER;

	return (*this);
}

bool8 _event_list::IsEventInList(const char *pcEventName) const {
	uint32 i;

	// Look for the event in the list.
	i = 0;
	while ((i < m_nNumRegisteredEvents) && strcmp(m_pNamedEventList[i].s_pcEventName, pcEventName))
		++i;

	// See which loop condition failed.
	if (i == m_nNumRegisteredEvents)
		return (FALSE8);
	else
		return (TRUE8);
}

void _event_list::AddEventForObject(const char *pcEventName) {
	uint32 i;

	// Look for a blank slot or the event already being in the list.
	i = 0;
	while ((i < m_nNumRegisteredEvents) && strcmp(m_pNamedEventList[i].s_pcEventName, EVENT_DELETED_PLACEHOLDER) && strcmp(m_pNamedEventList[i].s_pcEventName, pcEventName))
		++i;

	// Our index is now pointing at the first placeholder for a removed event or one-past-the-end-of-the-array or at
	// the event if it is already in the list.  Make sure the pending flag is not set.  However, we can't do this if
	// the event is already in the list, as there may be events of this type outstanding.
	if (i == m_nNumRegisteredEvents) {
		// We are increasing the size of the list.
		++m_nNumRegisteredEvents;
	}

	// If the event is already listed, don't overwrite it because we may destroy a pending event.
	if (strcmp(m_pNamedEventList[i].s_pcEventName, pcEventName)) {
		// Setting up a new event.
		// Set_string( pcEventName, m_pNamedEventList[ i ].s_pcEventName, MAXLEN_EVENT_NAME );
		m_pNamedEventList[i].s_pcEventName = const_cast<char *>(pcEventName);
		m_pNamedEventList[i].s_bPending = FALSE8;
		m_pNamedEventList[i].s_nLastSenderID = EVENT_INVALID_SENDER_ID;
	}
}

void _event_list::RemoveEventForObject(const char *pcEventName) {
	uint32 i;

	// Look for the event in the list of named events.
	i = 0;
	while ((i < m_nNumRegisteredEvents) && strcmp(m_pNamedEventList[i].s_pcEventName, pcEventName))
		++i;

	// If we didn't get to the end of the array then we found the event OK.
	if (i < m_nNumRegisteredEvents) {
		// So delete the event.
		m_pNamedEventList[i].s_pcEventName = EVENT_DELETED_PLACEHOLDER;
		m_pNamedEventList[i].s_nLastSenderID = EVENT_INVALID_SENDER_ID;

		// If the event was pending then we must remember to decrement the count of outstanding events
		// because the event will no longer be able to be cleared by checking for it.
		if (m_pNamedEventList[i].s_bPending) {
			m_pNamedEventList[i].s_bPending = FALSE8;
			--m_nNumNamedEventsPending;
		}

		// And return.
		return;
	}
}

void _event_list::ClearAllOutstandingEvents() {
	uint32 i;

	// Clear the generic event flag.
	m_bEventPending = FALSE8;

	// Now clear all the named events.
	for (i = 0; i < m_nNumRegisteredEvents; ++i)
		m_pNamedEventList[i].s_bPending = FALSE8;

	// Object has no outstanding named events.
	m_nNumNamedEventsPending = 0;
}

bool8 _event_list::PostNamedEvent(const char *pcEventName, int32 nSenderID) {
	uint32 i;

	// Look for the event in the list of events this object is interested in.
	i = 0;
	while ((i < m_nNumRegisteredEvents) && strcmp(m_pNamedEventList[i].s_pcEventName, pcEventName))
		++i;

	// If we didn't get to the end of the array then we found the event OK.
	if (i < m_nNumRegisteredEvents) {
		// Only one event of each type can be pending, so if the event is pending already, we are
		// not adding a new one.
		if (!m_pNamedEventList[i].s_bPending) {
			++m_nNumNamedEventsPending;
			m_pNamedEventList[i].s_bPending = TRUE8;
		}

		// Set the ID of the last sender of this event.
		m_pNamedEventList[i].s_nLastSenderID = nSenderID;

		// Also set the generic flag to trigger a rerun of context logic for the object.
		m_bEventPending = TRUE8;

		// Tell caller we added a new event.
		return (TRUE8);
	}

	// We're not registering a new event.
	return (FALSE8);
}

bool8 _event_list::CheckEventWaiting(const char *pcEventName) {
	uint32 i;

	// If we're not checking for line-of-sight events but there is one, we must clear it anyway.
	if (strcmp(pcEventName, EVENT_LINE_OF_SIGHT) && (m_pNamedEventList[0].s_bPending)) {
		m_pNamedEventList[0].s_bPending = FALSE8;
		--m_nNumNamedEventsPending;
	}

	// Quick check: if there are no events pending then the named one can't be pending.
	if (m_nNumNamedEventsPending == 0)
		return (FALSE8);

	// Look through the remaining named event list.
	for (i = 0; i < m_nNumRegisteredEvents; ++i) {
		if (!strcmp(m_pNamedEventList[i].s_pcEventName, pcEventName) && m_pNamedEventList[i].s_bPending) {
			// Clear the pending flag on the named event.
			m_pNamedEventList[i].s_bPending = FALSE8;
			--m_nNumNamedEventsPending;

			// Only clear the generic flag if we have just cleared the last named event.  Otherwise, we have to turn
			// it back on because HasEventsPending() always clears the flag regardless of whether or not there are
			// still any named events pending.
			if (m_nNumNamedEventsPending == 0)
				m_bEventPending = FALSE8;
			else
				m_bEventPending = TRUE8;

			// Tell caller that the named event was waiting for the object.
			return (TRUE8);
		}
	}

	// The object is either not registered for this event or the event is not pending.
	return (FALSE8);
}

bool8 _event_list::DidObjectSendLastNamedEvent(uint32 nObjectID, const char *pcEventName) const {
	uint32 i;

	// Find the event in the list of events the object is registered for.
	i = 0;
	while ((i < m_nNumRegisteredEvents) && strcmp(m_pNamedEventList[i].s_pcEventName, pcEventName))
		++i;

	// If we ran off the end of that array, then just return false, but log a warning.
	if (i == m_nNumRegisteredEvents) {
		Zdebug("_event_list::DidObjectSendLastEvent( %d, %s ) - event not listed for object.", nObjectID, pcEventName);
		return (FALSE8);
	}

	// Check if the object was the last sender of an event.
	if ((uint32)m_pNamedEventList[i].s_nLastSenderID == nObjectID)
		return (TRUE8);
	else
		return (FALSE8);
}

int32 _event_list::GetIDOfLastObjectToPostEvent(const char *pcEventName) const {
	uint32 i;

	// Find the event in the list of events the object is registered for.
	i = 0;
	while ((i < m_nNumRegisteredEvents) && strcmp(m_pNamedEventList[i].s_pcEventName, pcEventName))
		++i;

	// If we ran off the end of that array, then return EVENT_INVALID_SENDER_ID and log a warning.
	if (i == m_nNumRegisteredEvents) {
		Zdebug("_event_list::GetIDOfLastObjectToPostEvent( %s ) - event not listed for object.", pcEventName);
		return (EVENT_INVALID_SENDER_ID);
	}

	// Return the last sender ID.
	return (m_pNamedEventList[i].s_nLastSenderID);
}

void _event_list::Initialise() {
	uint32 i;

	// Initially, there are no pending events.
	m_nNumNamedEventsPending = 0;
	m_bEventPending = FALSE8;

	// All objects get a line-of-sight slot by default.
	m_pNamedEventList[0].s_pcEventName = EVENT_LINE_OF_SIGHT;
	m_pNamedEventList[0].s_nLastSenderID = EVENT_INVALID_SENDER_ID;
	m_pNamedEventList[0].s_bPending = FALSE8;
	m_nNumRegisteredEvents = 1;

	// Set all remaining slots to be empty.
	for (i = 1; i < EVENT_MAX_EVENTS_REGISTERED; ++i)
		m_pNamedEventList[i].s_pcEventName = EVENT_DELETED_PLACEHOLDER;
}

} // End of namespace ICB
