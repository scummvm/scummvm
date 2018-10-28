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
#include "gargoyle/clipboard.h"
#include "gargoyle/gargoyle.h"
#include "gargoyle/windows.h"

namespace Gargoyle {

void Events::getEvent(event_t *event, bool polled) {
	_currentEvent  = event;
	event->clear();

	Common::Event ev;
	dispatchEvent(*_currentEvent, polled);

	if (!polled) {
		while (!g_vm->shouldQuit() && _currentEvent->type == evtype_None && !_timeouts) {
			pollEvents();
			g_system->delayMillis(10);

			dispatchEvent(*_currentEvent, polled);
		}
	}

	if (_currentEvent->type == evtype_None && _timeouts) {
		store(evtype_Timer, NULL, 0, 0);
		dispatchEvent(*_currentEvent, polled);
		_timeouts = false;
	}

	_currentEvent = nullptr;
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

void Events::pollEvents() {
	Common::Event event;

	do {
		g_system->getEventManager()->pollEvent(event);

		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			handleKeyDown(event.kbd);
			return;
		case Common::EVENT_WHEELUP:
		case Common::EVENT_WHEELDOWN:
			handleScroll(event.type == Common::EVENT_WHEELUP);
			return;
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:

		default:
			break;
		}
	} while (event.type == Common::EVENT_MOUSEMOVE);
}

void Events::handleKeyDown(const Common::KeyState &ks) {
	Clipboard &clipboard = *g_vm->_clipboard;
	Windows &windows = *g_vm->_windows;

	if (ks.flags & Common::KBD_CTRL) {
		if (ks.keycode == Common::KEYCODE_a)
			windows.inputHandleKey(keycode_Home);
		else if (ks.keycode == Common::KEYCODE_c)
			clipboard.send(CLIPBOARD);
		else if (ks.keycode == Common::KEYCODE_e)
			windows.inputHandleKey(keycode_End);
		else if (ks.keycode == Common::KEYCODE_u)
			windows.inputHandleKey(keycode_Escape);
		else if (ks.keycode == Common::KEYCODE_v)
			clipboard.receive(CLIPBOARD);
		else if (ks.keycode == Common::KEYCODE_x)
			clipboard.send(CLIPBOARD);
		else if (ks.keycode == Common::KEYCODE_LEFT || ks.keycode == Common::KEYCODE_KP4)
			windows.inputHandleKey(keycode_SkipWordLeft);
		else if (ks.keycode == Common::KEYCODE_RIGHT || ks.keycode == Common::KEYCODE_KP6)
			windows.inputHandleKey(keycode_SkipWordRight);

		return;
	}

	if (ks.flags & Common::KBD_ALT)
		return;

	if (ks.keycode == Common::KEYCODE_RETURN) windows.inputHandleKey(keycode_Return);
	else if (ks.keycode == Common::KEYCODE_BACKSPACE) windows.inputHandleKey(keycode_Delete);
	else if (ks.keycode == Common::KEYCODE_DELETE) windows.inputHandleKey(keycode_Erase);
	else if (ks.keycode == Common::KEYCODE_TAB) windows.inputHandleKey(keycode_Tab);
	else if (ks.keycode == Common::KEYCODE_UP) windows.inputHandleKey(keycode_PageUp);
	else if (ks.keycode == Common::KEYCODE_PAGEDOWN) windows.inputHandleKey(keycode_PageDown);
	else if (ks.keycode == Common::KEYCODE_HOME) windows.inputHandleKey(keycode_Home);
	else if (ks.keycode == Common::KEYCODE_END) windows.inputHandleKey(keycode_End);
	else if (ks.keycode == Common::KEYCODE_LEFT) windows.inputHandleKey(keycode_Left);
	else if (ks.keycode == Common::KEYCODE_RIGHT) windows.inputHandleKey(keycode_Right);
	else if (ks.keycode == Common::KEYCODE_UP) windows.inputHandleKey(keycode_Up);
	else if (ks.keycode == Common::KEYCODE_DOWN) windows.inputHandleKey(keycode_Down);
	else if (ks.keycode == Common::KEYCODE_ESCAPE) windows.inputHandleKey(keycode_Escape);
	else if (ks.keycode == Common::KEYCODE_F1) windows.inputHandleKey(keycode_Func1);
	else if (ks.keycode == Common::KEYCODE_F2) windows.inputHandleKey(keycode_Func2);
	else if (ks.keycode == Common::KEYCODE_F3) windows.inputHandleKey(keycode_Func3);
	else if (ks.keycode == Common::KEYCODE_F4) windows.inputHandleKey(keycode_Func4);
	else if (ks.keycode == Common::KEYCODE_F5) windows.inputHandleKey(keycode_Func5);
	else if (ks.keycode == Common::KEYCODE_F6) windows.inputHandleKey(keycode_Func6);
	else if (ks.keycode == Common::KEYCODE_F7) windows.inputHandleKey(keycode_Func7);
	else if (ks.keycode == Common::KEYCODE_F8) windows.inputHandleKey(keycode_Func8);
	else if (ks.keycode == Common::KEYCODE_F9) windows.inputHandleKey(keycode_Func9);
	else if (ks.keycode == Common::KEYCODE_F10) windows.inputHandleKey(keycode_Func10);
	else if (ks.keycode == Common::KEYCODE_F11) windows.inputHandleKey(keycode_Func11);
	else if (ks.keycode == Common::KEYCODE_F12) windows.inputHandleKey(keycode_Func12);
	else windows.inputHandleKey(ks.ascii);
}

void Events::handleScroll(bool wheelUp) {
	g_vm->_windows->inputHandleKey(wheelUp ? keycode_MouseWheelUp : keycode_MouseWheelDown);
}

void Events::handleMouseMove(const Common::Point &pos) {
	// hyperlinks and selection
	// TODO: Properly handle commented out lines
	if (g_vm->_copySelect) {
		//gdk_window_set_cursor((GTK_WIDGET(widget)->window), gdk_ibeam);
		g_vm->_windowMask->moveSelection(pos);
	} else {
		if (g_vm->_windowMask->getHyperlink(pos)) {
			//gdk_window_set_cursor((GTK_WIDGET(widget)->window), gdk_hand);
		} else {
			//gdk_window_set_cursor((GTK_WIDGET(widget)->window), NULL);
		}
	}
}

void Events::handleButtonDown(bool isLeft, const Common::Point &pos) {
	if (isLeft)
		g_vm->_windows->inputHandleClick(pos);
	else
		g_vm->_clipboard->receive(PRIMARY);
}

void Events::handleButtonUp(bool isLeft, const Common::Point &pos) {
	if (isLeft) {
		g_vm->_copySelect = false;
		//gdk_window_set_cursor((GTK_WIDGET(widget)->window), NULL);
		g_vm->_clipboard->send(PRIMARY);
	}
}

} // End of namespace Gargoyle
