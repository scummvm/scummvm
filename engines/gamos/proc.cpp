/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "gamos/gamos.h"

namespace Gamos {
	
void SystemProc::processMessage(const Common::Event &ev) {
	switch(ev.type) {
	case Common::EVENT_KEYDOWN:
		if ((_gd2flags & 1) == 0)
			return;
		
		_ascii = ev.kbd.ascii;

		if (ev.kbd.keycode == _keyCodes[0])
			_act1 = 0;
		else if (ev.kbd.keycode == _keyCodes[1])
			_act1 = 1;
		else if (ev.kbd.keycode == _keyCodes[2])
			_act1 = 2;
		else if (ev.kbd.keycode == _keyCodes[3])
			_act1 = 3;
		else if (ev.kbd.keycode == _keyCodes[4])
			_act1 = 4;
		else if (ev.kbd.keycode == _keyCodes[5])
			_act1 = 5;
		else if (ev.kbd.keycode == _keyCodes[6])
			_act1 = 6;
		else if (ev.kbd.keycode == _keyCodes[7])
			_act1 = 7;
		else {
			if (ev.kbd.keycode == _keyCodes[8])
				_act2 = ACT2_82;
			else if (ev.kbd.keycode == _keyCodes[9])
				_act2 = ACT2_83;
			else if (ev.kbd.keycode == _keyCodes[10])
				_act2 = ACT2_8f;
			else if (ev.kbd.keycode == _keyCodes[11])
				_act2 = ACT2_84;
			else 
				return;

			_rawKeyCode = ev.kbd.keycode;
			return;
		}

		if ((_act1 < 8) && (ev.kbd.flags & Common::KBD_SHIFT))
			_act1 |= 8;

		_mouseActPos = _mouseReportedPos;
		break;
	
	case Common::EVENT_MOUSEMOVE:
		if ((_gd2flags & 2) == 0)
			return;

		_mouseReportedPos = ev.mouse;
		
		break;
	
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN:
		if ((_gd2flags & 2) == 0)
			return;
		
		_mouseActPos = ev.mouse;
		_act2 = ACT2_81;		
		break;
	
	case Common::EVENT_LBUTTONUP:
		if ((_gd2flags & 2) == 0)
			return;
		
		_mouseActPos = ev.mouse;
		_act2 = ACT2_82;
		_rawKeyCode = _keyCodes[8];
		break;
	
	case Common::EVENT_RBUTTONUP:
		if ((_gd2flags & 2) == 0)
			return;
		
		_mouseActPos = ev.mouse;
		_act2 = ACT2_83;
		_rawKeyCode = _keyCodes[9];
		break;
	
	default:
		break;
	}
}

}