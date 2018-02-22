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

#include "common/debug.h"
#include "common/error.h"
#include "common/system.h"
#include "common/events.h"
#include "graphics/screen.h"
#include "graphics/cursorman.h"

#include "engines/util.h"

#include "mutationofjb/mutationofjb.h"
#include "mutationofjb/room.h"
#include "mutationofjb/game.h"
#include "mutationofjb/encryptedfile.h"
#include "mutationofjb/util.h"

namespace MutationOfJB {

MutationOfJBEngine::MutationOfJBEngine(OSystem *syst)
: Engine(syst),
  _console(nullptr),
  _room(nullptr),
  _screen(nullptr)
{
	debug("MutationOfJBEngine::MutationOfJBEngine");
}

MutationOfJBEngine::~MutationOfJBEngine() {
	debug("MutationOfJBEngine::~MutationOfJBEngine");
}

bool MutationOfJBEngine::loadGameData(bool partB) {
	EncryptedFile file;
	const char *fileName = !partB ? "startup.dat" : "startupb.dat";
	file.open(fileName);
	if (!file.isOpen()) {
		reportFileMissingError(fileName);
		return false;
	}

	_gameData->loadFromStream(file);

	file.close();

	return true;
}

void MutationOfJBEngine::setupCursor()
{
	const uint8 white[] = {0xFF, 0xFF, 0xFF};
	const uint8 cursor[] = {0xFF};

	_screen->setPalette(white, 0xFF, 1);

	CursorMan.disableCursorPalette(true);
	CursorMan.pushCursor(cursor, 1, 1, 0, 0, 0);
	CursorMan.showMouse(true);
}

Common::Error MutationOfJBEngine::run() {
	debug("MutationOfJBEngine::run");

	initGraphics(320, 200);

	_console = new Console(this);
	_screen = new Graphics::Screen;
	setupCursor();

	_gameData = new GameData;
	_gameData->_currentScene = 13;
	loadGameData(false);

	_room = new Room(_screen);
	_room->load(_gameData->_currentScene, false);

	while(!shouldQuit()) {
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_LBUTTONDOWN:
			{
				const Scene* const scene = _gameData->getScene(_gameData->_currentScene);
				if (scene) {
					for (int i = 0; i < MIN(ARRAYSIZE(scene->_doors), (int) scene->_noDoors); ++i) {
						const Door &door = scene->_doors[i];
						if ((event.mouse.x >= door._x) && (event.mouse.x < door._x + door._width) && (event.mouse.y >= door._y) && (event.mouse.y < door._y + door._height)) {
							_gameData->_currentScene = door._destSceneId;
							_room->load(_gameData->_currentScene, false);
						}
					}
				}
				break;
			}
			default:
				break;
			}
		}

		_system->delayMillis(40);
		_screen->update();
	}

	return Common::kNoError;
}
}
