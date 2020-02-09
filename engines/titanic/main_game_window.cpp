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

#include "titanic/main_game_window.h"
#include "titanic/continue_save_dialog.h"
#include "titanic/debugger.h"
#include "titanic/game_manager.h"
#include "titanic/game_view.h"
#include "titanic/messages/messages.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/support/files_manager.h"
#include "titanic/titanic.h"
#include "common/config-manager.h"
#include "graphics/screen.h"

namespace Titanic {

CMainGameWindow::CMainGameWindow(TitanicEngine *vm): _vm(vm),
		_priorLeftDownTime(0), _priorMiddleDownTime(0) {
	_gameView = nullptr;
	_gameManager = nullptr;
	_project = nullptr;
	_inputAllowed = false;
	_image = nullptr;
	_cursor = nullptr;
	_pendingLoadSlot = -1;

	// Set the window as an event target
	vm->_events->addTarget(this);
}

CMainGameWindow::~CMainGameWindow() {
	delete _gameView;
	delete _gameManager;
	delete _project;
}

void CMainGameWindow::applicationStarting() {
	// Set the video mode
	CScreenManager *screenManager = CScreenManager::setCurrent();
	screenManager->setMode(640, 480, 16, 0, true);

	// Show the initial copyright & info screen for the game
	if (!isLoadingFromLauncher()) {
		Image image;
		image.load("Bitmap/TITANIC");
		_vm->_screen->blitFrom(image, Point(
			SCREEN_WIDTH / 2 - image.w / 2,
			SCREEN_HEIGHT / 2 - image.h / 2
			));

		// Delay for 5 seconds
		const int NUM_STEPS = 20;
		for (int idx = 0; idx < NUM_STEPS; ++idx) {
			_vm->_events->sleep(5000 / NUM_STEPS);
			if (_vm->_loadSaveSlot >= 0)
				break;
		}
	}

	// Set up the game project, and get game slot
	int saveSlot = getSavegameSlot();
	if (saveSlot == -2)
		return;

	// Create game view and manager
	_gameView = new CSTGameView(this);
	_gameManager = new CGameManager(_project, _gameView, g_vm->_mixer);
	_gameView->setGameManager(_gameManager);

	// Load either a new game or selected existing save
	_project->loadGame(saveSlot);
	_inputAllowed = true;
	_gameManager->_gameState.setMode(GSMODE_INTERACTIVE);

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

	_gameManager->markAllDirty();
}

int CMainGameWindow::getSavegameSlot() {
	_project = new CProjectItem();
	_project->setFilename("starship.prj");

	return selectSavegame();
}

bool CMainGameWindow::isLoadingFromLauncher() const {
	return ConfMan.hasKey("save_slot");
}

int CMainGameWindow::selectSavegame() {
	// If a savegame was selected from GMM during the startup, return it
	if (g_vm->_loadSaveSlot != -1)
		return g_vm->_loadSaveSlot;

	// If the user selected a savegame from the launcher, return it
	if (ConfMan.hasKey("save_slot"))
		return ConfMan.getInt("save_slot");

	CContinueSaveDialog dialog;
	bool hasSavegames = false;

	// Loop through save slots to find any existing save slots
	for (int idx = 0; idx <= MAX_SAVES; ++idx) {
		CString saveName = g_vm->getSavegameName(idx);
		if (!saveName.empty()) {
			dialog.addSavegame(idx, saveName);
			hasSavegames = true;
		}
	}

	// If there are savegames, show the select dialog and get a choice.
	// If there aren't, return -1 to indicate starting a new game
	return hasSavegames ? dialog.show() : -1;
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
		if (!_gameView->_surface) {
			CViewItem *view = _gameManager->getView();
			if (view)
				setActiveView(view);
		}

		CScreenManager *scrManager = CScreenManager::setCurrent();
		scrManager->clearSurface(SURFACE_BACKBUFFER, &_gameManager->_bounds);

		switch (_gameManager->_gameState._mode) {
		case GSMODE_PENDING_LOAD:
			// Pending savegame to load
			_gameManager->_gameState.setMode(GSMODE_INTERACTIVE);
			_project->loadGame(_pendingLoadSlot);
			_pendingLoadSlot = -1;

			_gameManager->markAllDirty();
			scrManager->setSurfaceBounds(SURFACE_PRIMARY, Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));

			// Intentional fall-through
			// to draw loaded game

		case GSMODE_INTERACTIVE:
		case GSMODE_CUTSCENE:
			if (_gameManager->_gameState._petActive)
				drawPet(scrManager);

			drawView();
			drawViewContents(scrManager);
			scrManager->drawCursors();
			break;

		case GSMODE_INSERT_CD:
			scrManager->drawCursors();
			_vm->_filesManager->insertCD(scrManager);
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
			petControl->draw(screenManager);
	}
}

void CMainGameWindow::drawView() {
	if (_gameView->_surface)
		_gameView->drawView();
}

void CMainGameWindow::drawViewContents(CScreenManager *screenManager) {
	// Get a reference to the room, validating that it's present
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
	if (_gameManager->_gameState._mode != GSMODE_INSERT_CD)
		_gameManager->update();
}

void CMainGameWindow::loadGame(int slotId) {
	_pendingLoadSlot = slotId;
	_gameManager->_gameState.setMode(GSMODE_PENDING_LOAD);
}

void CMainGameWindow::onIdle() {
	if (!_inputAllowed)
		return;
	CGameManager *gameManager = _gameManager;
	if (!gameManager)
		return;

	// Let the game manager perform any game updates
	gameManager->update();

	if (gameManager->_gameState._quitGame) {
		// Game needs to shut down
		_vm->quitGame();
	}
}

#define HANDLE_MESSAGE(METHOD) 	if (_inputAllowed) { \
	_gameManager->_inputTranslator.METHOD(g_vm->_events->getSpecialButtons(), mousePos); \
	mouseChanged(); \
	}


void CMainGameWindow::mouseMove(const Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	HANDLE_MESSAGE(mouseMove)
}

void CMainGameWindow::leftButtonDown(const Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	if ((_vm->_events->getTicksCount() - _priorLeftDownTime) < DOUBLE_CLICK_TIME) {
		_priorLeftDownTime = 0;
		leftButtonDoubleClick(mousePos);
	} else {
		_priorLeftDownTime = _vm->_events->getTicksCount();
		HANDLE_MESSAGE(leftButtonDown)
	}
}

void CMainGameWindow::leftButtonUp(const Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	HANDLE_MESSAGE(leftButtonUp)
}

void CMainGameWindow::leftButtonDoubleClick(const Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	HANDLE_MESSAGE(leftButtonDoubleClick)
}

void CMainGameWindow::middleButtonDown(const Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	if ((_vm->_events->getTicksCount() - _priorMiddleDownTime) < DOUBLE_CLICK_TIME) {
		_priorMiddleDownTime = 0;
		middleButtonDoubleClick(mousePos);
	} else {
		_priorMiddleDownTime = _vm->_events->getTicksCount();
		HANDLE_MESSAGE(middleButtonDown)
	}
}

void CMainGameWindow::middleButtonUp(const Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	HANDLE_MESSAGE(middleButtonUp)
}

void CMainGameWindow::middleButtonDoubleClick(const Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	HANDLE_MESSAGE(middleButtonDoubleClick)
}

void CMainGameWindow::mouseWheel(const Point &mousePos, bool wheelUp) {
	if (!isMouseControlEnabled())
		return;

	_gameManager->_inputTranslator.mouseWheel(wheelUp, mousePos);
	mouseChanged();
}

void CMainGameWindow::keyDown(Common::KeyState keyState) {
	if (keyState.keycode == Common::KEYCODE_c && (keyState.flags & Common::KBD_CTRL)) {
		// Cheat action
		if (_project && g_vm->canLoadGameStateCurrently()) {
			CViewItem *newView = _project->parseView("Cheat Room.Node 1.Cheat Rooms View");
			_gameManager->_gameState.changeView(newView, nullptr);
		}

	} else if (keyState.keycode == Common::KEYCODE_F5) {
		// Show the GMM save dialog
		g_vm->saveGameDialog();
	} else if (keyState.keycode == Common::KEYCODE_F7) {
		// Show the GMM load dialog
		g_vm->loadGameDialog();
	} else if (_inputAllowed) {
		_gameManager->_inputTranslator.keyDown(keyState);
	}
}

bool CMainGameWindow::isMouseControlEnabled() const {
	if (!_gameManager)
		return false;

	CScreenManager *screenMan = CScreenManager::_screenManagerPtr;
	if (!screenMan || !screenMan->_mouseCursor)
		return true;

	return screenMan->_mouseCursor->_inputEnabled;
}

} // End of namespace Titanic
