/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
 * Copyright (C) 2002-2005 Chris Apers - PalmOS Backend
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "be_zodiac.h"

void OSystem_PalmZodiac::get_coordinates(EventPtr ev, Coord &x, Coord &y) {
	Boolean dummy;
	EvtGetPenNative(WinGetDisplayWindow(), &ev->screenX, &ev->screenY, &dummy);

	x = (ev->screenX - _screenOffset.x);
	y = (ev->screenY - _screenOffset.y);

	if (_stretched) {
		Int32 w, h;

		h = (_ratio.adjustAspect == kRatioHeight ? _ratio.height : gVars->screenFullHeight);
		w = (_ratio.adjustAspect == kRatioWidth ? _ratio.width : gVars->screenFullWidth);
		x = (_screenWidth * x) / w;
		y = (_screenHeight * y) / h;
	}
}

bool OSystem_PalmZodiac::check_event(Event &event, EventPtr ev) {
	if (ev->eType == keyDownEvent) {
		switch (ev->data.keyDown.chr) {
		// F5 = menu
		case vchrThumbWheelBack:
			_lastKey = kKeyNone;
			event.type = EVENT_KEYDOWN;
			event.kbd.keycode = 319;
			event.kbd.ascii = 319;
			event.kbd.flags = 0;
			return true;

		case vchrTriggerLeft:
			hotswap_gfx_mode(_mode == GFX_WIDE ? GFX_NORMAL: GFX_WIDE);
			return false; // not a key

		case vchrTriggerRight:
			setFeatureState(kFeatureAspectRatioCorrection, 0);
			return false; // not a key

		case vchrRockerCenter:
		case vchrActionLeft:
			_lastKey = kKeyMouseLButton;
			event.type = EVENT_LBUTTONDOWN;
			event.mouse.x = _mouseCurState.x;
			event.mouse.y = _mouseCurState.y;
			return true;

		case vchrActionRight:
			_lastKey = kKeyNone;
			event.type = EVENT_RBUTTONDOWN;
			event.mouse.x = _mouseCurState.x;
			event.mouse.y = _mouseCurState.y;
			return true;

		// skip text
		case vchrActionDown:
			event.type = EVENT_KEYDOWN;
			event.kbd.keycode = '.';
			event.kbd.ascii = '.';
			event.kbd.flags = 0;
			return true;

		// trun off
		case vchrAutoOff:
		case vchrPowerOff:
			// pause the sound thread if any
			if (_sound.active)
				SndStreamPause(_sound.handle, true);
			break;
		
		case vchrLateWakeup:
			// resume the sound thread if any
			if (_sound.active)
				SndStreamPause(_sound.handle, false);
			break;
		}

	} else if (_overlayVisible) {
		switch (_lastKey) {
		case kKeyMouseLButton:
			_lastKey = kKeyNone;
			event.type = EVENT_LBUTTONUP;
			event.mouse.x = _mouseCurState.x;
			event.mouse.y = _mouseCurState.y;
			return true;

		}
	}

	return false;
}
