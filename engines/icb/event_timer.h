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

#ifndef ICB_EVENTTIMER_H_INCLUDED
#define ICB_EVENTTIMER_H_INCLUDED

#include "engines/icb/string_vest.h"
#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_string.h"
#include "engines/icb/event_list.h"

namespace ICB {

// class _event_timer
// This structure holds timing information for events that are posted into the future or have
// to happen repeatedly.
class _event_timer {
public:
	// Default constructor and destructor.
	inline _event_timer();
	~_event_timer() {}

	// Copy constructor.
	inline _event_timer(const _event_timer &oX);

	// Operator '='.
	inline const _event_timer &operator=(const _event_timer &oOpB);

	// Gets and sets.
	void SetObjectID(int32 nObjectID) { m_nObjectID = nObjectID; }
	void SetStart(uint32 nStart) { m_nStart = nStart; }
	void SetEnd(uint32 nEnd) { m_nEnd = nEnd; }
	void SetInterval(uint32 nInterval) { m_nInterval = nInterval; }
	void SetEventTime(uint32 nTime) { m_nCurrentTime = nTime; }
	inline void SetEventName(const char *pcEventName);

	int32 GetObjectID() const { return (m_nObjectID); }
	uint32 GetStart() const { return (m_nStart); }
	uint32 GetEnd() const { return (m_nEnd); }
	uint32 GetInterval() const { return (m_nInterval); }
	uint32 GetEventTime() const { return (m_nCurrentTime); }
	const char *GetEventName() const { return (m_pcEventName); }

private:
	int32 m_nObjectID;                     // ID of object initiating this timer.
	uint32 m_nStart;                       // When the event should start, relative to the time the event was placed.
	uint32 m_nEnd;                         // When the event should end, relative to the time the event was placed.
	uint32 m_nInterval;                    // Interval between occurrences of the event.
	uint32 m_nCurrentTime;                 // Relative to the reference time.
	char m_pcEventName[MAXLEN_EVENT_NAME]; // Name of the event.
};

inline _event_timer::_event_timer() {
	m_nObjectID = EVENT_INVALID_SENDER_ID;
	m_nStart = 0;
	m_nEnd = 0;
	m_nInterval = 0;
	m_nCurrentTime = 0;
	m_pcEventName[0] = '\0';
}

inline _event_timer::_event_timer(const _event_timer &oX) {
	m_nObjectID = oX.m_nObjectID;
	m_nStart = oX.m_nStart;
	m_nEnd = oX.m_nEnd;
	m_nInterval = oX.m_nInterval;
	m_nCurrentTime = oX.m_nCurrentTime;
	strcpy(m_pcEventName, oX.m_pcEventName);
}

inline const _event_timer &_event_timer::operator=(const _event_timer &oOpB) {
	m_nObjectID = oOpB.m_nObjectID;
	m_nStart = oOpB.m_nStart;
	m_nEnd = oOpB.m_nEnd;
	m_nInterval = oOpB.m_nInterval;
	m_nCurrentTime = oOpB.m_nCurrentTime;
	strcpy(m_pcEventName, oOpB.m_pcEventName);

	return (*this);
}

inline void _event_timer::SetEventName(const char *pcEventName) {
	if (strlen(pcEventName) >= MAXLEN_EVENT_NAME)
		Fatal_error("Event name [%s] too long (max %d) in _event_timer::SetEventName()", pcEventName, MAXLEN_EVENT_NAME - 1);

	strcpy(m_pcEventName, pcEventName);
}

} // End of namespace ICB

#endif // #if !defined( EVENTTIMER_H_INCLUDED )
