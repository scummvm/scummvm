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

#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/gameinterface.h"

#include "engines/stark/ui/dialogpanel.h"
#include "engines/stark/ui/fmvplayer.h"
#include "engines/stark/ui/gamewindow.h"
#include "engines/stark/ui/inventorywindow.h"
#include "engines/stark/ui/topmenu.h"

namespace Stark {

UI::UI(Gfx::Driver *gfx, Cursor *cursor) :
	_gfx(gfx),
	_cursor(cursor),
	_topMenu(nullptr),
	_dialogPanel(nullptr),
	_inventoryWindow(nullptr),
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
	delete _dialogPanel;
	delete _inventoryWindow;
	delete _fmvPlayer;
}

void UI::init() {
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

void UI::update() {
	StaticProvider *staticProvider = StarkServices::instance().staticProvider;
	staticProvider->onGameLoop();

	// Check for UI mouse overs
	for (uint i = 0; i < _windows.size(); i++) {
		if (_windows[i]->isVisible() && _windows[i]->isMouseInside()) {
			_windows[i]->handleMouseMove();
			return;
		}
	}
}

void UI::handleClick() {
	for (uint i = 0; i < _windows.size(); i++) {
		if (_windows[i]->isMouseInside()) {
			_windows[i]->handleClick();
			return;
		}
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
	_dialogPanel->notifySubtitle(subtitle);
}

void UI::notifyDialogOptions(const Common::StringArray &options) {
	_dialogPanel->notifyDialogOptions(options);
}

void UI::notifyShouldOpenInventory() {
	// Make the inventory update it's contents.
	_inventoryWindow->open();
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
	// TODO: Unify with the other windows
	if (_fmvPlayer->isPlaying()) {
		_fmvPlayer->render();
		return;
	}

	for (int i = _windows.size() - 1; i >= 0; i--) {
		_windows[i]->render();
	}
}

} // End of namespace Stark

