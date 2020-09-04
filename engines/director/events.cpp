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

#include "common/system.h"

#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/cursor.h"
#include "director/channel.h"
#include "director/sprite.h"
#include "director/window.h"
#include "director/castmember.h"
#include "director/lingo/lingo.h"

namespace Director {

bool processQuitEvent(bool click) {
	Common::Event event;

	while (g_system->getEventManager()->pollEvent(event)) {
		if (event.type == Common::EVENT_QUIT) {
			g_director->getCurrentMovie()->getScore()->_playState = kPlayStopped;
			return true;
		}

		if (click) {
			if (event.type == Common::EVENT_LBUTTONDOWN)
				return true;
		}
	}

	return false;
}

uint32 DirectorEngine::getMacTicks() { return g_system->getMillis() * 60 / 1000.; }

void DirectorEngine::processEvents() {
	debugC(3, kDebugEvents, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
	debugC(3, kDebugEvents, "@@@@   Processing events");
	debugC(3, kDebugEvents, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	Common::Event event;

	uint endTime = g_system->getMillis() + 10;

	while (g_system->getMillis() < endTime) {
		while (g_system->getEventManager()->pollEvent(event)) {
			if (_wm->processEvent(event)) {
				// window manager has done something! update the channels
				continue;
			}

			switch (event.type) {
			case Common::EVENT_QUIT:
				_stage->getCurrentMovie()->getScore()->_playState = kPlayStopped;
				break;
			default:
				break;
			}
		}

		g_system->delayMillis(10);
	}
}

bool Window::processEvent(Common::Event &event) {
	if (MacWindow::processEvent(event))
		return true;

	if (_currentMovie && _currentMovie->processEvent(event))
		return true;

	return false;
}

bool Movie::processEvent(Common::Event &event) {
	Score *sc = getScore();
	if (sc->getCurrentFrame() >= sc->_frames.size()) {
		warning("processEvents: request to access frame %d of %d", sc->getCurrentFrame(), sc->_frames.size() - 1);
		return false;
	}
	uint16 spriteId = 0;

	Common::Point pos;

	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		pos = _window->getMousePos();

		_lastEventTime = g_director->getMacTicks();
		_lastRollTime =	 _lastEventTime;

		sc->renderCursor(pos);

		if (_currentDraggedChannel) {
			if (_currentDraggedChannel->_sprite->_moveable) {
				pos = _window->getMousePos();

				_currentDraggedChannel->addDelta(pos - _draggingSpritePos);
				_draggingSpritePos = pos;
			} else {
				_currentDraggedChannel = nullptr;
			}
		}
		return true;

	case Common::EVENT_LBUTTONDOWN:
		if (sc->_waitForClick) {
			sc->_waitForClick = false;
			_vm->setCursor(kCursorDefault);
		} else {
			pos = _window->getMousePos();

			// D3 doesn't have both mouse up and down.
			// But we still want to know if the mouse is down for press effects.
			spriteId = sc->getMouseSpriteIDFromPos(pos);
			_currentClickOnSpriteId = sc->getActiveSpriteIDFromPos(pos);

			if (spriteId > 0 && sc->_channels[spriteId]->_sprite->shouldHilite())
				g_director->getCurrentWindow()->invertChannel(sc->_channels[spriteId]);

			_lastEventTime = g_director->getMacTicks();
			_lastClickTime = _lastEventTime;
			_lastClickPos = pos;

			debugC(3, kDebugEvents, "event: Button Down @(%d, %d), movie '%s', sprite id: %d", pos.x, pos.y, _macName.c_str(), spriteId);
			registerEvent(kEventMouseDown, spriteId);

			if (sc->_channels[spriteId]->_sprite->_moveable) {
				_draggingSpritePos = _window->getMousePos();
				_currentDraggedChannel = sc->_channels[spriteId];
			}
		}

		return true;

	case Common::EVENT_LBUTTONUP:
		pos = _window->getMousePos();

		spriteId = sc->getMouseSpriteIDFromPos(pos);
		_currentClickOnSpriteId = sc->getActiveSpriteIDFromPos(pos);

		if (spriteId > 0 && sc->_channels[spriteId]->_sprite->shouldHilite())
			g_director->getCurrentWindow()->invertChannel(sc->_channels[spriteId]);

		debugC(3, kDebugEvents, "event: Button Up @(%d, %d), movie '%s', sprite id: %d", pos.x, pos.y, _macName.c_str(), spriteId);

		_currentDraggedChannel = nullptr;

		{
			CastMember *cast = getCastMember(sc->getSpriteById(spriteId)->_castId);
			if (cast && cast->_type == kCastButton)
				cast->_hilite = !cast->_hilite;
		}

		registerEvent(kEventMouseUp, spriteId);
		sc->renderCursor(pos);
		return true;

	case Common::EVENT_KEYDOWN:
		_keyCode = _vm->_macKeyCodes.contains(event.kbd.keycode) ? _vm->_macKeyCodes[event.kbd.keycode] : 0;
		_key = (unsigned char)(event.kbd.ascii & 0xff);
		_keyFlags = event.kbd.flags;

		debugC(1, kDebugEvents, "processEvents(): movie '%s', keycode: %d", _macName.c_str(), _keyCode);

		_lastEventTime = g_director->getMacTicks();
		_lastKeyTime = _lastEventTime;
		registerEvent(kEventKeyDown);
		return true;

	case Common::EVENT_KEYUP:
		_keyFlags = event.kbd.flags;
		return true;

	default:
		break;
	}

	return false;
}

} // End of namespace Director
