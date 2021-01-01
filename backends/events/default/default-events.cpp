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

#include "common/scummsys.h"

#if !defined(DISABLE_DEFAULT_EVENTMANAGER)

#include "common/system.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "backends/events/default/default-events.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/virtual-mouse.h"
#include "backends/vkeybd/virtual-keyboard.h"

#include "engines/engine.h"
#include "gui/debugger.h"
#include "gui/message.h"

DefaultEventManager::DefaultEventManager(Common::EventSource *boss) :
	_buttonState(0),
	_modifierState(0),
	_shouldQuit(false),
	_shouldReturnToLauncher(false),
	_confirmExitDialogActive(false) {

	assert(boss);

	_dispatcher.registerSource(boss, false);
	_dispatcher.registerSource(&_artificialEventSource, false);

	_dispatcher.registerObserver(this, kEventManPriority, false);

#ifdef ENABLE_VKEYBD
	_vk = nullptr;
#endif

	_virtualMouse = new Common::VirtualMouse(&_dispatcher);

	_keymapper = new Common::Keymapper(this);
	_dispatcher.registerMapper(_keymapper);
}

DefaultEventManager::~DefaultEventManager() {
	delete _virtualMouse;
#ifdef ENABLE_VKEYBD
	delete _vk;
#endif
	delete _keymapper;
}

void DefaultEventManager::init() {
#ifdef ENABLE_VKEYBD
	_vk = new Common::VirtualKeyboard();

	if (ConfMan.hasKey("vkeybd_pack_name")) {
		_vk->loadKeyboardPack(ConfMan.get("vkeybd_pack_name"));
	} else {
		_vk->loadKeyboardPack("vkeybd_default");
	}
#endif
}

bool DefaultEventManager::pollEvent(Common::Event &event) {
	_dispatcher.dispatch();

	if (g_engine)
		// Handle autosaves if enabled
		g_engine->handleAutoSave();

	if (_eventQueue.empty()) {
		return false;
	}

	event = _eventQueue.pop();
	bool forwardEvent = true;

	// If the backend has the kFeatureNoQuit or the "Return to Launcher at Exit" option is enabled,
	// replace "Quit" event with "Return to Launcher". This is also handled in scummvm_main, but
	// doing it here allows getting the correct confirmation dialog if the "confirm_exit" setting
	// is set to true.
	if (event.type == Common::EVENT_QUIT && (g_system->hasFeature(OSystem::kFeatureNoQuit) || (ConfMan.getBool("gui_return_to_launcher_at_exit") && g_engine)))
		event.type = Common::EVENT_RETURN_TO_LAUNCHER;

	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		_modifierState = event.kbd.flags;

		if (event.kbd.keycode == Common::KEYCODE_BACKSPACE) {
			// WORKAROUND: Some engines incorrectly attempt to use the
			// ascii value instead of the keycode to detect the backspace
			// key (a non-portable behavior). This fails at least on
			// Mac OS X, possibly also on other systems.
			// As a workaround, we force the ascii value for backspace
			// key pressed. A better fix would be for engines to stop
			// making invalid assumptions about ascii values.
			event.kbd.ascii = Common::KEYCODE_BACKSPACE;
		}
		break;

	case Common::EVENT_KEYUP:
		_modifierState = event.kbd.flags;
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
		else if (_shouldReturnToLauncher)
			event.type = Common::EVENT_RETURN_TO_LAUNCHER;
		break;
#ifdef ENABLE_VKEYBD
	case Common::EVENT_VIRTUAL_KEYBOARD:
		if (!_vk)
			break;

		if (_vk->isDisplaying()) {
			_vk->close(true);
		} else {
			PauseToken pt;
			if (g_engine)
				pt = g_engine->pauseEngine();
			_vk->show();
			forwardEvent = false;
		}
		break;
#endif
	case Common::EVENT_RETURN_TO_LAUNCHER:
		if (ConfMan.getBool("confirm_exit")) {
			if (_confirmExitDialogActive) {
				forwardEvent = false;
				break;
			}
			_confirmExitDialogActive = true;
			{
				PauseToken pt;
				if (g_engine)
					pt = g_engine->pauseEngine();
				GUI::MessageDialog alert(_("Do you really want to return to the Launcher?\nAny unsaved progress will be lost."), _("Yes"), _("Cancel"));
				forwardEvent = _shouldReturnToLauncher = (alert.runModal() == GUI::kMessageOK);
			}
			_confirmExitDialogActive = false;
		} else
			_shouldReturnToLauncher = true;
		break;

	case Common::EVENT_MUTE:
		if (g_engine)
			g_engine->flipMute();
		break;

	case Common::EVENT_QUIT:
		if (g_engine && ConfMan.getBool("confirm_exit")) {
			if (_confirmExitDialogActive) {
				forwardEvent = false;
				break;
			}
			_confirmExitDialogActive = true;

			{
				PauseToken pt;
				pt = g_engine->pauseEngine();
				GUI::MessageDialog alert(_("Do you really want to quit?\nAny unsaved progress will be lost."), _("Quit"), _("Cancel"));
				forwardEvent = _shouldQuit = (alert.runModal() == GUI::kMessageOK);
			}
			_confirmExitDialogActive = false;
		} else {
			_shouldQuit = true;
		}
		break;

	case Common::EVENT_DEBUGGER: {
		GUI::Debugger *debugger = g_engine ? g_engine->getOrCreateDebugger() : nullptr;
		if (debugger && !debugger->isActive()) {
			debugger->attach();
			debugger->onFrame();
			forwardEvent = false;
		}
		break;
	}

	case Common::EVENT_INPUT_CHANGED: {
		Common::HardwareInputSet *inputSet = g_system->getHardwareInputSet();
		Common::KeymapperDefaultBindings *backendDefaultBindings = g_system->getKeymapperDefaultBindings();

		_keymapper->registerHardwareInputSet(inputSet, backendDefaultBindings);
		break;
	}

	default:
		break;
	}

	return forwardEvent;
}

void DefaultEventManager::pushEvent(const Common::Event &event) {
	// If already received an EVENT_QUIT, don't add another one
	if (event.type == Common::EVENT_QUIT) {
		if (!_shouldQuit)
			_artificialEventSource.addEvent(event);
	} else
		_artificialEventSource.addEvent(event);
}

void DefaultEventManager::purgeMouseEvents() {
	_dispatcher.dispatch();

	Common::Queue<Common::Event> filteredQueue;
	while (!_eventQueue.empty()) {
		Common::Event event = _eventQueue.pop();
		switch (event.type) {
		// Update button state even when purging events to avoid desynchronisation with real button state
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

		case Common::EVENT_WHEELUP:
		case Common::EVENT_WHEELDOWN:
		case Common::EVENT_MBUTTONDOWN:
		case Common::EVENT_MBUTTONUP:
		case Common::EVENT_X1BUTTONDOWN:
		case Common::EVENT_X1BUTTONUP:
		case Common::EVENT_X2BUTTONDOWN:
		case Common::EVENT_X2BUTTONUP:
		case Common::EVENT_MOUSEMOVE:
			// do nothing
			break;
		default:
			filteredQueue.push(event);
			break;
		}
	}
	_eventQueue = filteredQueue;
}

Common::Keymap *DefaultEventManager::getGlobalKeymap() {
	using namespace Common;

	// Now create the global keymap
	Keymap *globalKeymap = new Keymap(Keymap::kKeymapTypeGlobal, kGlobalKeymapName, _("Global"));

	Action *act;
	act = new Action("MENU", _("Global Main Menu"));
	act->addDefaultInputMapping("C+F5");
	act->addDefaultInputMapping("JOY_START");
	act->setEvent(EVENT_MAINMENU);
	globalKeymap->addAction(act);

#ifdef ENABLE_VKEYBD
	act = new Action("VIRT", _("Display keyboard"));
	act->addDefaultInputMapping("C+F7");
	act->addDefaultInputMapping("JOY_BACK");
	act->setEvent(EVENT_VIRTUAL_KEYBOARD);
	globalKeymap->addAction(act);
#endif

	act = new Action("MUTE", _("Toggle mute"));
	act->addDefaultInputMapping("C+u");
	act->setEvent(EVENT_MUTE);
	globalKeymap->addAction(act);

	if (!g_system->hasFeature(OSystem::kFeatureNoQuit)) {
		act = new Action("QUIT", _("Quit"));
		act->setEvent(EVENT_QUIT);

#if defined(MACOSX)
		// On Macintosh, Cmd-Q quits
		act->addDefaultInputMapping("M+q");
#elif defined(POSIX)
		// On other *nix systems, Control-Q quits
		act->addDefaultInputMapping("C+q");
#else
		// Ctrl-z quits
		act->addDefaultInputMapping("C+z");

#ifdef WIN32
		// On Windows, also use the default Alt-F4 quit combination
		act->addDefaultInputMapping("A+F4");
#endif
#endif

		globalKeymap->addAction(act);
	}

	act = new Action("DEBUGGER", _("Open Debugger"));
	act->addDefaultInputMapping("C+A+d");
	act->setEvent(EVENT_DEBUGGER);
	globalKeymap->addAction(act);

	_virtualMouse->addActionsToKeymap(globalKeymap);

	return globalKeymap;
}

#endif // !defined(DISABLE_DEFAULT_EVENTMANAGER)
