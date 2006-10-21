/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "be_os5.h"

void OSystem_PalmOS5::get_coordinates(EventPtr ev, Coord &x, Coord &y) {
	Boolean dummy;
	EvtGetPenNative(WinGetDisplayWindow(), &ev->screenX, &ev->screenY, &dummy);
	
	x = (ev->screenX - _screenOffset.x);
	y = (ev->screenY - _screenOffset.y);
	
	if (_stretched) {
		if (OPTIONS_TST(kOptModeLandscape)) {
			x = (x * 2 / 3);
			y = (y * 2 / 3);
		} else {
			y =       ((ev->screenX - _screenOffset.y) * 2) / 3;
			x = 320 - ((ev->screenY - _screenOffset.x) * 2) / 3 - 1;	
		}
	}
}

bool OSystem_PalmOS5::check_event(Event &event, EventPtr ev) {
	if (ev->eType == keyDownEvent) {
		switch (ev->data.keyDown.chr) {
		case vchrHard4:
			_lastKey = kKeyNone;
			event.type = EVENT_RBUTTONDOWN;
			event.mouse.x = _mouseCurState.x;
			event.mouse.y = _mouseCurState.y;
			return true;
		}
		
		if (_keyMouse.hasMore) {
			switch (ev->data.keyDown.chr) {
			// hot swap gfx
			case vchrHard1:
				if (OPTIONS_TST(kOptCollapsible))
					hotswap_gfx_mode(_mode == GFX_WIDE ? GFX_NORMAL: GFX_WIDE);
				return false; // not a key

			// ESC key
			case vchrHard2:
				_lastKey = kKeyNone;
				event.type = EVENT_KEYDOWN;
				event.kbd.keycode = 27;
				event.kbd.ascii = 27;
				event.kbd.flags = 0;
				return true;
			
			// F5 = menu
			case vchrHard3:
				_lastKey = kKeyNone;
				event.type = EVENT_KEYDOWN;
				event.kbd.keycode = 319;
				event.kbd.ascii = 319;
				event.kbd.flags = 0;
				return true;
			}
		}
	}

	return false;
}
