/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/ui/world/gamescreen.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/userinterface.h"
#include "engines/stark/services/global.h"

#include "engines/stark/ui/cursor.h"
#include "engines/stark/ui/world/actionmenu.h"
#include "engines/stark/ui/world/dialogpanel.h"
#include "engines/stark/ui/world/gamewindow.h"
#include "engines/stark/ui/world/inventorywindow.h"
#include "engines/stark/ui/world/topmenu.h"

#include "engines/stark/resources/level.h"
#include "engines/stark/resources/location.h"

namespace Stark {

GameScreen::GameScreen(Gfx::Driver *gfx, Cursor *cursor) :
		Screen(Screen::kScreenGame),
		_gfx(gfx),
		_cursor(cursor) {

	_topMenu = new TopMenu(_gfx, _cursor);
	_dialogPanel = new DialogPanel(_gfx, _cursor);
	_actionMenu = new ActionMenu(_gfx, _cursor);
	_inventoryWindow = new InventoryWindow(_gfx, _cursor, _actionMenu);
	_actionMenu->setInventory(_inventoryWindow);
	_gameWindow = new GameWindow(_gfx, _cursor, _actionMenu, _inventoryWindow);

	_gameScreenWindows.push_back(_actionMenu);
	_gameScreenWindows.push_back(_inventoryWindow);
	_gameScreenWindows.push_back(_gameWindow);
	_gameScreenWindows.push_back(_topMenu);
	_gameScreenWindows.push_back(_dialogPanel);
}

GameScreen::~GameScreen() {
	delete _gameWindow;
	delete _actionMenu;
	delete _topMenu;
	delete _dialogPanel;
	delete _inventoryWindow;
}

void GameScreen::open() {
	pauseGame(false);
	StarkUserInterface->freeGameScreenThumbnail();
}

void GameScreen::close() {
	_cursor->setMouseHint("");
	pauseGame(true);
	StarkUserInterface->saveGameScreenThumbnail();
}

void GameScreen::handleGameLoop() {
	for (int i = _gameScreenWindows.size() - 1; i >= 0; i--) {
		_gameScreenWindows[i]->handleGameLoop();
	}
}

void GameScreen::render() {
	for (int i = _gameScreenWindows.size() - 1; i >= 0; i--) {
		_gameScreenWindows[i]->render();
	}
}

InventoryWindow *GameScreen::getInventoryWindow() const {
	return _inventoryWindow;
}

void GameScreen::reset() {
	_dialogPanel->reset();
	_gameWindow->reset();
	_inventoryWindow->reset();
	_actionMenu->close();
}

GameWindow *GameScreen::getGameWindow() const {
	return _gameWindow;
}

DialogPanel *GameScreen::getDialogPanel() const {
	return _dialogPanel;
}

void GameScreen::handleMouseMove() {
	dispatchEvent(&Window::handleMouseMove);
}

void GameScreen::handleClick() {
	dispatchEvent(&Window::handleClick);
}

void GameScreen::handleRightClick() {
	dispatchEvent(&Window::handleRightClick);
}

void GameScreen::handleDoubleClick() {
	dispatchEvent(&Window::handleDoubleClick);
}

void GameScreen::dispatchEvent(WindowHandler handler) {
	for (uint i = 0; i < _gameScreenWindows.size(); i++) {
		if (_gameScreenWindows[i]->isMouseInside()) {
			(*_gameScreenWindows[i].*handler)();
			return;
		}
	}
}

void GameScreen::onScreenChanged() {
	_cursor->onScreenChanged();
	_dialogPanel->onScreenChanged();
	_topMenu->onScreenChanged();
	_gameWindow->onScreenChanged();
	_actionMenu->onScreenChanged();
}

void GameScreen::notifyInventoryItemEnabled(uint16 itemIndex) {
	_topMenu->notifyInventoryItemEnabled(itemIndex);
}

void GameScreen::notifyDiaryEntryEnabled() {
	_topMenu->notifyDiaryEntryEnabled();
}

void GameScreen::pauseGame(bool pause) {
	if (StarkGlobal->getLevel()) {
		StarkGlobal->getLevel()->onEnginePause(pause);
	}
	if (StarkGlobal->getCurrent()) {
		StarkGlobal->getCurrent()->getLevel()->onEnginePause(pause);
		StarkGlobal->getCurrent()->getLocation()->onEnginePause(pause);
	}
}

} // End of namespace Stark
