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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/system.h"
#include "common/events.h"
#include "common/fs.h"
#include "graphics/screen.h"
#include "graphics/cursorman.h"

#include "engines/util.h"

#include "mutationofjb/mutationofjb.h"
#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/debug.h"
#include "mutationofjb/room.h"

namespace MutationOfJB {

MutationOfJBEngine::MutationOfJBEngine(OSystem *syst)
	: Engine(syst),
	  _console(nullptr),
	  _screen(nullptr),
	  _game(nullptr),
	  _mapObjectId(0),
	  _cursorState(CURSOR_IDLE) {

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "data");
}

MutationOfJBEngine::~MutationOfJBEngine() {}

void MutationOfJBEngine::setupCursor() {
	const uint8 cursor[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	updateCursorPalette();

	CursorMan.disableCursorPalette(true);
	CursorMan.pushCursor(cursor, 15, 15, 7, 7, 0);
	CursorMan.showMouse(true);
}

void MutationOfJBEngine::updateCursorPalette() {
	if (_cursorState == CURSOR_OFF) {
		return;
	}

	const uint8 white[] = {0xFF, 0xFF, 0xFF};
	const uint8 blue[] = {0x00, 0xFF, 0xC3};

	_screen->setPalette(_cursorState == CURSOR_ACTIVE ? blue : white, 0xFF, 1);
}

Graphics::Screen *MutationOfJBEngine::getScreen() const {
	return _screen;
}

Game &MutationOfJBEngine::getGame() {
	return *_game;
}

void MutationOfJBEngine::setCursorState(CursorState cursorState) {
	if (_cursorState == cursorState) {
		return;
	}

	_cursorState = cursorState;
	updateCursorPalette();
}

void MutationOfJBEngine::updateCursor() {
	if (_cursorState == CURSOR_OFF) {
		return;
	}

	if (_game->isCurrentSceneMap()) {
		if (_cursorState != CURSOR_IDLE) {
			_cursorState = CURSOR_IDLE;
			updateCursorPalette();
		}
	} else {
		const Common::Point point = _eventMan->getMousePos();
		updateCursorHitTest(point.x, point.y);
	}
}

void MutationOfJBEngine::handleNormalScene(const Common::Event &event) {
	Scene *const scene = _game->getGameData().getCurrentScene();

	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN: {
		const int16 x = event.mouse.x;
		const int16 y = event.mouse.y;

		if (Door *const door = scene->findDoor(x, y)) {
			if (!_game->startActionSection(_game->getCurrentAction(), door->_name) && _game->getCurrentAction() == ActionInfo::Walk && door->_destSceneId != 0) {
				_game->changeScene(door->_destSceneId, _game->getGameData()._partB);
			}
		} else if (Static *const stat = scene->findStatic(x, y)) {
			_game->startActionSection(_game->getCurrentAction(), stat->_name);
		}
		break;
	}
	case Common::EVENT_MOUSEMOVE: {
		const int16 x = event.mouse.x;
		const int16 y = event.mouse.y;

		if (_cursorState != CURSOR_OFF) {
			updateCursorHitTest(x, y);
		}
		break;
	}
	default:
		break;
	}
	_game->getGui().handleEvent(event);
}

void MutationOfJBEngine::handleMapScene(const Common::Event &event) {
	Scene *const scene = _game->getGameData().getCurrentScene();

	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN: {
		const int16 x = event.mouse.x;
		const int16 y = event.mouse.y;

		int index = 0;
		if (Bitmap *const bitmap = scene->findBitmap(x, y, &index)) {
			Static *const stat = scene->getStatic(index);
			if (stat && stat->_active == 1) {
				_game->startActionSection(ActionInfo::Walk, stat->_name);
			}
		}
		break;
	}
	case Common::EVENT_MOUSEMOVE: {
		const int16 x = event.mouse.x;
		const int16 y = event.mouse.y;

		int index = 0;
		bool found = false;
		if (Bitmap *const bitmap = scene->findBitmap(x, y, &index)) {
			Static *const stat = scene->getStatic(index);
			if (stat && stat->_active == 1) {
				Object *const object = scene->getObject(index);
				if (object) {
					found = true;
					if (index != _mapObjectId) {
						if (_mapObjectId) {
							_game->getRoom().drawObjectAnimation(_mapObjectId, 1);
							_mapObjectId = 0;
						}

						_mapObjectId = index;
						_game->getRoom().drawObjectAnimation(_mapObjectId, 0);
					}
				}
			}
		}

		if (!found && _mapObjectId != 0) {
			_game->getRoom().drawObjectAnimation(_mapObjectId, 1);
			_mapObjectId = 0;
		}
		break;
	}
	default:
		break;
	}
}

void MutationOfJBEngine::updateCursorHitTest(int16 x, int16 y) {
	Scene *const scene = _game->getGameData().getCurrentScene();
	if (!scene) {
		return;
	}

	bool entityHit = false;
	if (Door *const door = scene->findDoor(x, y)) {
		if (door->_destSceneId != 0) {
			entityHit = true;
		}
	} else if (Static *const stat = scene->findStatic(x, y)) {
		entityHit = true;
	}
	bool cursorPaletteChange = false;
	if ((_cursorState == CURSOR_ACTIVE && !entityHit) || (_cursorState == CURSOR_IDLE && entityHit)) {
		cursorPaletteChange = true;
	}
	_cursorState = entityHit ? CURSOR_ACTIVE : CURSOR_IDLE;
	if (cursorPaletteChange) {
		updateCursorPalette();
	}

}

Common::Error MutationOfJBEngine::run() {
	initGraphics(320, 200);

	_console = new Console(this);
	_screen = new Graphics::Screen();
	_game = new Game(this);

	setupCursor();

	_game->changeScene(13, false); // Initial scene.

	while (!shouldQuit()) {
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN: {
				if ((event.kbd.hasFlags(Common::KBD_CTRL) && event.kbd.keycode == Common::KEYCODE_d) ||
						event.kbd.ascii == '~' || event.kbd.ascii == '#') {
					_console->attach();
				}
				break;
			}
			case Common::EVENT_KEYUP: {
				switch (event.kbd.ascii) {
				case 'g':
					_game->setCurrentAction(ActionInfo::Walk);
					break;
				case 'r':
					_game->setCurrentAction(ActionInfo::Talk);
					break;
				case 's':
					_game->setCurrentAction(ActionInfo::Look);
					break;
				case 'b':
					_game->setCurrentAction(ActionInfo::Use);
					break;
				case 'n':
					_game->setCurrentAction(ActionInfo::PickUp);
					break;
				}
				break;
			}
			default:
				break;
			}

			if (!_game->isCurrentSceneMap()) {
				handleNormalScene(event);
			} else {
				handleMapScene(event);
			}
		}

		_console->onFrame();
		_game->update();
		_system->delayMillis(10);
		_screen->update();
	}

	return Common::kNoError;
}

}
