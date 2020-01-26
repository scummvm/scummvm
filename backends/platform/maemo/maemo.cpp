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
#ifdef ENABLE_KEYMAPPER
	delete _keymapperDefaultBindings;
#endif
}

#ifdef ENABLE_KEYMAPPER
static void registerDefaultKeyBindings(Common::KeymapperDefaultBindings *_keymapperDefaultBindings, Model _model) {
	_keymapperDefaultBindings->setDefaultBinding("gui", "REMP", "HOME");
	_keymapperDefaultBindings->setDefaultBinding("global", "REMP", "HOME");

	if (_model.hasMenuKey && _model.hasHwKeyboard) {
		_keymapperDefaultBindings->setDefaultBinding("gui", "FULS", "FULLSCREEN");
		_keymapperDefaultBindings->setDefaultBinding("global", "FULS", "FULLSCREEN");
	}

	if (_model.hasHwKeyboard) {
		_keymapperDefaultBindings->setDefaultBinding("gui", "VIRT", "C+ZOOMMINUS");
		_keymapperDefaultBindings->setDefaultBinding("global", "VIRT", "C+ZOOMMINUS");
	} else {
		_keymapperDefaultBindings->setDefaultBinding("gui", "VIRT", "FULLSCREEN");
		_keymapperDefaultBindings->setDefaultBinding("global", "VIRT", "FULLSCREEN");
	}

	if (_model.hasMenuKey )
		_keymapperDefaultBindings->setDefaultBinding("global", "MENU", "MENU");
	else
		_keymapperDefaultBindings->setDefaultBinding("global", "MENU", "S+C+M");

	_keymapperDefaultBindings->setDefaultBinding("gui", "CLOS", "ESCAPE");

	_keymapperDefaultBindings->setDefaultBinding("maemo", "RCLK", "ZOOMPLUS");
	_keymapperDefaultBindings->setDefaultBinding("maemo", "CLKM", "ZOOMMINUS");
}
#endif

void OSystem_SDL_Maemo::init() {
	// Use an iconless window for Maemo
	// also N900 is hit by SDL_WM_SetIcon bug (window cannot receive input)
	// http://bugzilla.libsdl.org/show_bug.cgi?id=586
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

#ifdef ENABLE_KEYMAPPER
	if (_keymapperDefaultBindings == 0)
		_keymapperDefaultBindings = new Common::KeymapperDefaultBindings();
#endif

	_model = detectModel();

#ifdef ENABLE_KEYMAPPER
	registerDefaultKeyBindings(_keymapperDefaultBindings, _model);
#endif

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

void OSystem_SDL_Maemo::setWindowCaption(const char *caption) {
	Common::String cap;
	byte c;

	// The string caption is supposed to be in LATIN-1 encoding.
	// SDL expects UTF-8. So we perform the conversion here.
	while ((c = *(const byte *)caption++)) {
		if (c < 0x80)
			cap += c;
		else {
			cap += 0xC0 | (c >> 6);
			cap += 0x80 | (c & 0x3F);
		}
	}

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

#ifdef ENABLE_KEYMAPPER
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
	return new Common::HardwareInputSet(true, maemoKeys);
}

Common::Keymap *OSystem_SDL_Maemo::getGlobalKeymap() {
	using namespace Common;
	Keymap *globalMap = new Keymap(Keymap::kKeymapTypeGlobal, "maemo");

	Action *act;

	act = new Action(globalMap, "CLKM", _("Click Mode"));
	Event evt = Event();
	evt.type = EVENT_CUSTOM_BACKEND_ACTION;
	evt.customType = Maemo::kEventClickMode;
	act->setEvent(evt);

	act = new Action(globalMap, "LCLK", _("Left Click"));
	act->setLeftClickEvent();

	act = new Action(globalMap, "MCLK", _("Middle Click"));
	act->setMiddleClickEvent();

	act = new Action(globalMap, "RCLK", _("Right Click"));
	act->setRightClickEvent();

	return globalMap;
}
#endif

void OSystem_SDL_Maemo::initObserver() {
	assert(_eventManager);
	_eventManager->getEventDispatcher()->registerObserver(_eventObserver, 10, false);
}

} //namespace Maemo

#endif
