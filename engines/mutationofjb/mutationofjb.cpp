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
#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/debug.h"

namespace MutationOfJB {

MutationOfJBEngine::MutationOfJBEngine(OSystem *syst)
: Engine(syst),
 _console(nullptr),
 _screen(nullptr) {
	debug("MutationOfJBEngine::MutationOfJBEngine");
}

MutationOfJBEngine::~MutationOfJBEngine() {
	debug("MutationOfJBEngine::~MutationOfJBEngine");
}


void MutationOfJBEngine::setupCursor() {
	const uint8 white[] = {0xFF, 0xFF, 0xFF};
	const uint8 cursor[] = {0xFF};

	_screen->setPalette(white, 0xFF, 1);

	CursorMan.disableCursorPalette(true);
	CursorMan.pushCursor(cursor, 1, 1, 0, 0, 0);
	CursorMan.showMouse(true);
}

Graphics::Screen *MutationOfJBEngine::getScreen() const {
	return _screen;
}

Game &MutationOfJBEngine::getGame() {
	return *_game;
}

Common::Error MutationOfJBEngine::run() {
	debug("MutationOfJBEngine::run");

	initGraphics(320, 200);

	_console = new Console(this);
	_screen = new Graphics::Screen();
	_game = new Game(this);
	ActionInfo::Action currentAction = ActionInfo::Walk;

	setupCursor();

	while(!shouldQuit()) {
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
			{
				if ((event.kbd.hasFlags(Common::KBD_CTRL) && event.kbd.keycode == Common::KEYCODE_d) ||
						event.kbd.ascii == '~' || event.kbd.ascii == '#') {
					_console->attach();
				}
				break;
			}
			case Common::EVENT_LBUTTONDOWN:
			{
				if (Door *const door = _game->findDoor(event.mouse.x, event.mouse.y)) {
					if (!_game->startActionSection(currentAction, door->_name) && currentAction == ActionInfo::Walk && door->_destSceneId != 0) {
						_game->changeScene(door->_destSceneId, _game->getGameData()._partB);
					}
				} else if (Static *const stat = _game->findStatic(event.mouse.x, event.mouse.y)) {
					_game->startActionSection(currentAction, stat->_name);
				}
				break;
			}
			case Common::EVENT_KEYUP:
			{
				switch (event.kbd.ascii) {
				case 'g':
					currentAction = ActionInfo::Walk;
					break;
				case 'r':
					currentAction = ActionInfo::Talk;
					break;
				case 's':
					currentAction = ActionInfo::Look;
					break;
				case 'b':
					currentAction = ActionInfo::Use;
					break;
				case 'n':
					currentAction = ActionInfo::PickUp;
					break;
				}
			}
			default:
				break;
			}
		}

		_console->onFrame();
		_game->update();
		_system->delayMillis(40);
		_screen->update();
	}

	return Common::kNoError;
}

}
