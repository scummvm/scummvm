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
#include "director/sprite.h"
#include "director/stage.h"
#include "director/castmember.h"
#include "director/lingo/lingo.h"

namespace Director {

bool processQuitEvent(bool click) {
	Common::Event event;

	while (g_system->getEventManager()->pollEvent(event)) {
		if (event.type == Common::EVENT_QUIT) {
			g_director->getCurrentMovie()->getScore()->_stopPlay = true;
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

void DirectorEngine::processEvents(bool bufferLingoEvents) {
	Common::Event event;

	uint endTime = g_system->getMillis() + 10;

	Movie *m = getCurrentMovie();
	Score *sc = m->getScore();
	if (sc->getCurrentFrame() >= sc->_frames.size()) {
		warning("processEvents: request to access frame %d of %d", sc->getCurrentFrame(), sc->_frames.size() - 1);
		return;
	}
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
				m->_lastEventTime = g_director->getMacTicks();
				m->_lastRollTime =	 m->_lastEventTime;

				if (_draggingSprite) {
					Sprite *draggedSprite = sc->getSpriteById(_draggingSpriteId);
					if (draggedSprite->_moveable) {
						pos = getStage()->getMousePos();

						sc->_channels[_draggingSpriteId]->addDelta(pos - _draggingSpritePos);
						_draggingSpritePos = pos;
					} else {
						releaseDraggedSprite();
					}
				}
				break;

			case Common::EVENT_LBUTTONDOWN:
				pos = _currentStage->getMousePos();

				// D3 doesn't have both mouse up and down.
				// But we still want to know if the mouse is down for press effects.
				spriteId = sc->getSpriteIDFromPos(pos, true);
				m->_currentMouseDownSpriteId = spriteId;
				m->_currentClickOnSpriteId = spriteId;

				if (spriteId > 0 && sc->_channels[spriteId]->_sprite->shouldHilite())
					g_director->getStage()->invertChannel(sc->_channels[spriteId]);

				m->_lastEventTime = g_director->getMacTicks();
				m->_lastClickTime = m->_lastEventTime;

				debugC(3, kDebugEvents, "event: Button Down @(%d, %d), sprite id: %d", pos.x, pos.y, spriteId);
				_lingo->registerEvent(kEventMouseDown, spriteId);

				if (sc->getSpriteById(spriteId)->_moveable)
					g_director->setDraggedSprite(spriteId);

				break;

			case Common::EVENT_LBUTTONUP:
				pos = _currentStage->getMousePos();

				spriteId = sc->getSpriteIDFromPos(pos, true);

				if (!sc->getChannelById(m->_currentMouseDownSpriteId)->getBbox().contains(pos))
					m->_currentMouseDownSpriteId = 0;

				if (spriteId > 0 && sc->_channels[spriteId]->_sprite->shouldHilite())
					g_director->getStage()->invertChannel(sc->_channels[spriteId]);

				if (!(g_director->_wm->_mode & Graphics::kWMModeButtonDialogStyle))
					m->_currentMouseDownSpriteId = spriteId;

				debugC(3, kDebugEvents, "event: Button Up @(%d, %d), sprite id: %d", pos.x, pos.y, spriteId);

				releaseDraggedSprite();

				{
					CastMember *cast = g_director->getCurrentMovie()->getCastMember(sc->getSpriteById(spriteId)->_castId);
					if (cast && cast->_type == kCastButton)
						cast->_hilite = !cast->_hilite;
				}

				_lingo->registerEvent(kEventMouseUp, spriteId);
				m->_currentMouseDownSpriteId = 0;
				break;

			case Common::EVENT_KEYDOWN:
				_keyCode = _macKeyCodes.contains(event.kbd.keycode) ? _macKeyCodes[event.kbd.keycode] : 0;
				_key = (unsigned char)(event.kbd.ascii & 0xff);

				debugC(1, kDebugEvents, "processEvents(): keycode: %d", _keyCode);

				m->_lastEventTime = g_director->getMacTicks();
				m->_lastKeyTime = m->_lastEventTime;
				_lingo->registerEvent(kEventKeyDown);
				break;

			default:
				break;
			}
		}

		if (!bufferLingoEvents)
			_lingo->processEvents();

		g_system->updateScreen();
		g_system->delayMillis(10);

		if (getVersion() >= 3 && sc->getCurrentFrame() > 0 && !sc->_stopPlay && _lingo->getEventCount() == 0)
			_lingo->registerEvent(kEventIdle);

		if (!bufferLingoEvents)
			_lingo->processEvents();
	}
}

void DirectorEngine::setDraggedSprite(uint16 id) {
	_draggingSprite = true;
	_draggingSpriteId = id;
	_draggingSpritePos = _currentStage->getMousePos();
}

void DirectorEngine::releaseDraggedSprite() {
	_draggingSprite = false;
	_draggingSpriteId = 0;
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
