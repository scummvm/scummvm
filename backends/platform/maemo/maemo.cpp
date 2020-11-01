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

#if defined(MAEMO)

#define FORBIDDEN_SYMBOL_EXCEPTION_getenv

#include "common/scummsys.h"
#include "common/config-manager.h"

#include "backends/platform/maemo/maemo.h"
#include "backends/events/maemosdl/maemosdl-events.h"
#include "backends/graphics/maemosdl/maemosdl-graphics.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/keymapper-defaults.h"
#include "common/textconsole.h"
#include "common/translation.h"

namespace Maemo {

OSystem_SDL_Maemo::OSystem_SDL_Maemo()
	:
	_eventObserver(0),
	OSystem_POSIX() {
}

OSystem_SDL_Maemo::~OSystem_SDL_Maemo() {
	delete _eventObserver;
}

void OSystem_SDL_Maemo::init() {
	// Use an iconless window for Maemo
	// also N900 is hit by SDL_WM_SetIcon bug (window cannot receive input)
	// http://bugzilla.libsdl.org/show_bug.cgi?id=586
	initSDL();
	_window = new SdlIconlessWindow();

	OSystem_POSIX::init();
}

void OSystem_SDL_Maemo::initBackend() {
	ConfMan.registerDefault("fullscreen", true);
	ConfMan.registerDefault("aspect_ratio", true);

	// Create the events manager
	if (_eventSource == 0)
		_eventSource = new MaemoSdlEventSource();

	if (_graphicsManager == 0)
		_graphicsManager = new MaemoSdlGraphicsManager(_eventSource, _window);

	if (_eventObserver == 0)
		_eventObserver = new MaemoSdlEventObserver((MaemoSdlEventSource *)_eventSource);

	_model = detectModel();

	// Call parent implementation of this method
	OSystem_POSIX::initBackend();
	initObserver();
}

void OSystem_SDL_Maemo::quit() {
	delete this;
}

void OSystem_SDL_Maemo::fatalError() {
	delete this;
}

void OSystem_SDL_Maemo::setXWindowName(const char *caption) {
	SDL_SysWMinfo info;
	if (_window->getSDLWMInformation(&info)) {
		Display *dpy = info.info.x11.display;
		Window win;
		win = info.info.x11.fswindow;
		if (win) XStoreName(dpy, win, caption);
		win = info.info.x11.wmwindow;
		if (win) XStoreName(dpy, win, caption);
	}
}

void OSystem_SDL_Maemo::setWindowCaption(const Common::U32String &caption) {
	Common::String cap = caption.encode();
	_window->setWindowCaption(cap);

	Common::String cap2("ScummVM - "); // 2 lines in OS2008 task switcher, set first line
	cap = cap2 + cap;
	setXWindowName(cap.c_str());
}

static const Model models[] = {
	{"SU-18", kModelType770, "770", false, true},
	{"RX-34", kModelTypeN800, "N800", false, true},
	{"RX-44", kModelTypeN810, "N810", true, true},
	{"RX-48", kModelTypeN810, "N810W", true, true},
	{"RX-51", kModelTypeN900, "N900", true, false},
	{0, kModelTypeInvalid, 0, true, true}
};

const Maemo::Model OSystem_SDL_Maemo::detectModel() {
	Common::String deviceHwId = Common::String(getenv("SCUMMVM_MAEMO_DEVICE"));
	const Model *model;
	for (model = models; model->hwId; ++model) {
		if (deviceHwId.equals(model->hwId))
			return *model;
	}
	return *model;
}

static const Common::KeyTableEntry maemoKeys[] = {
	// Function keys
	{"MENU", Common::KEYCODE_F11, "Menu"},
	{"HOME", Common::KEYCODE_F12, "Home"},
	{"FULLSCREEN", Common::KEYCODE_F13, "FullScreen"},
	{"ZOOMPLUS", Common::KEYCODE_F14, "Zoom+"},
	{"ZOOMMINUS", Common::KEYCODE_F15, "Zoom-"},

	{0, Common::KEYCODE_INVALID, 0}
};

Common::HardwareInputSet *OSystem_SDL_Maemo::getHardwareInputSet() {
	Common::CompositeHardwareInputSet *inputSet = new Common::CompositeHardwareInputSet();
	inputSet->addHardwareInputSet(new Common::MouseHardwareInputSet(Common::defaultMouseButtons));
	inputSet->addHardwareInputSet(new Common::KeyboardHardwareInputSet(maemoKeys, Common::defaultModifiers));
	inputSet->addHardwareInputSet(new Common::KeyboardHardwareInputSet(Common::defaultKeys, Common::defaultModifiers));

	return inputSet;
}

Common::KeymapArray OSystem_SDL_Maemo::getGlobalKeymaps() {
	using namespace Common;
	KeymapArray globalMaps = OSystem_POSIX::getGlobalKeymaps();

	Keymap *globalMap = new Keymap(Keymap::kKeymapTypeGlobal, "maemo", "Maemo");

	Action *act;

	act = new Action("CLKM", _("Click Mode"));
	act->setCustomBackendActionEvent(Maemo::kEventClickMode);
	globalMap->addAction(act);

	act = new Action("LCLK", _("Left Click"));
	act->setLeftClickEvent();
	globalMap->addAction(act);

	act = new Action("MCLK", _("Middle Click"));
	act->setMiddleClickEvent();
	globalMap->addAction(act);

	act = new Action("RCLK", _("Right Click"));
	act->setRightClickEvent();
	globalMap->addAction(act);

	globalMaps.push_back(globalMap);

	return globalMaps;
}

Common::KeymapperDefaultBindings *OSystem_SDL_Maemo::getKeymapperDefaultBindings() {
	Common::KeymapperDefaultBindings *keymapperDefaultBindings = new Common::KeymapperDefaultBindings();

	keymapperDefaultBindings->setDefaultBinding("gui", "REMP", "HOME");
	keymapperDefaultBindings->setDefaultBinding("global", "REMP", "HOME");

	if (_model.hasMenuKey && _model.hasHwKeyboard) {
		keymapperDefaultBindings->setDefaultBinding("gui", "FULS", "FULLSCREEN");
		keymapperDefaultBindings->setDefaultBinding("global", "FULS", "FULLSCREEN");
	}

	if (_model.hasHwKeyboard) {
		keymapperDefaultBindings->setDefaultBinding("gui", "VIRT", "C+ZOOMMINUS");
		keymapperDefaultBindings->setDefaultBinding("global", "VIRT", "C+ZOOMMINUS");
	} else {
		keymapperDefaultBindings->setDefaultBinding("gui", "VIRT", "FULLSCREEN");
		keymapperDefaultBindings->setDefaultBinding("global", "VIRT", "FULLSCREEN");
	}

	if (_model.hasMenuKey )
		keymapperDefaultBindings->setDefaultBinding("global", "MENU", "MENU");
	else
		keymapperDefaultBindings->setDefaultBinding("global", "MENU", "S+C+M");

	keymapperDefaultBindings->setDefaultBinding("gui", "CLOS", "ESCAPE");

	keymapperDefaultBindings->setDefaultBinding("maemo", "RCLK", "ZOOMPLUS");
	keymapperDefaultBindings->setDefaultBinding("maemo", "CLKM", "ZOOMMINUS");

	return keymapperDefaultBindings;
}

void OSystem_SDL_Maemo::initObserver() {
	assert(_eventManager);
	_eventManager->getEventDispatcher()->registerObserver(_eventObserver, 10, false);
}

} //namespace Maemo

#endif
