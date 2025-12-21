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
#include "common/events.h"

#include "pelrock/events.h"
#include "pelrock/pelrock.h"

namespace Pelrock {

PelrockEventManager::PelrockEventManager() {
}

void PelrockEventManager::pollEvent() {

	Common::EventManager *eventMan = g_engine->_system->getEventManager();
	while (eventMan->pollEvent(_event)) {
		if (isMouseEvent(_event)) {
			_mouseX = _event.mouse.x;
			_mouseY = _event.mouse.y;
		}
		switch (_event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return;

		// case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
		// 	// handle action
		// 	handleKey(_event);
		// 	break;

		// case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
		// 	break;
		// case Common::EVENT_KEYDOWN:
		// 	changeGameSpeed(_event);
		// 	_keyPressed = true;
		// 	_lastKeyEvent = _event;
		// 	return;
		// case Common::EVENT_KEYUP:
		// 	return;
		case Common::EVENT_LBUTTONDOWN:
			if (_leftMouseButton == 0) {
				_clickTime = g_system->getMillis();
			}
			_leftMouseButton = 1;
			break;
		case Common::EVENT_LBUTTONUP:
			if (_leftMouseButton == 1) {
				// Don't treat as regular click if we're in popup selection mode
				if (!_popupSelectionMode) {
					_leftMouseClicked = true;
				}
				_mouseClickX = _event.mouse.x;
				_mouseClickY = _event.mouse.y;
				_longClicked = false;
			} else {
				_leftMouseClicked = false;
			}
			_longClicked = false;
			_leftMouseButton = 0;
			_clickTime = 0;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_rightMouseButton = 1;
			debug("Right mouse button down");
			break;
		case Common::EVENT_RBUTTONUP:
			if (_rightMouseButton == 1) {
				debug("Right mouse clicked");
				_rightMouseClicked = true;
			} else {
				_rightMouseClicked = false;
			}
			_rightMouseButton = 0;
			break;
		default:
			break;
		}
	}

	if (_leftMouseButton) {
		uint32 elapsedLongClick = g_system->getMillis() - _clickTime;
		if (elapsedLongClick >= kDoubleClickDelay) {
			elapsedLongClick = 0;
			_longClicked = true;
			_popupSelectionMode = true;
		}
	}
}

void PelrockEventManager::waitForKey() {
	bool waitForKey = false;
	Common::Event e;
	debug("Waiting for key!");
	while (!waitForKey && !g_engine->shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_KEYDOWN) {
				waitForKey = true;
			}
		}

		g_engine->_screen->update();
		g_system->delayMillis(10);
	}
}
} // namespace Pelrock
