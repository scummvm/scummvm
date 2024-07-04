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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/events.h"
#include "common/textconsole.h"
#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/system/input/input_wndproc.h"
#include "qdengine/core/system/input/keyboard_input.h"
#include "qdengine/core/system/input/mouse_input.h"


namespace QDEngine {

namespace input {

bool keyboard_wndproc(const Common::Event &event, keyboardDispatcher *dsp) {
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		dsp->handle_event((int)event.kbd.ascii, true);
		return true;
	case Common::EVENT_KEYUP:
		dsp->handle_event((int)event.kbd.ascii, false);
		return true;
	}
	return false;
}

bool mouse_wndproc(const Common::Event &event, mouseDispatcher *dsp) {
	int x, y;
	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		x = event.mouse.x;
		y = event.mouse.y;
		dsp->handle_event(mouseDispatcher::EV_MOUSE_MOVE, x, y, 0);
		return true;
	case Common::EVENT_LBUTTONDOWN:
		x = event.mouse.x;
		y = event.mouse.y;
		dsp->handle_event(mouseDispatcher::EV_LEFT_DOWN, x, y, 0);
		return true;
	case Common::EVENT_RBUTTONDOWN:
		x = event.mouse.x;
		y = event.mouse.y;
		dsp->handle_event(mouseDispatcher::EV_RIGHT_DOWN, x, y, 0);
		return true;
	case Common::EVENT_LBUTTONUP:
		x = event.mouse.x;
		y = event.mouse.y;
		dsp->handle_event(mouseDispatcher::EV_LEFT_UP, x, y, 0);
		return true;
	case Common::EVENT_RBUTTONUP:
		x = event.mouse.x;
		y = event.mouse.y;
		dsp->handle_event(mouseDispatcher::EV_RIGHT_UP, x, y, 0);
		return true;
	}

	return false;
}

}; /* namespace input */

} // namespace QDEngine
