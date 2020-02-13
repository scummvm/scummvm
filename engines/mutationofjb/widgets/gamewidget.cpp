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

#include "mutationofjb/widgets/gamewidget.h"

#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/guiscreen.h"
#include "mutationofjb/mutationofjb.h"
#include "mutationofjb/room.h"

#include "common/events.h"
#include "graphics/screen.h"

namespace MutationOfJB {

GameWidget::GameWidget(GuiScreen &gui) :
	Widget(gui, Common::Rect(GAME_NORMAL_AREA_WIDTH, GAME_NORMAL_AREA_HEIGHT)),
	_currentMapObjectId(0),
	_nextMapObjectId(0),
	_callback(nullptr) {}

void GameWidget::handleEvent(const Common::Event &event) {
	if (!_enabled)
		return;

	if (!_gui.getGame().isCurrentSceneMap()) {
		handleNormalScene(event);
	} else {
		handleMapScene(event);
	}
}

void GameWidget::clearState() {
	_currentMapObjectId = _nextMapObjectId = 0;
}

void GameWidget::draw(Graphics::ManagedSurface &) {
	Room &room = _gui.getGame().getRoom();

	// Full redraw using background buffer.
	if (_dirtyBits == DIRTY_ALL) {
		room.redraw();
		return;
	}

	// Full redraw without background buffer.
	if (_dirtyBits & DIRTY_AFTER_SCENE_CHANGE) {
		room.redraw(false); // Don't use background buffer.
		return;
	}

	// Only selection changed.
	if (_dirtyBits & DIRTY_MAP_SELECTION) {
		if (_currentMapObjectId != _nextMapObjectId) {
			if (_currentMapObjectId) {
				room.drawObjectAnimation(_currentMapObjectId, 1);
			}
			if (_nextMapObjectId) {
				room.drawObjectAnimation(_nextMapObjectId, 0);
			}
			_currentMapObjectId = _nextMapObjectId;
		}
	}
}

void GameWidget::handleNormalScene(const Common::Event &event) {
	Game &game = _gui.getGame();
	Scene *const scene = game.getGameData().getCurrentScene();

	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN: {
		const int16 x = event.mouse.x;
		const int16 y = event.mouse.y;

		if (!_area.contains(x, y))
			break;

		if (Door *const door = scene->findDoor(x, y)) {
			if (_callback)
				_callback->onGameDoorClicked(this, door);
		} else if (Static *const stat = scene->findStatic(x, y)) {
			if (_callback)
				_callback->onGameStaticClicked(this, stat);
		}
		break;
	}
	case Common::EVENT_MOUSEMOVE: {

		const int16 x = event.mouse.x;
		const int16 y = event.mouse.y;

		if (!_area.contains(x, y))
			break;

		bool entityHit = false;
		if (Door *const door = scene->findDoor(x, y)) {
			if (_callback)
				_callback->onGameEntityHovered(this, door->_name);
			entityHit = true;
		} else if (Static *const stat = scene->findStatic(x, y)) {
			if (_callback)
				_callback->onGameEntityHovered(this, stat->_name);
			entityHit = true;
		}

		if (_callback && !entityHit)
			_callback->onGameEntityHovered(this, Common::String());

		_gui.getGame().getEngine().setCursorState(entityHit ? MutationOfJBEngine::CURSOR_ACTIVE : MutationOfJBEngine::CURSOR_IDLE);
		break;
	}
	default:
		break;
	}
}

void GameWidget::handleMapScene(const Common::Event &event) {
	Game &game = _gui.getGame();
	Scene *const scene = game.getGameData().getCurrentScene();

	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN: {
		const int16 x = event.mouse.x;
		const int16 y = event.mouse.y;

		int index = 0;
		if (scene->findBitmap(x, y, &index)) {
			Static *const stat = scene->getStatic(index);
			if (stat && stat->_active == 1) {
				game.startActionSection(ActionInfo::Walk, stat->_name);
			}
		}
		break;
	}
	case Common::EVENT_MOUSEMOVE: {
		const int16 x = event.mouse.x;
		const int16 y = event.mouse.y;

		_nextMapObjectId = 0;

		int index = 0;
		//bool found = false;
		if (scene->findBitmap(x, y, &index)) {
			Static *const stat = scene->getStatic(index);
			if (stat && stat->_active == 1) {
				Object *const object = scene->getObject(index);
				if (object) {
					_nextMapObjectId = index;
				}
			}
		}
		if (_currentMapObjectId != _nextMapObjectId)
			markDirty(DIRTY_MAP_SELECTION);

		_gui.getGame().getEngine().setCursorState(_nextMapObjectId ? MutationOfJBEngine::CURSOR_ACTIVE : MutationOfJBEngine::CURSOR_IDLE);
		break;
	}
	default:
		break;
	}
}

}
