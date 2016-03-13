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

#include "titanic/titanic.h"
#include "titanic/main_game_window.h"
#include "titanic/game_manager.h"
#include "titanic/game_view.h"

namespace Titanic {

CMainGameWindow::CMainGameWindow(TitanicEngine *vm): _vm(vm) {
	_gameView = nullptr;
	_gameManager = nullptr;
	_project = nullptr;
	_field50 = 0;
	_image = nullptr;
	_cursor = nullptr;
}

bool CMainGameWindow::Create() {
	Image image;
	bool result = image.loadResource("TITANIC");
	if (!result)
		return true;

	// TODO: Stuff
	return true;
}

void CMainGameWindow::applicationStarting() {
	// Set up the game project, and get game slot
	int saveSlot = loadGame();
	assert(_project);
	
	// Set the video mode
	CScreenManager *screenManager = CScreenManager::setCurrent();
	screenManager->setMode(640, 480, 1, 1, false);

	// TODO: Clear surfaces

	// Create game view and manager
	_gameView = new CSTGameView(this);
	_gameManager = new CGameManager(_project, _gameView);
	_gameView->setGameManager(_gameManager);

	// Load either a new game or selected existing save
	_project->loadGame(saveSlot);

	// TODO: Cursor/image and message generation
}

int CMainGameWindow::loadGame() {
	_project = new CProjectItem();
	_project->setFilename("starship.prj");

	return selectSavegame();
}

int CMainGameWindow::selectSavegame() {
	// TODO: For now, hardcoded to -1 for new saves
	return -1;
}

void CMainGameWindow::fn1(int v) {
	warning("TODO");
}

void CMainGameWindow::fn2() {
	warning("TODO");
}

} // End of namespace Titanic
