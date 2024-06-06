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
#include "common/textconsole.h"
#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/system/input/input_recorder.h"
#include "qdengine/core/system/input/input_wndproc.h"
#include "qdengine/core/system/input/keyboard_input.h"
#include "qdengine/core/system/input/mouse_input.h"


namespace QDEngine {

namespace input {

bool keyboard_wndproc(const MSG &msg, keyboardDispatcher *dsp) {
	switch (msg.message) {
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		dsp->handle_event((int)msg.wParam, true);
		inputRecorder::instance().dispatch_message(msg);
		return true;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		dsp->handle_event((int)msg.wParam, false);
		inputRecorder::instance().dispatch_message(msg);
		return true;
	}
	return false;
}

bool mouse_wndproc(const MSG &msg, mouseDispatcher *dsp) {
	int x, y;
	switch (msg.message) {
	case WM_MOUSEMOVE:
		x = LOWORD(msg.lParam);
		y = HIWORD(msg.lParam);
		dsp->handle_event(mouseDispatcher::EV_MOUSE_MOVE, x, y, msg.wParam);
		inputRecorder::instance().dispatch_message(msg);
		return true;
	case WM_LBUTTONDOWN:
		x = LOWORD(msg.lParam);
		y = HIWORD(msg.lParam);
		dsp->handle_event(mouseDispatcher::EV_LEFT_DOWN, x, y, msg.wParam);
		inputRecorder::instance().dispatch_message(msg);
		return true;
	case WM_RBUTTONDOWN:
		x = LOWORD(msg.lParam);
		y = HIWORD(msg.lParam);
		dsp->handle_event(mouseDispatcher::EV_RIGHT_DOWN, x, y, msg.wParam);
		inputRecorder::instance().dispatch_message(msg);
		return true;
	case WM_LBUTTONUP:
		x = LOWORD(msg.lParam);
		y = HIWORD(msg.lParam);
		dsp->handle_event(mouseDispatcher::EV_LEFT_UP, x, y, msg.wParam);
		inputRecorder::instance().dispatch_message(msg);
		return true;
	case WM_RBUTTONUP:
		x = LOWORD(msg.lParam);
		y = HIWORD(msg.lParam);
		dsp->handle_event(mouseDispatcher::EV_RIGHT_UP, x, y, msg.wParam);
		inputRecorder::instance().dispatch_message(msg);
		return true;
	}

	return false;
}

}; /* namespace input */

} // namespace QDEngine
