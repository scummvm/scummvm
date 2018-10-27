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

#include "gargoyle/events.h"

namespace Gargoyle {

void Events::getEvent(event_t *event, bool polled) {
	// TODO
}

void Events::store(EvType type, Window *win, uint32 val1, uint32 val2) {
	Event ev(type, win, val1, val2);

	switch (type) {
	case evtype_Arrange:
	case evtype_Redraw:
	case evtype_SoundNotify:
	case evtype_Timer:
		_eventsPolled.push(ev);
		break;

	default:
		_eventsLogged.push(ev);
		break;
	}
}

void Events::dispatchEvent(Event &ev, bool polled) {
	Event dispatch;

	if (!polled) {
		dispatch = _eventsLogged.retrieve();
		if (dispatch)
			dispatch = _eventsPolled.retrieve();
	} else {
		dispatch = _eventsPolled.retrieve();
	}

	if (dispatch)
		ev = dispatch;
}

} // End of namespace Gargoyle
