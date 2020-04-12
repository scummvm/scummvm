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
#include "director/frame.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/lingo/lingo.h"

namespace Director {

bool processQuitEvent(bool click) {
	Common::Event event;

	while (g_system->getEventManager()->pollEvent(event)) {
		if (event.type == Common::EVENT_QUIT) {
			g_director->getCurrentScore()->_stopPlay = true;
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
	Common::Event event;

	uint endTime = g_system->getMillis() + 10;

	Score *sc = getCurrentScore();
	if (sc->getCurrentFrame() >= sc->_frames.size()) {
		warning("processEvents: request to access frame %d of %d", sc->getCurrentFrame(), sc->_frames.size() - 1);
		return;
	}
	Frame *currentFrame = sc->_frames[sc->getCurrentFrame()];
	uint16 spriteId = 0;

	Common::Point pos;

	while (g_system->getMillis() < endTime) {
		while (g_system->getEventManager()->pollEvent(event)) {
			if (_wm->processEvent(event))
				continue;

			switch (event.type) {
			case Common::EVENT_QUIT:
				sc->_stopPlay = true;
				break;

      case Common::EVENT_MOUSEMOVE:
        sc->_lastEventTime = g_director->getMacTicks();
        sc->_lastRollTime =  sc->_lastEventTime;
        break;

			case Common::EVENT_LBUTTONDOWN:
				pos = g_system->getEventManager()->getMousePos();

				// D3 doesn't have both mouse up and down.
				// But we still want to know if the mouse is down for press effects.
				spriteId = currentFrame->getSpriteIDFromPos(pos);
				sc->_currentMouseDownSpriteId = spriteId;

				sc->_mouseIsDown = true;
        sc->_lastEventTime = g_director->getMacTicks();
        sc->_lastClickTime = sc->_lastEventTime;

				debugC(3, kDebugEvents, "event: Button Down @(%d, %d), sprite id: %d", pos.x, pos.y, spriteId);
				_lingo->processEvent(kEventMouseDown);

				if (currentFrame->_sprites[spriteId]->_moveable) {
					warning("Moveable");
				}
				break;

			case Common::EVENT_LBUTTONUP:
				pos = g_system->getEventManager()->getMousePos();

				spriteId = currentFrame->getSpriteIDFromPos(pos);

				debugC(3, kDebugEvents, "event: Button Up @(%d, %d), sprite id: %d", pos.x, pos.y, spriteId);

				sc->_mouseIsDown = false;

				_lingo->processEvent(kEventMouseUp);
				sc->_currentMouseDownSpriteId = 0;
				break;

			case Common::EVENT_KEYDOWN:
				_keyCode = event.kbd.keycode;
				_key = (unsigned char)(event.kbd.ascii & 0xff);

				switch (_keyCode) {
				case Common::KEYCODE_LEFT:
					_keyCode = 123;
					break;
				case Common::KEYCODE_RIGHT:
					_keyCode = 124;
					break;
				case Common::KEYCODE_DOWN:
					_keyCode = 125;
					break;
				case Common::KEYCODE_UP:
					_keyCode = 126;
					break;
				default:
					debugC(1, kDebugEvents, "processEvents(): keycode: %d", _keyCode);
				}

        sc->_lastEventTime = g_director->getMacTicks();
        sc->_lastKeyTime = sc->_lastEventTime;
				_lingo->processEvent(kEventKeyDown);
				break;

			default:
				break;
			}
		}

		g_system->updateScreen();
		g_system->delayMillis(10);

		if (sc->getCurrentFrame() > 0)
			_lingo->processEvent(kEventIdle);
	}
}

void DirectorEngine::setDraggedSprite(uint16 id) {
	_draggingSprite = true;
	_draggingSpriteId = id;
	_draggingSpritePos = g_system->getEventManager()->getMousePos();

	warning("STUB: DirectorEngine::setDraggedSprite(%d)", id);
}

void DirectorEngine::waitForClick() {
	setCursor(kCursorMouseUp);

	bool cursor = false;
	uint32 nextTime = g_system->getMillis() + 1000;

	while (!processQuitEvent(true)) {
		g_system->updateScreen();
		g_system->delayMillis(10);

		if (g_system->getMillis() >= nextTime) {
			nextTime = g_system->getMillis() + 1000;

			setCursor(kCursorDefault);

			setCursor(cursor ? kCursorMouseDown : kCursorMouseUp);

			cursor = !cursor;
		}
	}

	setCursor(kCursorDefault);
}

} // End of namespace Director
