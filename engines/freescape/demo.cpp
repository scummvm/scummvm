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

#include "freescape/freescape.h"

namespace Freescape {

void FreescapeEngine::generateDemoInput() {
	Common::Event event;
	if (isDOS()) {

		if (_currentDemoInputRepetition == 0) {
			_currentDemoInputRepetition = 1;
			_currentDemoInputCode = _demoData[_demoIndex++];
			if (_currentDemoInputCode & 0x80) {
				_currentDemoInputRepetition = (_currentDemoInputCode & 0x7F) /*+ 1*/;
				//if (_currentDemoInputRepetition == 1)
				//	_currentDemoInputRepetition = 255;
				_currentDemoInputCode = _demoData[_demoIndex++];
			}
		}

		if (_currentDemoInputCode >= 0x16 && _currentDemoInputCode <= 0x1a) {
			event = decodeDOSMouseEvent(_currentDemoInputCode, _currentDemoInputRepetition);

			Common::Point resolution = _gfx->nativeResolution();
			event.mouse.x = resolution.x * event.mouse.x / _screenW;
			event.mouse.y = resolution.y * event.mouse.y / _screenH ;

			_demoEvents.push_back(event);
			g_system->delayMillis(10);
			_currentDemoInputRepetition = 0;
		} else if (_currentDemoInputCode == 0x7f) {
			// NOP
			_currentDemoInputRepetition--;
		} else if (_currentDemoInputCode == 0x0) {
			_forceEndGame = true;
		} else {
			event = Common::Event();
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = (Common::KeyCode)decodeDOSKey(_currentDemoInputCode);
			event.customType = 0xde00;
			_demoEvents.push_back(event);
			debugC(1, kFreescapeDebugMove, "Pushing key: %x with repetition %d", event.kbd.keycode, _currentDemoInputRepetition);
			g_system->delayMillis(100);
			_currentDemoInputRepetition--;
		}

		return;
	}

	int mouseX = _demoData[_demoIndex++] << 1;
	int mouseY = _demoData[_demoIndex++];
	debugC(1, kFreescapeDebugMove, "Mouse moved to: %d, %d", mouseX, mouseY);

	event.type = Common::EVENT_MOUSEMOVE;
	event.mouse = Common::Point(mouseX, mouseY);
	event.customType = 0xde00;

	byte nextKeyCode = _demoData[_demoIndex++];

	if (nextKeyCode == 0x30) {
		Common::Event spaceEvent;
		spaceEvent.type = Common::EVENT_KEYDOWN;
		spaceEvent.kbd.keycode = Common::KEYCODE_SPACE;
		spaceEvent.customType = 0xde00;

		_demoEvents.push_back(spaceEvent);
		_demoEvents.push_back(event); // Mouse pointer is moved
		event.type = Common::EVENT_LBUTTONDOWN; // Keep same event fields
		_demoEvents.push_back(event); // Mouse is clicked
		_demoEvents.push_back(spaceEvent);
		nextKeyCode = _demoData[_demoIndex++];
	}

	while (nextKeyCode != 0) {
		event = Common::Event();
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = (Common::KeyCode)decodeAmigaAtariKey(nextKeyCode);
		debugC(1, kFreescapeDebugMove, "Pushing key: %x", event.kbd.keycode);
		event.customType = 0xde00;
		_demoEvents.push_back(event);
		nextKeyCode = _demoData[_demoIndex++];
	}
	assert(!nextKeyCode);
	g_system->delayMillis(100);
}

Common::Event FreescapeEngine::decodeDOSMouseEvent(int index, int repetition) {
	Common::Event event;
	event.type = Common::EVENT_MOUSEMOVE;
	event.customType = 0xde00;
	switch(index) {
		case 0x16:
			assert(repetition == 1);
			event.type = Common::EVENT_LBUTTONDOWN;
			break;
		case 0x17:
			_currentDemoMousePosition.x += repetition;
			break;
		case 0x18:
			_currentDemoMousePosition.x -= repetition;
			break;
		case 0x19:
			_currentDemoMousePosition.y += repetition;
			break;
		case 0x1a:
			_currentDemoMousePosition.y -= repetition;
			break;
		default:
			error("Unreachable");
	}
	event.mouse = _currentDemoMousePosition;
	return event;
}

int FreescapeEngine::decodeAmigaAtariKey(int index) {
	switch (index) {
	case 0x41:
		return Common::KEYCODE_a;
	case 0x44:
		return Common::KEYCODE_d;
	case 0x46:
		return Common::KEYCODE_f;
	case 0x4c:
		return Common::KEYCODE_l;
	case 0x4e:
		return Common::KEYCODE_n;
	case 0x50:
		return Common::KEYCODE_p;
	case 0x52:
		return Common::KEYCODE_r;
	case 0x53:
		return Common::KEYCODE_s;
	case 0x55:
		return Common::KEYCODE_u;
	case 0x58:
		return Common::KEYCODE_x;
	case 0x5a:
		return Common::KEYCODE_z;
	case 0x5f:
		return Common::KEYCODE_UNDERSCORE;
	case 0x96:
		return Common::KEYCODE_UP;
	case 0x97:
		return Common::KEYCODE_DOWN;
	case 0x98:
		return Common::KEYCODE_w; // Right
	case 0x99:
		return Common::KEYCODE_q; // Left
	default:
		error("Invalid key index: %x", index);
	}
	return 0;
}

int FreescapeEngine::decodeDOSKey(int index) {
	switch (index) {
	case 1:
		return Common::KEYCODE_r;
	case 2:
		return Common::KEYCODE_f;
	case 3:
		return Common::KEYCODE_UP;
	case 4:
		return Common::KEYCODE_DOWN;
	case 5:
		return Common::KEYCODE_q;
	case 6:
		return Common::KEYCODE_w;
	case 7:
		return Common::KEYCODE_p;
	case 8:
		return Common::KEYCODE_l;
	case 30:
		return Common::KEYCODE_SPACE;
	case 40:
		return Common::KEYCODE_d;
	default:
		error("Invalid key index: %x", index);
	}
	return 0;
}

} // End of namespace Freescape
