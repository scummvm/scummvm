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

#include "engines/icb/event_manager.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/mission.h"

#include "common/stream.h"

namespace ICB {

bool8 _event_manager::HasEventPending(int32 nObjectID) {
	// First check the ID is in range and the object is running.
	if ((nObjectID < 0) || ((uint32)nObjectID >= m_nNumObjects))
		Fatal_error("Object ID %d out of range in _event_manager::HasEventPending()", nObjectID);

	// If the object is not running, simply return false.
	if (!m_pbRunning[nObjectID] || m_pbSuspended[nObjectID])
		return (FALSE8);

	// Return the flag for the object.
	return (m_pEventLists[nObjectID].HasEventPending());
}

bool8 _event_manager::CheckEventWaitingForObject(int32 nObjectID, const char *pcEventName) {
	// First check the ID is in range and the object is running.
	if ((nObjectID < 0) || ((uint32)nObjectID >= m_nNumObjects))
		Fatal_error("Object ID %d out of range in _event_manager::CheckEventWaitingForObject()", nObjectID);

	// If the object is not running, simply return false.
	if (!m_pbRunning[nObjectID] || m_pbSuspended[nObjectID])
		return (FALSE8);

	// Check the object's event list.
	return (m_pEventLists[nObjectID].CheckEventWaiting(pcEventName));
}

void _event_manager::ShutDownEventProcessingForObject(int32 nObjectID) {
	// First check the ID is in range and the object is running.
	if ((nObjectID < 0) || ((uint32)nObjectID >= m_nNumObjects))
		Fatal_error("Object ID %d out of range in _event_manager::ShutDownEventProcessingForObject()", nObjectID);

	// Mark it shut down.
	m_pbRunning[nObjectID] = FALSE8;
}

void _event_manager::RegisterForEvent(int32 nObjectID, const char *pcEventName) {
	// First check the ID is in range and the object is running.
	if ((nObjectID < 0) || ((uint32)nObjectID >= m_nNumObjects))
		Fatal_error("Object ID %d out of range in _event_manager::RegisterForEvent()", nObjectID);

	// Add the event for the object.
	if (m_pbRunning[nObjectID])
		m_pEventLists[nObjectID].AddEventForObject(pcEventName);
}

void _event_manager::UnregisterForEvent(int32 nObjectID, const char *pcEventName) {
	// First check the ID is in range and the object is running.
	if ((nObjectID < 0) || ((uint32)nObjectID >= m_nNumObjects))
		Fatal_error("Object ID %d out of range in _event_manager::UnregisterForEvent()", nObjectID);

	// Make sure the object is running.
	if (m_pbRunning[nObjectID]) {
		// Remove event for the object.
		m_pEventLists[nObjectID].RemoveEventForObject(pcEventName);

		// This is a useful opportunity to do some cleaning up.  Having worked with the implementers, it
		// seems like desirable behaviour that all events get cleared when you unregister an object for one.
		m_pEventLists[nObjectID].ClearAllOutstandingEvents();
	}
}

void _event_manager::ClearAllEventsForObject(int32 nObjectID) {
	// First check the ID is in range and the object is running.
	if ((nObjectID < 0) || ((uint32)nObjectID >= m_nNumObjects))
		Fatal_error("Object ID %d out of range in _event_manager::ClearAllEventsForObject()", nObjectID);

	// Clear the object's events.
	if (m_pbRunning[nObjectID])
		m_pEventLists[nObjectID].ClearAllOutstandingEvents();
}

void _event_manager::SetSuspendFlagForObject(int32 nObjectID, bool8 bState) {
	// First check the ID is in range and the object is running.
	if ((nObjectID < 0) || ((uint32)nObjectID >= m_nNumObjects))
		Fatal_error("Object ID %d out of range in _event_manager::SetSuspendFlagForObject()", nObjectID);

	// Flag the object suspended or not.
	m_pbSuspended[nObjectID] = bState;
}

bool8 _event_manager::DidObjectSendLastNamedEvent(int32 nCallerID, int32 nObjectID, const char *pcEventName) const {
	if ((nCallerID < 0) || ((uint32)nCallerID >= m_nNumObjects))
		Fatal_error("Caller ID %d out of range in _event_manager::DidObjectSendLastEvent()", nCallerID);

	if ((nObjectID < 0) || ((uint32)nObjectID >= m_nNumObjects))
		Fatal_error("Object ID %d out of range in _event_manager::DidObjectSendLastEvent()", nObjectID);

	// Get the ID of last poster of named event.
	if (!m_pbRunning[nCallerID] || m_pbSuspended[nCallerID])
		return (FALSE8);
	else
		return (m_pEventLists[nCallerID].DidObjectSendLastNamedEvent(nObjectID, pcEventName));
}

int32 _event_manager::GetIDOfLastObjectToPostEvent(int32 nCallerID, const char *pcEventName) const {
	if ((nCallerID < 0) || ((uint32)nCallerID >= m_nNumObjects))
		Fatal_error("Caller ID %d out of range in _event_manager::GetIDOfLastObjectToPostEvent()", nCallerID);

	// Get the ID of last poster of named event.
	if (!m_pbRunning[nCallerID] || m_pbSuspended[nCallerID])
		return (FALSE8);
	else
		return (m_pEventLists[nCallerID].GetIDOfLastObjectToPostEvent(pcEventName));
}

void _event_manager::PostNamedEventToObject(const char *pcEventName, int32 nTargetID, int32 nSenderID) {
	if ((nSenderID < 0) || ((uint32)nSenderID >= m_nNumObjects))
		Fatal_error("Sender ID %d out of range in _event_manager::PostNamedEventToObject()", nSenderID);

	if ((nTargetID < 0) || ((uint32)nTargetID >= m_nNumObjects))
		Fatal_error("Target ID %d out of range in _event_manager::PostNamedEventToObject()", nTargetID);

	// Post the named event for one object only.
	if (m_pbRunning[nTargetID] || m_pbSuspended[nTargetID])
		m_pEventLists[nTargetID].PostNamedEvent(pcEventName, nSenderID);
}

bool8 _event_manager::IsObjectRegisteredForEvent(int32 nCallerID, const char *pcEventName) {
	if ((nCallerID < 0) || ((uint32)nCallerID >= m_nNumObjects))
		Fatal_error("Sender ID %d out of range in _event_manager::IsObjectRegisteredForEvent()", nCallerID);

	if (m_pbRunning[nCallerID])
		return (m_pEventLists[nCallerID].IsEventInList(pcEventName));
	else
		return (FALSE8);
}

void _event_manager::Initialise() {
	uint32 i;

	// Initialize the line-of-sight module.
	g_oLineOfSight->Initialise();

	// Add an entry for each of the objects in this session.
	m_nNumObjects = (uint8)MS->total_objects;
	memset(m_pbRunning, 0, MAX_session_objects * sizeof(bool8));
	memset(m_pbSuspended, 0, MAX_session_objects * sizeof(bool8));

	for (i = 0; i < m_nNumObjects; ++i) {
		// Create an event list for the object.
		m_pEventLists[i].SetNewObjectName(MS->logic_structs[i]->GetName());

		// Flag it running or not.
		if (MS->logic_structs[i]->ob_status == OB_STATUS_HELD)
			m_pbRunning[i] = FALSE8;
		else
			m_pbRunning[i] = TRUE8;
	}

	// To start with, no event timers are running.
	for (i = 0; i < EVENT_MANAGER_MAX_TIMERS; ++i)
		m_pbActiveTimers[i] = FALSE8;
}

void _event_manager::PostNamedEvent(const char *pcEventName, int32 nSenderID) {
	uint32 i;

	// Loop through each object in the session.
	for (i = 0; i < m_nNumObjects; ++i) {
		// Post the event only if the object is marked as running.
		if (m_pbRunning[i] && !m_pbSuspended[i])
			m_pEventLists[i].PostNamedEvent(pcEventName, nSenderID);
	}
}

void _event_manager::CycleEventManager() {
	uint32 i;
	uint32 nElapsed;

	// Loop for each timer.
	for (i = 0; i < EVENT_MANAGER_MAX_TIMERS; ++i) {
		// Watch out for unused slots.
		if (m_pbActiveTimers[i]) {
			// Process the timer.
			m_pEventTimers[i].SetEventTime(m_pEventTimers[i].GetEventTime() + 1);

			// If it has gone past the end then it has expired and so needs removing.
			if (m_pEventTimers[i].GetEventTime() > m_pEventTimers[i].GetEnd()) {
				// Delete this expired timer.
				m_pbActiveTimers[i] = FALSE8;
			} else {
				// Timer has not expired but might not even have started.
				if (m_pEventTimers[i].GetEventTime() >= m_pEventTimers[i].GetEnd()) {
					// Yes, it has started and has not expired.  So check if it matches some multiple of
					// the interval.
					nElapsed = m_pEventTimers[i].GetEventTime() - m_pEventTimers[i].GetStart();
					if ((m_pEventTimers[i].GetInterval() == 0) || ((nElapsed % m_pEventTimers[i].GetInterval()) == 0)) {
						// We need to generate an event.
						PostNamedEvent(m_pEventTimers[i].GetEventName(), m_pEventTimers[i].GetObjectID());
					}
				}
			}
		}
	}
}

void _event_manager::PostRepeatingEvent(const char *pcEventName, uint32 nStart, uint32 nInterval, uint32 nEnd) {
	uint32 i;

	// Look for a vacant slot.
	i = 0;
	while ((i < EVENT_MANAGER_MAX_TIMERS) && m_pbActiveTimers[i])
		++i;

	// If we ran off the end, we failed to find a spare slot.
	if (i == EVENT_MANAGER_MAX_TIMERS)
		Fatal_error("Run out of event timer slots in _event_manager::PostRepeatingEvent()");

	// Set up the timer.
	m_pEventTimers[i].SetEventName(pcEventName);
	m_pEventTimers[i].SetStart(nStart);
	m_pEventTimers[i].SetEnd(nEnd);
	m_pEventTimers[i].SetInterval(nInterval);
	m_pEventTimers[i].SetEventTime(0);

	// Flag it as running.
	m_pbActiveTimers[i] = TRUE8;
}

void _event_manager::Save(Common::WriteStream *stream) const {
	uint32 i;
	uint32 nNumActiveTimers = 0;

	// First, work out how many timers we are going to write.
	for (i = 0; i < EVENT_MANAGER_MAX_TIMERS; ++i) {
		if (m_pbActiveTimers[i])
			++nNumActiveTimers;
	}

	// Write the number of active event timers in the file.
	stream->write(&nNumActiveTimers, sizeof(nNumActiveTimers)); // TODO: DON'T WRITE LIKE THIS

	// Now write the actual active event timers.
	for (i = 0; i < EVENT_MANAGER_MAX_TIMERS; ++i) {
		if (m_pbActiveTimers[i])
			stream->write(&m_pEventTimers[i], sizeof(_event_timer)); // TODO: DON'T WRITE LIKE THIS
	}
}

void _event_manager::Restore(Common::SeekableReadStream *stream) {
	uint32 i;
	uint32 nNumActiveTimers;
	_event_timer oEventTimer;

	// Make sure all timers are currently deactivated.  This is the same as overwriting them.
	for (i = 0; i < EVENT_MANAGER_MAX_TIMERS; ++i)
		m_pbActiveTimers[i] = FALSE8;

	// Find out how many timers we are going to read in.
	nNumActiveTimers = stream->readUint32LE();
	if (stream->err())
		Fatal_error("Error restoring event timers from save file");

	// Read them in.
	for (i = 0; i < nNumActiveTimers; ++i) {
		if (stream->read(&oEventTimer, sizeof(oEventTimer)) != sizeof(oEventTimer)) { // TODO: We REALLY shouldn't read structs this way. FIXME HACK
			Fatal_error("Error restoring %d event timers from save file", nNumActiveTimers);
		} else {
			m_pEventTimers[i] = oEventTimer;
			m_pbActiveTimers[i] = TRUE8;
		}
	}
}

} // End of namespace ICB
