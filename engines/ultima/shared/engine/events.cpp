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
#include "graphics/cursorman.h"
#include "common/events.h"
#include "common/endian.h"
#include "common/system.h"
#include "engines/util.h"
#include "ultima/shared/engine/ultima.h"
#include "ultima/shared/engine/events.h"

namespace Ultima {
namespace Shared {

EventsManager::EventsManager(EventsCallback *callback) : _callback(callback), _playTime(0),
		_gameCounter(0), _frameCounter(0), _priorFrameCounterTime(0), _buttonsDown(0),
		_specialButtons(0), _priorFrameTime(0) {
}

void EventsManager::showCursor() {
	CursorMan.showMouse(true);
}

void EventsManager::hideCursor() {
	CursorMan.showMouse(false);
}

bool EventsManager::isCursorVisible() {
	return CursorMan.isVisible();
}

bool EventsManager::pollEvent(Common::Event &event) {
	uint32 timer = g_system->getMillis();

	if (timer >= (_priorFrameCounterTime + GAME_FRAME_TIME)) {
		// Time to build up next game frame
		_priorFrameCounterTime = timer;
		nextFrame();
	}

	// Event handling
	if (g_system->getEventManager()->pollEvent(event)) {
		if (isMouseDownEvent(event.type)) {
			setButtonDown(whichButton(event.type), true);
			_mousePos = event.mouse;
		} else if (isMouseUpEvent(event.type)) {
			setButtonDown(whichButton(event.type), false);
			_mousePos = event.mouse;
		} else if (event.type == Common::EVENT_MOUSEMOVE) {
			_mousePos = event.mouse;
		}

		switch (event.type) {
		case Common::EVENT_KEYDOWN: {
			handleKbdSpecial(event.kbd);
			break;
		}
		case Common::EVENT_KEYUP:
			handleKbdSpecial(event.kbd);
			break;
		case Common::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			break;
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			break;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			break;
		default:
 			break;
		}

		return true;
	}

	return false;
}

void EventsManager::pollEvents() {
	Common::Event event;

	while (pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			eventTarget()->mouseMove(_mousePos);
			break;
		case Common::EVENT_LBUTTONDOWN:
			eventTarget()->leftButtonDown(_mousePos);
			break;
		case Common::EVENT_LBUTTONUP:
			eventTarget()->leftButtonUp(_mousePos);
			break;
		case Common::EVENT_MBUTTONDOWN:
			eventTarget()->middleButtonDown(_mousePos);
			break;
		case Common::EVENT_MBUTTONUP:
			eventTarget()->middleButtonUp(_mousePos);
			break;
		case Common::EVENT_RBUTTONDOWN:
			eventTarget()->rightButtonDown(_mousePos);
			break;
		case Common::EVENT_RBUTTONUP:
			eventTarget()->rightButtonUp(_mousePos);
			break;
		case Common::EVENT_WHEELUP:
		case Common::EVENT_WHEELDOWN:
			eventTarget()->mouseWheel(_mousePos, event.type == Common::EVENT_WHEELUP);
			break;
		case Common::EVENT_KEYDOWN:
			eventTarget()->keyDown(event.kbd);
			break;
		case Common::EVENT_KEYUP:
			eventTarget()->keyUp(event.kbd);
			break;
		default:
			break;
		}
	}
}

void EventsManager::pollEventsAndWait() {
	pollEvents();
	g_system->delayMillis(10);
}

void EventsManager::nextFrame() {
	++_frameCounter;
	++_playTime;

	// Handle any idle updates
	if (!_eventTargets.empty())
		eventTarget()->onIdle();

	// Render anything pending for the screen
	Graphics::Screen *screen = _callback->getScreen();
	if (screen)
		screen->update();
}

void EventsManager::setButtonDown(MouseButton button, bool isDown) {
	assert(button != BUTTON_NONE);

	byte mask = 0;
	switch (button) {
	case BUTTON_LEFT:
		mask = MK_LBUTTON;
		break;
	case BUTTON_RIGHT:
		mask = MK_RBUTTON;
		break;
	case BUTTON_MIDDLE:
		mask = MK_MBUTTON;
		break;
	default:
		break;	
	}

	if (isDown) {
		_buttonsDown |= BUTTON_MASK(button);
		_specialButtons |= mask;
	} else {
		_buttonsDown &= ~BUTTON_MASK(button);
		_specialButtons &= ~mask;
	}
}

uint32 EventsManager::getTicksCount() const {
	return _frameCounter * GAME_FRAME_TIME;
}

void EventsManager::sleep(uint time) {
	uint32 delayEnd = g_system->getMillis() + time;

	while (!g_engine->shouldQuit() && g_system->getMillis() < delayEnd)
		pollEventsAndWait();
}

bool EventsManager::waitForPress(uint expiry) {
	uint32 delayEnd = g_system->getMillis() + expiry;
	CPressTarget pressTarget;
	addTarget(&pressTarget);

	while (!g_engine->shouldQuit() && g_system->getMillis() < delayEnd && !pressTarget._pressed) {
		pollEventsAndWait();
	}

	removeTarget();
	return pressTarget._pressed;
}

void EventsManager::setMousePos(const Point &pt) {
	g_system->warpMouse(pt.x, pt.y);
	_mousePos = pt;
	eventTarget()->mouseMove(_mousePos);
}

void EventsManager::handleKbdSpecial(Common::KeyState keyState) {
	if (keyState.flags & Common::KBD_CTRL)
		_specialButtons |= MK_CONTROL;
	else
		_specialButtons &= ~MK_CONTROL;

	if (keyState.flags & Common::KBD_SHIFT)
		_specialButtons |= MK_SHIFT;
	else
		_specialButtons &= ~MK_SHIFT;
}


bool shouldQuit() {
	return g_engine->shouldQuit();
}

bool isMouseDownEvent(Common::EventType type) {
	return type == Common::EVENT_LBUTTONDOWN || type == Common::EVENT_RBUTTONDOWN
		|| type == Common::EVENT_MBUTTONDOWN;
}

bool isMouseUpEvent(Common::EventType type) {
	return type == Common::EVENT_LBUTTONUP || type == Common::EVENT_RBUTTONUP
		|| type == Common::EVENT_MBUTTONUP;
}

MouseButton whichButton(Common::EventType type) {
	if (type == Common::EVENT_LBUTTONDOWN || type == Common::EVENT_LBUTTONUP)
		return BUTTON_LEFT;
	else if (type == Common::EVENT_RBUTTONDOWN || type == Common::EVENT_RBUTTONUP)
		return BUTTON_RIGHT;
	else if (type == Common::EVENT_MBUTTONDOWN || type == Common::EVENT_MBUTTONUP)
		return BUTTON_MIDDLE;
	else
		return BUTTON_NONE;
}

} // End of namespace Shared
} // End of namespace Ultima
