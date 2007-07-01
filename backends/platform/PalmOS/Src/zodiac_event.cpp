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

#include "be_zodiac.h"

bool OSystem_PalmZodiac::check_event(Common::Event &event, EventPtr ev) {
	if (ev->eType == keyUpEvent) {
		switch (ev->data.keyDown.chr) {
		case vchrActionLeft:
			event.type = Common::EVENT_LBUTTONUP;
			event.mouse.x = _mouseCurState.x;
			event.mouse.y = _mouseCurState.y;
			return true;

		case vchrActionRight:
			event.type = Common::EVENT_RBUTTONUP;
			event.mouse.x = _mouseCurState.x;
			event.mouse.y = _mouseCurState.y;
			return true;
		}

	} else if (ev->eType == keyDownEvent) {
		switch (ev->data.keyDown.chr) {
		// F5 = menu
		case vchrThumbWheelBack:
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = Common::KEYCODE_F5;
			event.kbd.ascii = Common::ASCII_F5;
			event.kbd.flags = 0;
			return true;

		case vchrTriggerLeft:
			hotswap_gfx_mode(_mode == GFX_WIDE ? GFX_NORMAL: GFX_WIDE);
			return false; // not a key

		case vchrTriggerRight:
			setFeatureState(kFeatureAspectRatioCorrection, 0);
			return false; // not a key

		case vchrActionLeft:
			event.type = Common::EVENT_LBUTTONDOWN;
			event.mouse.x = _mouseCurState.x;
			event.mouse.y = _mouseCurState.y;
			return true;

		case vchrActionRight:
			event.type = Common::EVENT_RBUTTONDOWN;
			event.mouse.x = _mouseCurState.x;
			event.mouse.y = _mouseCurState.y;
			return true;

		// skip text
		case vchrActionDown:
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = '.';
			event.kbd.ascii = '.';
			event.kbd.flags = 0;
			return true;

		// trun off
		case vchrAutoOff:
		case vchrPowerOff:
			// pause the sound thread if any
			if (_sound.active)
				SndStreamPause(_soundEx.handle, true);
			break;
		
		case vchrLateWakeup:
			// resume the sound thread if any
			if (_sound.active)
				SndStreamPause(_soundEx.handle, false);
			break;
		}

	}

	return false;
}
