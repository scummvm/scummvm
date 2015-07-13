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

#include "engines/stark/ui.h"

#include "engines/stark/ui/actionmenu.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/services/fmvplayer.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/userinterface.h"

#include "engines/stark/ui/dialoginterface.h"
#include "engines/stark/ui/gamewindow.h"
#include "engines/stark/ui/inventoryinterface.h"
#include "engines/stark/ui/topmenu.h"

namespace Stark {

UI::UI(Gfx::Driver *gfx, Cursor *cursor) :
	_gfx(gfx),
	_cursor(cursor),
	_topMenu(nullptr),
	_dialogInterface(nullptr),
	_inventoryInterface(nullptr),
	_exitGame(false),
	_fmvPlayer(nullptr),
	_actionMenu(nullptr),
	_gameWindow(nullptr)
	{
}

UI::~UI() {
	delete _gameWindow;
	delete _actionMenu;
	delete _topMenu;
	delete _dialogInterface;
	delete _inventoryInterface;
	delete _fmvPlayer;
}

void UI::init() {
	_topMenu = new TopMenu(_gfx, _cursor);
	_dialogInterface = new DialogInterface();
	_fmvPlayer = new FMVPlayer();
	_actionMenu = new ActionMenu(_gfx, _cursor);
	_inventoryInterface = new InventoryInterface(_gfx, _cursor, _actionMenu);
	_actionMenu->setInventory(_inventoryInterface);
	_gameWindow = new GameWindow(_gfx, _cursor, _actionMenu, _inventoryInterface);

	_windows.push_back(_actionMenu);
	_windows.push_back(_inventoryInterface);
	_windows.push_back(_gameWindow);
	_windows.push_back(_topMenu);
}

void UI::update() {
	StaticProvider *staticProvider = StarkServices::instance().staticProvider;
	staticProvider->onGameLoop();

	// Check for UI mouse overs
	_topMenu->handleMouseMove();

	if (_actionMenu->isVisible() && _actionMenu->isMouseInside()) {
		_actionMenu->handleMouseMove();
		return;
	}

	if (_inventoryInterface->isVisible()) {
		_inventoryInterface->handleMouseMove();
		return;
	}

	if (_gameWindow->isVisible() && _gameWindow->isMouseInside()) {
		_gameWindow->handleMouseMove();
		return;
	}

	Common::Point pos = _cursor->getMousePosition();
	if (_dialogInterface->containsPoint(pos)) {
		_dialogInterface->handleMouseOver(pos);
		return;
	}
}

void UI::handleClick() {
	for (uint i = 0; i < _windows.size(); i++) {
		if (_windows[i]->isMouseInside()) {
			_windows[i]->handleClick();
			return;
		}
	}

	// TODO: Unify with the other windows
	if (_dialogInterface->containsPoint(_cursor->getMousePosition())) {
		_dialogInterface->handleClick(_cursor->getMousePosition());
		return;
	}
}

void UI::handleRightClick() {
	for (uint i = 0; i < _windows.size(); i++) {
		if (_windows[i]->isMouseInside()) {
			_windows[i]->handleRightClick();
			return;
		}
	}
}

void UI::notifySubtitle(const Common::String &subtitle) {
	_dialogInterface->notifySubtitle(subtitle);
}

void UI::notifyDialogOptions(const Common::StringArray &options) {
	_dialogInterface->notifyDialogOptions(options);
}

void UI::notifyShouldOpenInventory() {
	// Make the inventory update it's contents.
	_inventoryInterface->open();
}

void UI::notifyFMVRequest(const Common::String &name) {
	_fmvPlayer->play(name);
}

bool UI::isPlayingFMV() const {
	return _fmvPlayer->isPlaying();
}

void UI::stopPlayingFMV() {
	_fmvPlayer->stop();
}

void UI::render() {
	if (_fmvPlayer->isPlaying()) {
		_fmvPlayer->render();
		return;
	}

	// TODO: Unhardcode
	if (_cursor->getMousePosition().y < 40) {
		_topMenu->render();
	}

	_gameWindow->render();
	_inventoryInterface->render();

	_dialogInterface->render();
	_actionMenu->render();
}

} // End of namespace Stark

