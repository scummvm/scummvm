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

#include "common/scummsys.h"

#if defined(SDL_BACKEND)

#include "backends/events/sdl/sdl-events.h"
#include "backends/platform/sdl/sdl.h"
#include "backends/graphics/graphics.h"
#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/fs.h"
#include "engines/engine.h"
#include "gui/gui-manager.h"

SdlEventSource::~SdlEventSource() {
	closeJoystick();
}

bool SdlEventSource::processMouseEvent(Common::Event &event, int x, int y, int relx, int rely) {
	_mouseX = x;
	_mouseY = y;

	event.mouse.x = x;
	event.mouse.y = y;
	event.relMouse.x = relx;
	event.relMouse.y = rely;

	if (_graphicsManager) {
		return _graphicsManager->notifyMousePosition(event.mouse);
	}

	return true;
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
Common::Point SdlEventSource::getTouchscreenSize() {
	int windowWidth, windowHeight;
	SDL_GetWindowSize((dynamic_cast<SdlGraphicsManager*>(_graphicsManager))->getWindow()->getSDLWindow(), &windowWidth, &windowHeight);
	return Common::Point(windowWidth, windowHeight);
}

bool SdlEventSource::isTouchPortTouchpadMode(SDL_TouchID port) {
       return g_system->getFeatureState(OSystem::kFeatureTouchpadMode);
}

bool SdlEventSource::isTouchPortActive(SDL_TouchID port) {
	return true;
}

void SdlEventSource::convertTouchXYToGameXY(float touchX, float touchY, int *gameX, int *gameY) {
	int windowWidth, windowHeight;
	SDL_GetWindowSize((dynamic_cast<SdlGraphicsManager*>(_graphicsManager))->getWindow()->getSDLWindow(), &windowWidth, &windowHeight);

	*gameX = windowWidth * touchX;
	*gameY = windowHeight * touchY;
}
#endif

bool SdlEventSource::handleMouseMotion(SDL_Event &ev, Common::Event &event) {
	event.type = Common::EVENT_MOUSEMOVE;

	return processMouseEvent(event, ev.motion.x, ev.motion.y, ev.motion.xrel, ev.motion.yrel);
}

bool SdlEventSource::handleMouseButtonDown(SDL_Event &ev, Common::Event &event) {
	if (ev.button.button == SDL_BUTTON_LEFT)
		event.type = Common::EVENT_LBUTTONDOWN;
	else if (ev.button.button == SDL_BUTTON_RIGHT)
		event.type = Common::EVENT_RBUTTONDOWN;
#if defined(SDL_BUTTON_WHEELUP) && defined(SDL_BUTTON_WHEELDOWN)
	else if (ev.button.button == SDL_BUTTON_WHEELUP)
		event.type = Common::EVENT_WHEELUP;
	else if (ev.button.button == SDL_BUTTON_WHEELDOWN)
		event.type = Common::EVENT_WHEELDOWN;
#endif
#if defined(SDL_BUTTON_MIDDLE)
	else if (ev.button.button == SDL_BUTTON_MIDDLE)
		event.type = Common::EVENT_MBUTTONDOWN;
#endif
#if defined(SDL_BUTTON_X1)
	else if (ev.button.button == SDL_BUTTON_X1)
		event.type = Common::EVENT_X1BUTTONDOWN;
#endif
#if defined(SDL_BUTTON_X2)
	else if (ev.button.button == SDL_BUTTON_X2)
		event.type = Common::EVENT_X2BUTTONDOWN;
#endif
	else
		return false;

	return processMouseEvent(event, ev.button.x, ev.button.y);
}

bool SdlEventSource::handleMouseButtonUp(SDL_Event &ev, Common::Event &event) {
	if (ev.button.button == SDL_BUTTON_LEFT)
		event.type = Common::EVENT_LBUTTONUP;
	else if (ev.button.button == SDL_BUTTON_RIGHT)
		event.type = Common::EVENT_RBUTTONUP;
#if defined(SDL_BUTTON_MIDDLE)
	else if (ev.button.button == SDL_BUTTON_MIDDLE)
		event.type = Common::EVENT_MBUTTONUP;
#endif
#if defined(SDL_BUTTON_X1)
	else if (ev.button.button == SDL_BUTTON_X1)
		event.type = Common::EVENT_X1BUTTONUP;
#endif
#if defined(SDL_BUTTON_X2)
	else if (ev.button.button == SDL_BUTTON_X2)
		event.type = Common::EVENT_X2BUTTONUP;
#endif
	else
		return false;

	return processMouseEvent(event, ev.button.x, ev.button.y);
}

bool SdlEventSource::handleSysWMEvent(SDL_Event &ev, Common::Event &event) {
	return false;
}

int SdlEventSource::mapSDLJoystickButtonToOSystem(Uint8 sdlButton) {
	const Common::JoystickButton osystemButtons[] = {
	    Common::JOYSTICK_BUTTON_A,
	    Common::JOYSTICK_BUTTON_B,
	    Common::JOYSTICK_BUTTON_X,
	    Common::JOYSTICK_BUTTON_Y,
	    Common::JOYSTICK_BUTTON_LEFT_SHOULDER,
	    Common::JOYSTICK_BUTTON_RIGHT_SHOULDER,
	    Common::JOYSTICK_BUTTON_BACK,
	    Common::JOYSTICK_BUTTON_START,
	    Common::JOYSTICK_BUTTON_LEFT_STICK,
	    Common::JOYSTICK_BUTTON_RIGHT_STICK
	};

	if (sdlButton >= ARRAYSIZE(osystemButtons)) {
		return -1;
	}

	return osystemButtons[sdlButton];
}

bool SdlEventSource::handleJoyButtonDown(SDL_Event &ev, Common::Event &event) {
	int button = mapSDLJoystickButtonToOSystem(ev.jbutton.button);
	if (button < 0) {
		return false;
	}

	event.type = Common::EVENT_JOYBUTTON_DOWN;
	event.joystick.button = button;

	return true;
}

bool SdlEventSource::handleJoyButtonUp(SDL_Event &ev, Common::Event &event) {
	int button = mapSDLJoystickButtonToOSystem(ev.jbutton.button);
	if (button < 0) {
		return false;
	}

	event.type = Common::EVENT_JOYBUTTON_UP;
	event.joystick.button = button;

	return true;
}

bool SdlEventSource::handleJoyAxisMotion(SDL_Event &ev, Common::Event &event) {
	event.type = Common::EVENT_JOYAXIS_MOTION;
	event.joystick.axis = ev.jaxis.axis;
	event.joystick.position = ev.jaxis.value;

	return true;
}

#define HANDLE_HAT_UP(new, old, mask, joybutton) \
	if ((old & mask) && !(new & mask)) { \
		event.joystick.button = joybutton; \
		g_system->getEventManager()->pushEvent(event); \
	}

#define HANDLE_HAT_DOWN(new, old, mask, joybutton) \
	if ((new & mask) && !(old & mask)) { \
		event.joystick.button = joybutton; \
		g_system->getEventManager()->pushEvent(event); \
	}

bool SdlEventSource::handleJoyHatMotion(SDL_Event &ev, Common::Event &event) {
	event.type = Common::EVENT_JOYBUTTON_UP;
	HANDLE_HAT_UP(ev.jhat.value, _lastHatPosition, SDL_HAT_UP, Common::JOYSTICK_BUTTON_DPAD_UP)
	HANDLE_HAT_UP(ev.jhat.value, _lastHatPosition, SDL_HAT_DOWN, Common::JOYSTICK_BUTTON_DPAD_DOWN)
	HANDLE_HAT_UP(ev.jhat.value, _lastHatPosition, SDL_HAT_LEFT, Common::JOYSTICK_BUTTON_DPAD_LEFT)
	HANDLE_HAT_UP(ev.jhat.value, _lastHatPosition, SDL_HAT_RIGHT, Common::JOYSTICK_BUTTON_DPAD_RIGHT)

	event.type = Common::EVENT_JOYBUTTON_DOWN;
	HANDLE_HAT_DOWN(ev.jhat.value, _lastHatPosition, SDL_HAT_UP, Common::JOYSTICK_BUTTON_DPAD_UP)
	HANDLE_HAT_DOWN(ev.jhat.value, _lastHatPosition, SDL_HAT_DOWN, Common::JOYSTICK_BUTTON_DPAD_DOWN)
	HANDLE_HAT_DOWN(ev.jhat.value, _lastHatPosition, SDL_HAT_LEFT, Common::JOYSTICK_BUTTON_DPAD_LEFT)
	HANDLE_HAT_DOWN(ev.jhat.value, _lastHatPosition, SDL_HAT_RIGHT, Common::JOYSTICK_BUTTON_DPAD_RIGHT)

	_lastHatPosition = ev.jhat.value;

	return false;
}

bool SdlEventSource::remapKey(SDL_Event &ev, Common::Event &event) {
	return false;
}

void SdlEventSource::fakeWarpMouse(const int x, const int y) {
	_queuedFakeMouseMove = true;
	_fakeMouseMove.type = Common::EVENT_MOUSEMOVE;
	_fakeMouseMove.mouse = Common::Point(x, y);
}

void SdlEventSource::setEngineRunning(const bool value) {
	_engineRunning = value;
}

bool SdlEventSource::handleResizeEvent(Common::Event &event, int w, int h) {
	if (_graphicsManager) {
		_graphicsManager->notifyResize(w, h);

		// If the screen changed, send an Common::EVENT_SCREEN_CHANGED
		int screenID = g_system->getScreenChangeID();
		if (screenID != _lastScreenID) {
			_lastScreenID = screenID;
			event.type = Common::EVENT_SCREEN_CHANGED;
			return true;
		}
	}

	return false;
}

#endif
