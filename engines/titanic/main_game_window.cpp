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
#include "titanic/messages/messages.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

CMainGameWindow::CMainGameWindow(TitanicEngine *vm): _vm(vm) {
	_gameView = nullptr;
	_gameManager = nullptr;
	_project = nullptr;
	_inputAllowed = false;
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
	screenManager->setMode(640, 480, 16, 0, true);

	// TODO: Remove initial background and palette


	// Create game view and manager
	_gameView = new CSTGameView(this);
	_gameManager = new CGameManager(_project, _gameView);
	_gameView->setGameManager(_gameManager);

	// Load either a new game or selected existing save
	_project->loadGame(saveSlot);
	_inputAllowed = true;
	_gameManager->_gameState.setMode(GSMODE_1);

	// TODO: Cursor/image

	// Generate starting messages for entering the view, node, and room.
	// Note the old fields are nullptr, since there's no previous view/node/room
	CViewItem *view = _gameManager->_gameState._gameLocation.getView();
	CEnterViewMsg enterViewMsg(nullptr, view);
	enterViewMsg.execute(view, nullptr, MSGFLAG_SCAN);

	CNodeItem *node = view->findNode();
	CEnterNodeMsg enterNodeMsg(nullptr, node);
	enterNodeMsg.execute(node, nullptr, MSGFLAG_SCAN);

	CRoomItem *room = view->findRoom();
	CEnterRoomMsg enterRoomMsg(nullptr, room);
	enterRoomMsg.execute(room, nullptr, MSGFLAG_SCAN);

	_gameManager->initBounds();
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

void CMainGameWindow::setActiveView(CViewItem *viewItem) {
	_gameManager->_gameState._gameLocation.setView(viewItem);
	
	CResourceKey key;
	if (viewItem->getResourceKey(&key)) {
		// Create a surface based on the key
		_gameView->createSurface(key);
	}
}

void CMainGameWindow::draw() {
	if (_gameManager) {
		if (_gameView->_surface) {
			CViewItem *view = _gameManager->getView();
			if (view)
				setActiveView(view);
		}

		CScreenManager *scrManager = CScreenManager::setCurrent();
		scrManager->clearSurface(SURFACE_BACKBUFFER, &_gameManager->_bounds);

		switch (_gameManager->_gameState._mode) {
		case GSMODE_1:
		case GSMODE_2:
			if (_gameManager->_gameState._petActive)
				drawPet(scrManager);

			drawView();
			drawViewContents(scrManager);
			scrManager->drawCursors();
			break;

		case GSMODE_5:
			g_vm->_filesManager.debug(scrManager);
			break;

		default:
			break;
		}
	}
}

void CMainGameWindow::drawPet(CScreenManager *screenManager) {
	if (_gameView && _gameView->_surface) {
		CPetControl *petControl = _gameManager->_project->getPetControl();
		if (petControl)
			petControl->proc26();
	}
}

void CMainGameWindow::drawView() {
	if (_gameView->_surface)
		_gameView->drawView();
}

void CMainGameWindow::drawViewContents(CScreenManager *screenManager) {
	// Get a reference to the reference, validating that it's present
	if (!screenManager)
		return;
	CViewItem *view = _gameManager->getView();
	if (!view)
		return;
	CNodeItem *node = view->findNode();
	if (!node)
		return;
	CRoomItem *room = node->findRoom();
	if (!room)
		return;

	double xVal = 0.0, yVal = 0.0;
	room->calcNodePosition(node->_nodePos, xVal, yVal);

	// Iterate through drawing all the items in the scene except any item
	// that's currently being dragged
	for (CTreeItem *treeItem = view; treeItem; treeItem = treeItem->scan(view)) {
		if (treeItem != _gameManager->_dragItem)
			treeItem->draw(screenManager);
	}

	// Finally draw the drag item if there is one
	if (_gameManager->_dragItem)
		_gameManager->_dragItem->draw(screenManager);
}

void CMainGameWindow::mouseChanged() {
	if (_gameManager->_gameState._mode != GSMODE_5)
		_gameManager->update();
}

} // End of namespace Titanic
