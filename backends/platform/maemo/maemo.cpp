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
 */

#if defined(MAEMO)

#define FORBIDDEN_SYMBOL_EXCEPTION_getenv

#include "common/scummsys.h"
#include "common/config-manager.h"

#include "backends/platform/maemo/maemo.h"
#include "backends/platform/maemo/maemo-keys.h"
#include "backends/events/maemosdl/maemosdl-events.h"
#include "backends/graphics/maemosdl/maemosdl-graphics.h"
#include "backends/keymapper/keymapper.h"
#include "common/textconsole.h"
#include "common/translation.h"


#include <SDL/SDL_syswm.h>
#include <X11/Xutil.h>

namespace Maemo {

OSystem_SDL_Maemo::OSystem_SDL_Maemo()
	:
	OSystem_POSIX() {
}

OSystem_SDL_Maemo::~OSystem_SDL_Maemo() {
	delete _eventObserver;
}

void OSystem_SDL_Maemo::initBackend() {
	// Create the events manager
	if (_eventSource == 0)
		_eventSource = new MaemoSdlEventSource();

	if (_graphicsManager == 0)
		_graphicsManager = new MaemoSdlGraphicsManager(_eventSource);

	if (_eventObserver == 0)
		_eventObserver = new MaemoSdlEventObserver((MaemoSdlEventSource *)_eventSource);

	ConfMan.set("vkeybdpath", DATA_PATH);

	_model = Model(detectModel());

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
	SDL_VERSION(&info.version);
	if (SDL_GetWMInfo(&info)) {
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

	SDL_WM_SetCaption(cap.c_str(), cap.c_str());

	Common::String cap2("ScummVM - "); // 2 lines in OS2008 task switcher, set first line
	cap = cap2 + cap;
	setXWindowName(cap.c_str());
}

const Maemo::Model OSystem_SDL_Maemo::detectModel() {
	Common::String deviceHwId = Common::String(getenv("SCUMMVM_MAEMO_DEVICE"));
	const Model *model;
	for (model = models; model->hwId; model++) {
		if (deviceHwId.equals(model->hwId))
			return *model;
	}
	return *model;
}

void OSystem_SDL_Maemo::setupIcon() {
	// no Maemo version needs setupIcon
	// also N900 is hit by SDL_WM_SetIcon bug (window cannot receive input)
	// http://bugzilla.libsdl.org/show_bug.cgi?id=586
}

Common::HardwareKeySet *OSystem_SDL_Maemo::getHardwareKeySet() {
#ifdef ENABLE_KEYMAPPER
	return new Common::HardwareKeySet(Common::maemoKeys, Common::maemoModifiers);
#else
	return OSystem_POSIX::getHardwareKeySet();
#endif
}

Common::Keymap *OSystem_SDL_Maemo::getGlobalKeymap() {
#ifdef ENABLE_KEYMAPPER
	using namespace Common;
	Keymap *globalMap = new Keymap("maemo");

	Action *act;

	act = new Action(globalMap, "CLKM", _("Click Mode"), kKeyRemapActionType);
	Event evt = Event();
	evt.type = EVENT_CUSTOM_BACKEND;
	evt.customType = Maemo::kEventClickMode;
	act->addEvent(evt);

	act = new Action(globalMap, "LCLK", _("Left Click"), kKeyRemapActionType);
	act->addLeftClickEvent();

	act = new Action(globalMap, "MCLK", _("Middle Click"), kKeyRemapActionType);
	act->addMiddleClickEvent();

	act = new Action(globalMap, "RCLK", _("Right Click"), kKeyRemapActionType);
	act->addRightClickEvent();

	return globalMap;
#else
	return OSystem_POSIX::getGlobalKeymap();
#endif
}

void OSystem_SDL_Maemo::initObserver() {
	assert(_eventManager);
	_eventManager->getEventDispatcher()->registerObserver(_eventObserver, 10, false);
}

} //namespace Maemo

#endif
