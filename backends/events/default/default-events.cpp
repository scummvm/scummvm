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

#if !defined(DISABLE_DEFAULT_EVENTMANAGER)

#include "common/system.h"
#include "common/config-manager.h"
#include "backends/events/default/default-events.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/remap-dialog.h"
#include "backends/vkeybd/virtual-keyboard.h"

#include "engines/engine.h"
#include "gui/message.h"

DefaultEventManager::DefaultEventManager(Common::EventSource *boss) :
	_buttonState(0),
	_modifierState(0),
	_shouldQuit(false),
	_shouldRTL(false),
	_confirmExitDialogActive(false) {

	assert(boss);

	_dispatcher.registerSource(boss, false);
	_dispatcher.registerSource(&_artificialEventSource, false);

	_dispatcher.registerObserver(this, kEventManPriority, false);

	// Reset key repeat
	_currentKeyDown.keycode = 0;

#ifdef ENABLE_VKEYBD
	_vk = new Common::VirtualKeyboard();
#endif
#ifdef ENABLE_KEYMAPPER
	_keymapper = new Common::Keymapper(this);
	// EventDispatcher will automatically free the keymapper
	_dispatcher.registerMapper(_keymapper);
	_remap = false;
#endif
}

DefaultEventManager::~DefaultEventManager() {
#ifdef ENABLE_VKEYBD
	delete _vk;
#endif
}

void DefaultEventManager::init() {
#ifdef ENABLE_VKEYBD
	if (ConfMan.hasKey("vkeybd_pack_name")) {
		_vk->loadKeyboardPack(ConfMan.get("vkeybd_pack_name"));
	} else {
		_vk->loadKeyboardPack("vkeybd_default");
	}
#endif
}

bool DefaultEventManager::pollEvent(Common::Event &event) {
	uint32 time = g_system->getMillis();
	bool result = false;

	_dispatcher.dispatch();
	if (!_eventQueue.empty()) {
		event = _eventQueue.pop();
		result = true;
	}

	if (result) {
		event.synthetic = false;
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_modifierState = event.kbd.flags;
			// init continuous event stream
			// not done on PalmOS because keyboard is emulated and keyup is not generated
#if !defined(PALMOS_MODE)
			_currentKeyDown.ascii = event.kbd.ascii;
			_currentKeyDown.keycode = event.kbd.keycode;
			_currentKeyDown.flags = event.kbd.flags;
			_keyRepeatTime = time + kKeyRepeatInitialDelay;
#endif
			// Global Main Menu
			if (event.kbd.flags == Common::KBD_CTRL && event.kbd.keycode == Common::KEYCODE_F5) {
				if (g_engine && !g_engine->isPaused()) {
					Common::Event menuEvent;
					menuEvent.type = Common::EVENT_MAINMENU;

					// FIXME: GSoC RTL branch passes the F6 key event to the
					// engine, and also enqueues a EVENT_MAINMENU. For now,
					// we just drop the key event and return an EVENT_MAINMENU
					// instead. This way, we don't have to add special cases
					// to engines (like it was the case for LURE in the RTL branch).
					//
					// However, this has other consequences, possibly negative ones.
					// Like, what happens with key repeat for the trigger key?

					//pushEvent(menuEvent);
					event = menuEvent;

					// FIXME: Since now we do not push another MAINMENU event onto
					// our event stack, the GMM would never open, so we have to do
					// that here. Of course when the engine would handle MAINMENU
					// as an event now and open up the GMM itself it would open the
					// menu twice.
					if (g_engine && !g_engine->isPaused())
						g_engine->openMainMenuDialog();

					if (_shouldQuit)
						event.type = Common::EVENT_QUIT;
					else if (_shouldRTL)
						event.type = Common::EVENT_RTL;
				}
			}
#ifdef ENABLE_VKEYBD
			else if (event.kbd.keycode == Common::KEYCODE_F7 && event.kbd.flags == 0) {
				if (_vk->isDisplaying()) {
					_vk->close(true);
				} else {
					if (g_engine)
						g_engine->pauseEngine(true);
					_vk->show();
					if (g_engine)
						g_engine->pauseEngine(false);
					result = false;
				}
			}
#endif
#ifdef ENABLE_KEYMAPPER
			else if (event.kbd.keycode == Common::KEYCODE_F8 && event.kbd.flags == 0) {
				if (!_remap) {
					_remap = true;
					Common::RemapDialog _remapDialog;
					if (g_engine)
						g_engine->pauseEngine(true);
					_remapDialog.runModal();
					if (g_engine)
						g_engine->pauseEngine(false);
					_remap = false;
				}
			}
#endif
			break;

		case Common::EVENT_KEYUP:
			_modifierState = event.kbd.flags;
			if (event.kbd.keycode == _currentKeyDown.keycode) {
				// Only stop firing events if it's the current key
				_currentKeyDown.keycode = 0;
			}
			break;

		case Common::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			break;

		case Common::EVENT_LBUTTONDOWN:
			_mousePos = event.mouse;
			_buttonState |= LBUTTON;
			break;

		case Common::EVENT_LBUTTONUP:
			_mousePos = event.mouse;
			_buttonState &= ~LBUTTON;
			break;

		case Common::EVENT_RBUTTONDOWN:
			_mousePos = event.mouse;
			_buttonState |= RBUTTON;
			break;

		case Common::EVENT_RBUTTONUP:
			_mousePos = event.mouse;
			_buttonState &= ~RBUTTON;
			break;

		case Common::EVENT_MAINMENU:
			if (g_engine && !g_engine->isPaused())
				g_engine->openMainMenuDialog();

			if (_shouldQuit)
				event.type = Common::EVENT_QUIT;
			else if (_shouldRTL)
				event.type = Common::EVENT_RTL;
			break;

		case Common::EVENT_RTL:
			if (ConfMan.getBool("confirm_exit")) {
				if (g_engine)
					g_engine->pauseEngine(true);
				GUI::MessageDialog alert("Do you really want to return to the Launcher?", "Launcher", "Cancel");
				result = _shouldRTL = (alert.runModal() == GUI::kMessageOK);
				if (g_engine)
					g_engine->pauseEngine(false);
			} else
				_shouldRTL = true;
			break;

		case Common::EVENT_MUTE:
			if (g_engine)
				g_engine->flipMute();
			break;

		case Common::EVENT_QUIT:
			if (ConfMan.getBool("confirm_exit")) {
				if (_confirmExitDialogActive) {
					result = false;
					break;
				}
				_confirmExitDialogActive = true;
				if (g_engine)
					g_engine->pauseEngine(true);
				GUI::MessageDialog alert("Do you really want to quit?", "Quit", "Cancel");
				result = _shouldQuit = (alert.runModal() == GUI::kMessageOK);
				if (g_engine)
					g_engine->pauseEngine(false);
				_confirmExitDialogActive = false;
			} else
				_shouldQuit = true;

			break;

		default:
			break;
		}
	} else {
		// Check if event should be sent again (keydown)
		if (_currentKeyDown.keycode != 0 && _keyRepeatTime < time) {
			// fire event
			event.type = Common::EVENT_KEYDOWN;
			event.synthetic = true;
			event.kbd.ascii = _currentKeyDown.ascii;
			event.kbd.keycode = (Common::KeyCode)_currentKeyDown.keycode;
			event.kbd.flags = _currentKeyDown.flags;
			_keyRepeatTime = time + kKeyRepeatSustainDelay;
			result = true;
		}
	}

	return result;
}

void DefaultEventManager::pushEvent(const Common::Event &event) {
	// If already received an EVENT_QUIT, don't add another one
	if (event.type == Common::EVENT_QUIT) {
		if (!_shouldQuit)
			_artificialEventSource.addEvent(event);
	} else
		_artificialEventSource.addEvent(event);
}

#endif // !defined(DISABLE_DEFAULT_EVENTMANAGER)
