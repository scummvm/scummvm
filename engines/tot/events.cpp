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

#include "tot/events.h"
#include "events.h"
#include "tot/util.h"

namespace Tot {

TotEventManager::TotEventManager() {
}

void TotEventManager::pollEvent(bool allowDrag) {

	Common::EventManager *eventMan = g_engine->_system->getEventManager();

	zeroEvents(allowDrag);
	while (eventMan->pollEvent(_event)) {
		if (isMouseEvent(_event)) {
			g_engine->_mouse->warpMouse(_event.mouse);
			g_engine->_mouse->mouseX = _event.mouse.x;
			g_engine->_mouse->mouseY = _event.mouse.y;
		}
		switch (_event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return;

		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			// handle action
			handleKey(_event);
			break;

		case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
			break;
		case Common::EVENT_KEYDOWN:
			changeGameSpeed(_event);
			_keyPressed = true;
			_lastKeyEvent = _event;
			return;
		case Common::EVENT_KEYUP:
			return;
		case Common::EVENT_MOUSEMOVE:
			_mouseX = _event.mouse.x;
			_mouseY = _event.mouse.y;
			break;
		case Common::EVENT_LBUTTONDOWN:
			_leftMouseButton = 1;
			g_engine->_mouse->mouseClickX = _event.mouse.x;
			g_engine->_mouse->mouseClickY = _event.mouse.y;
			break;
		case Common::EVENT_LBUTTONUP:
			_leftMouseButton = 0;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_rightMouseButton = 0;
			g_engine->_mouse->mouseClickX = _event.mouse.x;
			g_engine->_mouse->mouseClickY = _event.mouse.y;
			break;
		case Common::EVENT_RBUTTONUP:
			_rightMouseButton = 1;
			break;
		default:
			break;
		}
	}
}

void TotEventManager::zeroEvents(bool allowDrag) {
	if (!allowDrag) {
		_leftMouseButton = 0;
		_rightMouseButton = 0;
	}
	_escKeyFl = false;
	_gameKey = KEY_NONE;
	_keyPressed = 0;
	_lastKeyEvent = Common::Event();
}

void TotEventManager::waitForPress() {
	bool waitForKey = false;
	while (!waitForKey && !g_engine->shouldQuit()) {
		g_engine->_events->pollEvent();
		if (g_engine->_events->_keyPressed) {
			waitForKey = true;
		}

		g_engine->_screen->update();
		g_system->delayMillis(10);
	}
}

void TotEventManager::handleKey(const Common::Event &event) {
	if (event.customType == kActionVolume)
		_gameKey = KEY_VOLUME;
	else if (event.customType == kActionSaveLoad)
		_gameKey = KEY_SAVELOAD;
	else if (event.customType == kActionTalk)
		_gameKey = KEY_TALK;
	else if (event.customType == kActionPickup)
		_gameKey = KEY_PICKUP;
	else if (event.customType == kActionLookAt)
		_gameKey = KEY_LOOKAT;
	else if (event.customType == kActionUse)
		_gameKey = KEY_USE;
	else if (event.customType == kActionOpen)
		_gameKey = KEY_OPEN;
	else if (event.customType == kActionClose)
		_gameKey = KEY_CLOSE;
	else if (event.customType == kActionYes)
		_gameKey = KEY_YES;
	else if (event.customType == kActionNo)
		_gameKey = KEY_NO;
	else if (event.customType == kActionEscape) {
		_gameKey = KEY_ESCAPE;
		_escKeyFl = true;
	}
}

} // End of namespace Tot
