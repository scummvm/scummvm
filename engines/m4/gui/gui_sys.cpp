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

#include "m4/gui/gui_sys.h"
#include "m4/gui/gui_dialog.h"
#include "m4/gui/gui_event.h"
#include "m4/gui/gui_vmng.h"
#include "m4/core/mouse.h"
#include "m4/mem/memman.h"
#include "m4/vars.h"

namespace M4 {

static bool inScreen(ScreenContext *sc, int32 x, int32 y);

bool gui_system_init() {
	_G(systemHotkeys) = nullptr;
	return true;
}

void gui_system_shutdown() {
	Hotkey *myHotkeys, *tempHotkey;

	myHotkeys = _G(systemHotkeys);
	tempHotkey = myHotkeys;

	while (tempHotkey) {
		myHotkeys = myHotkeys->next;
		mem_free(tempHotkey);
		tempHotkey = myHotkeys;
	}
}

void gui_system_event_handler() {
	MouseEvent newMouseEvent;

	ScreenContext *myScreen;
	Hotkey *myHotkey;
	int32 parm1 = 0;
	bool handled;
	bool blocked;
	bool found;

	if (!_G(vmng_Initted))
		return;

	// Allow pending events to be processed
	g_events->process();

	// WORKAROUND: Keep the player_info up to date, in case the game
	// is saved directly via the GMM without moving the mouse
	if (_G(my_walker) && _G(player).walker_in_this_scene &&
			_G(player).walker_visible && INTERFACE_VISIBLE)
		player_update_info();

	// Deal with mouse events first..
	_G(mouseX) = _G(MouseState).CursorColumn;
	_G(mouseY) = _G(MouseState).CursorRow;

	newMouseEvent = mouse_get_event();

	if (newMouseEvent != _ME_no_event) {	// We have a mouse event
		gui_mouse_refresh();
		_G(oldX) = _G(mouseX);
		_G(oldY) = _G(mouseY);

		// currScreen is a global, the address of which is passed to every window's evtHandler.
		// If the evtHandler sets currScreen to true, then that window will receive all mouse events,
		// regardless of any windows in front of this one, until its evtHandler sets currScreen to false.
		if (_G(currScreen)) {
			if (_G(eventToScreen)->scrnFlags & SF_GET_MOUSE) {
				(_G(eventToScreen)->evtHandler)(_G(eventToScreen)->scrnContent, EVENT_MOUSE,
					newMouseEvent, _G(mouseX), _G(mouseY), &_G(currScreen));
			}
		} else {
			myScreen = _G(frontScreen);
			blocked = false;
			found = false;

			// Loop through the active window list, until we find a window which accepts
			// mouse events and/or one that blocks them.
			while (myScreen && (!found) && (!blocked)) {
				if (myScreen->scrnFlags & SF_BLOCK_MOUSE)
					blocked = true;
				if ((myScreen->scrnFlags & SF_GET_MOUSE) && inScreen(myScreen, _G(mouseX), _G(mouseY)))
					found = true;											// The mouse must be within the window's
				else
					myScreen = myScreen->behind;							// Boundaries to receive the event.
			}
			if (found && (myScreen->evtHandler)) {
				// NOTE: if the window accepts mouse events, and the mouse is within the window's
				// boundaries, the event will never be passed on.
				(myScreen->evtHandler)(myScreen->scrnContent, EVENT_MOUSE, newMouseEvent, _G(mouseX), _G(mouseY), &_G(currScreen));
			} else {
				_G(currScreen) = false;
			}

			if (_G(currScreen))
				_G(eventToScreen) = myScreen;
		}
	}

	// Check keyboard
	if (util_kbd_check(&parm1)) {
		// Scan window list for windows with event handlers, aborting the scan if we find
		// a window which handles or blocks key events. The event is passed to the handler
		// if found.
		//
		handled = false;
		myScreen = _G(frontScreen);
		found = false;
		blocked = false;
		while (myScreen && (!found) && (!blocked)) {	// Loop through windows until we find a window that accepts key events and/or
			if (myScreen->scrnFlags & SF_BLOCK_KEY)
				blocked = true;									// one that blocks them.
			if (myScreen->scrnFlags & SF_GET_KEY) {
				found = true;
				if (myScreen->evtHandler) {
					handled = (myScreen->evtHandler)(myScreen->scrnContent, EVENT_KEY, parm1, 0, 0, nullptr);
				}
			} else myScreen = myScreen->behind;
		}

		// Scan window list for windows with hotkey lists, aborting the scan if we find
		// a window which blocks key events. If the window has a hotkey list, we check if
		// the key event matches any of the hotkeys. If it does, scanning aborts, and that
		// key's callback is invoked. If it doesn't the search continues.
		blocked = false;
		while (myScreen && (!blocked) && (!handled)) {
			if (myScreen->scrnFlags & SF_BLOCK_KEY)
				blocked = true;
			if (myScreen->scrnFlags & SF_GET_KEY) {
				myHotkey = myScreen->scrnHotkeys;
				while (myHotkey && (!handled)) {
					if (myHotkey->myKey == parm1) {
						handled = true;
						_G(currScreen) = false;
						Dialog_KeyMouseCollision();
						if (myHotkey->callback) {
							(myHotkey->callback)((void *)static_cast<intptr>(parm1), (void *)myScreen->scrnContent);
						}
					} else myHotkey = myHotkey->next;
				}
			}
			myScreen = myScreen->behind;
		}

		// finally, if no window blocked the key, and no window handled the key, check
		// the list of system hotkeys, invoking the callback if one is found.
		if ((!handled) && (!blocked)) {
			myHotkey = _G(systemHotkeys);
			while (myHotkey && (!handled)) {
				if (myHotkey->myKey == parm1) {
					handled = true;
					_G(currScreen) = false;
					Dialog_KeyMouseCollision();
					if (myHotkey->callback) {
						(myHotkey->callback)((void *)static_cast<intptr>(parm1), nullptr);
					}
				} else myHotkey = myHotkey->next;
			}
		}
	} // end key handling check
}

void AddSystemHotkey(int32 myKey, HotkeyCB callback) {
	Hotkey *myHotkey;

	if (!_G(vmng_Initted)) {
		return;
	}

	myHotkey = _G(systemHotkeys);
	while (myHotkey && (myHotkey->myKey != myKey)) {
		myHotkey = myHotkey->next;
	}

	if (myHotkey) {
		myHotkey->callback = callback;
	} else {
		if ((myHotkey = (Hotkey *)mem_alloc(sizeof(Hotkey), "hotkey")) == nullptr) {
			return;
		}

		myHotkey->myKey = myKey;
		myHotkey->callback = callback;
		myHotkey->next = _G(systemHotkeys);
		_G(systemHotkeys) = myHotkey;
	}
}

void RemoveSystemHotkey(int32 myKey) {
	Hotkey *myHotkey, *tempHotkey;

	if (!_G(vmng_Initted))
		return;

	myHotkey = _G(systemHotkeys);
	tempHotkey = myHotkey;

	while (myHotkey && (myHotkey->myKey != myKey)) {
		if (tempHotkey != myHotkey) tempHotkey = tempHotkey->next;
		myHotkey = myHotkey->next;
	}

	if (myHotkey) {
		if (myHotkey == tempHotkey)
			_G(systemHotkeys) = _G(systemHotkeys)->next;
		else
			tempHotkey->next = myHotkey->next;

		mem_free(myHotkey);
	}
}

HotkeyCB GetSystemHotkey(int32 myKey) {
	Hotkey *myHotkey, *tempHotkey;
	if (!_G(vmng_Initted))
		return (nullptr);

	myHotkey = _G(systemHotkeys);
	tempHotkey = myHotkey;

	while (myHotkey && (myHotkey->myKey != myKey)) {
		if (tempHotkey != myHotkey)
			tempHotkey = tempHotkey->next;

		myHotkey = myHotkey->next;
	}
	if (myHotkey) {
		return myHotkey->callback;
	}

	return nullptr;
}

static bool inScreen(ScreenContext *sc, int32 x, int32 y) {
	if (x >= sc->x1 && x <= sc->x2 && y >= sc->y1 && y <= sc->y2)
		return true;

	return false;
}

} // End of namespace M4
