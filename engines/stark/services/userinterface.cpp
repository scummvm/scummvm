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

#include "common/system.h"
#include "common/events.h"

#include "engines/stark/services/userinterface.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/gameinterface.h"

#include "engines/stark/ui/actionmenu.h"
#include "engines/stark/ui/dialogpanel.h"
#include "engines/stark/ui/fmvplayer.h"
#include "engines/stark/ui/gamewindow.h"
#include "engines/stark/ui/inventorywindow.h"
#include "engines/stark/ui/topmenu.h"

namespace Stark {

UserInterface::UserInterface(Gfx::Driver *gfx, Cursor *cursor) :
	_gfx(gfx),
	_cursor(cursor),
	_topMenu(nullptr),
	_dialogPanel(nullptr),
	_inventoryWindow(nullptr),
	_exitGame(false),
	_fmvPlayer(nullptr),
	_actionMenu(nullptr),
	_gameWindow(nullptr),
	_interactive(true)
	{
}

UserInterface::~UserInterface() {
	delete _gameWindow;
	delete _actionMenu;
	delete _topMenu;
	delete _dialogPanel;
	delete _inventoryWindow;
	delete _fmvPlayer;
}

void UserInterface::init() {
	_topMenu = new TopMenu(_gfx, _cursor);
	_dialogPanel = new DialogPanel(_gfx, _cursor);
	_fmvPlayer = new FMVPlayer(_gfx, _cursor);
	_actionMenu = new ActionMenu(_gfx, _cursor);
	_inventoryWindow = new InventoryWindow(_gfx, _cursor, _actionMenu);
	_actionMenu->setInventory(_inventoryWindow);
	_gameWindow = new GameWindow(_gfx, _cursor, _actionMenu, _inventoryWindow);

	_windows.push_back(_actionMenu);
	_windows.push_back(_inventoryWindow);
	_windows.push_back(_gameWindow);
	_windows.push_back(_topMenu);
	_windows.push_back(_dialogPanel);
}

void UserInterface::update() {
	StarkStaticProvider->onGameLoop();

	// Check for UI mouse overs
	for (uint i = 0; i < _windows.size(); i++) {
		if (_windows[i]->isVisible() && _windows[i]->isMouseInside()) {
			_windows[i]->handleMouseMove();
			return;
		}
	}
}

void UserInterface::handleClick() {
	for (uint i = 0; i < _windows.size(); i++) {
		if (_windows[i]->isMouseInside()) {
			_windows[i]->handleClick();
			return;
		}
	}
}

void UserInterface::handleRightClick() {
	for (uint i = 0; i < _windows.size(); i++) {
		if (_windows[i]->isMouseInside()) {
			_windows[i]->handleRightClick();
			return;
		}
	}
}

void UserInterface::notifyShouldOpenInventory() {
	// Make the inventory update its contents.
	_inventoryWindow->open();
}

void UserInterface::notifyFMVRequest(const Common::String &name) {
	_fmvPlayer->play(name);
}

bool UserInterface::isPlayingFMV() const {
	return _fmvPlayer->isPlaying();
}

void UserInterface::stopPlayingFMV() {
	_fmvPlayer->stop();
}

void UserInterface::render() {
	// TODO: Unify with the other windows
	if (_fmvPlayer->isPlaying()) {
		_fmvPlayer->render();
		return;
	}

	for (int i = _windows.size() - 1; i >= 0; i--) {
		_windows[i]->render();
	}
}

bool UserInterface::isInteractive() const {
	return _interactive;
}

void UserInterface::setInteractive(bool interactive) {
	_interactive = interactive;
}

} // End of namespace Stark

