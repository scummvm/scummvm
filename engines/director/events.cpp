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
#include "common/events.h"

#include "director/director.h"
#include "director/frame.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/lingo/lingo.h"

namespace Director {

void processQuitEvent() {
	Common::Event event;

	while (g_system->getEventManager()->pollEvent(event)) {
		if (event.type == Common::EVENT_QUIT)
			g_director->getCurrentScore()->_stopPlay = true;
	}
}

void DirectorEngine::processEvents() {
	Common::Event event;

	uint endTime = g_system->getMillis() + 200;

	Score *sc = getCurrentScore();
	Frame *currentFrame = sc->_frames[sc->getCurrentFrame()];
	uint16 spriteId = 0;

	// TODO: re-instate when we know which script to run.
	//if (currentFrame > 0)
	//	_lingo->processEvent(kEventIdle, currentFrame - 1);

	Common::Point pos;

	while (g_system->getMillis() < endTime) {
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
				sc->_stopPlay = true;
				break;

			case Common::EVENT_LBUTTONDOWN:
				pos = g_system->getEventManager()->getMousePos();

				// D3 doesn't have both mouse up and down.
				// But we still want to know if the mouse is down for press effects.
				spriteId = currentFrame->getSpriteIDFromPos(pos);
				sc->_currentMouseDownSpriteId = spriteId;

				if (getVersion() > 3) {
					// TODO: check that this is the order of script execution!
					_lingo->processEvent(kEventMouseDown, kCastScript, currentFrame->_sprites[spriteId]->_castId);
					_lingo->processEvent(kEventMouseDown, kSpriteScript, currentFrame->_sprites[spriteId]->_scriptId);
				}
				break;

			case Common::EVENT_LBUTTONUP:
				pos = g_system->getEventManager()->getMousePos();

				spriteId = currentFrame->getSpriteIDFromPos(pos);
				if (getVersion() > 3) {
					// TODO: check that this is the order of script execution!
					_lingo->processEvent(kEventMouseUp, kCastScript, currentFrame->_sprites[spriteId]->_castId);
					_lingo->processEvent(kEventMouseUp, kSpriteScript, currentFrame->_sprites[spriteId]->_scriptId);
				} else {
					// Frame script overrides sprite script
					if (!currentFrame->_sprites[spriteId]->_scriptId)
						_lingo->processEvent(kEventMouseUp, kSpriteScript, currentFrame->_sprites[spriteId]->_castId + 1024);
					else
						_lingo->processEvent(kEventMouseUp, kFrameScript, currentFrame->_sprites[spriteId]->_scriptId);
				}

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
					warning("Keycode: %d", _keyCode);
				}

				_lingo->processEvent(kEventKeyDown, kGlobalScript, 0);
				break;

			default:
				break;
			}
		}

		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

void DirectorEngine::setDraggedSprite(uint16 id) {
	_draggingSprite = true;
	_draggingSpriteId = id;
	_draggingSpritePos = g_system->getEventManager()->getMousePos();

	warning("STUB: DirectorEngine::setDraggedSprite(%d)", id);
}

} // End of namespace Director
