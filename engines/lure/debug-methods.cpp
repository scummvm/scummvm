/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "lure/debug-methods.h"
#include "lure/luredefs.h"

#include "lure/events.h"
#include "lure/surface.h"
#include "lure/screen.h"
#include "lure/res.h"
#include "lure/strings.h"
#include "lure/room.h"

#ifdef LURE_DEBUG

namespace Lure {

void showActiveHotspots() {
	char buffer[16384];
	char *lines[100];
	char *s = buffer;
	int numLines = 0;
	lines[0] = s;
	*s = '\0';

	Resources &resources = Resources::getReference();
	Mouse &mouse = Mouse::getReference();
	Events &events = Events::getReference();
	Screen &screen = Screen::getReference();

	HotspotList::iterator i = resources.activeHotspots().begin();
	for (; i != resources.activeHotspots().end(); ++i) {
		Hotspot &h = *i.operator*();
		lines[numLines++] = s;

		if (numLines == 16) {
			strcpy(s, "..more..");
			break;
		}

		sprintf(s, "%x", h.hotspotId());
		s += strlen(s);

		sprintf(s, "h pos=(%d,%d,%d) size=(%d,%d) - ",
			h.resource().roomNumber, h.x(), h.y(), h.width(), h.height());
		s += strlen(s);

		uint16 nameId = h.resource().nameId;
		if (nameId != 0) {
			StringData::getReference().getString(nameId, s, NULL, NULL);
			s += strlen(s);
		}
		++s;
	}

	Surface *surface = Surface::newDialog(300, numLines, lines);
	mouse.cursorOff();
	surface->copyToScreen(10, 40);
	events.waitForPress();
	screen.update();
	mouse.cursorOn();
	delete surface;
}

void showRoomHotspots() {
	char buffer[16384];
	char *lines[100];
	char *s = buffer;
	int numLines = 0;
	lines[0] = s;
	*s = '\0';

	Resources &resources = Resources::getReference();
	Mouse &mouse = Mouse::getReference();
	Events &events = Events::getReference();
	Screen &screen = Screen::getReference();
	uint16 roomNumber = Room::getReference().roomNumber();

	HotspotDataList::iterator i = resources.hotspotData().begin();
	for (; i != resources.hotspotData().end(); ++i) {
		HotspotData &h = *i.operator*();
		if (h.roomNumber == roomNumber) {
			lines[numLines++] = s;

			sprintf(s, "%x", h.hotspotId);
			s += strlen(s);

			sprintf(s, "h pos=(%d,%d) size=(%d,%d) - ",
				h.startX, h.startY, h.width, h.height);
			s += strlen(s);

			uint16 nameId = h.nameId;
			if (nameId != 0) {
				StringData::getReference().getString(nameId, s, NULL, NULL);
				s += strlen(s);
			}
			++s;
		}
	}

	Surface *surface = Surface::newDialog(300, numLines, lines);
	mouse.cursorOff();
	surface->copyToScreen(10, 40);
	events.waitForPress();
	screen.update();
	mouse.cursorOn();
	delete surface;
}


} // end of namespace Lure

#endif
