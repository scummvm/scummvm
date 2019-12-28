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
#include "ultima/shared/core/ultima.h"
#include "ultima/shared/core/events.h"

namespace Ultima {
namespace Shared {

EventsManager::EventsManager() : _playTime(0), _gameCounter(0), _frameCounter(0),
	_priorFrameCounterTime(0), _lastAutosaveTime(0), _buttonsDown(0) {
}

EventsManager::~EventsManager() {
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

	// Handle auto saves
	if (!_lastAutosaveTime)
		_lastAutosaveTime = timer;
	g_ultima->autoSaveCheck(_lastAutosaveTime);

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
		case Common::EVENT_KEYDOWN:
			// Check for debugger
			if (g_ultima->_events != nullptr && event.kbd.keycode == Common::KEYCODE_d
					&& (event.kbd.flags & Common::KBD_CTRL)) {
				// Attach to the debugger
				g_ultima->_debugger->attach();
				g_ultima->_debugger->onFrame();
				return false;
			}
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

void EventsManager::nextFrame() {
	++_frameCounter;
	++_playTime;
}

void EventsManager::setButtonDown(MouseButton button, bool isDown) {
	assert(button != BUTTON_NONE);
	if (isDown)
		_buttonsDown |= BUTTON_MASK(button);
	else
		_buttonsDown &= ~BUTTON_MASK(button);
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

bool shouldQuit() {
	return g_engine->shouldQuit();
}

} // End of namespace Shared
} // End of namespace Ultima
