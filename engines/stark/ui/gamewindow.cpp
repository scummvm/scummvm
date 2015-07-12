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

#include "engines/stark/ui/gamewindow.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/knowledgeset.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/location.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/userinterface.h"

#include "engines/stark/actionmenu.h"
#include "engines/stark/ui/inventoryinterface.h"

namespace Stark {

GameWindow::GameWindow(Gfx::Driver *gfx, Cursor *cursor, ActionMenu *actionMenu, InventoryInterface *inventory) :
	Window(gfx, cursor),
	_actionMenu(actionMenu),
	_inventory(inventory),
	_objectUnderCursor(nullptr) {
	_position = Common::Rect(Gfx::Driver::kGameViewportWidth, Gfx::Driver::kGameViewportHeight);
	_position.translate(0, Gfx::Driver::kTopBorderHeight);
	_visible = true;
}

void GameWindow::onRender() {
	Global *global = StarkServices::instance().global;

	// List the items to render
	_renderEntries = global->getCurrent()->getLocation()->listRenderEntries();

	// Render all the scene items
	Gfx::RenderEntryArray::iterator element = _renderEntries.begin();
	while (element != _renderEntries.end()) {
		// Draw the current element
		(*element)->render(_gfx);

		// Go for the next one
		element++;
	}
}

void GameWindow::onMouseMove(const Common::Point &pos) {
	Global *global = StarkServices::instance().global;
	_renderEntries = global->getCurrent()->getLocation()->listRenderEntries();

	updateItems();
}

void GameWindow::onClick(const Common::Point &pos) {
	UserInterface *ui = StarkServices::instance().userInterface;

	_actionMenu->close();

	if (_objectUnderCursor) {
		// Possibilities:
		// * Click on something that doesn't take an action
		// * Click on something that takes exactly 1 action.
		// * Click on something that takes more than 1 action (open action menu)
		// * Click in the action menu, which has 0 available actions (TODO)

		int32 defaultAction = ui->itemGetDefaultActionAt(_objectUnderCursor, _objectRelativePosition);
		int16 selectedInventoryItem = _inventory->getSelectedInventoryItem();

		if (defaultAction != -1) {
			ui->itemDoActionAt(_objectUnderCursor, defaultAction, _objectRelativePosition);
		} else if (selectedInventoryItem != -1) {
			if (!ui->itemDoActionAt(_objectUnderCursor, selectedInventoryItem, _objectRelativePosition)) {
				warning("Could not perform action %d on %s", selectedInventoryItem, _objectUnderCursor->getName().c_str());
			}
		} else {
			Resources::ActionArray actions = ui->getStockActionsPossibleForObject(_objectUnderCursor, _objectRelativePosition);
			if (actions.size() == 1) {
				ui->itemDoActionAt(_objectUnderCursor, actions[0], _objectRelativePosition);
			} else if (actions.size() > 1) {
				_actionMenu->open(_objectUnderCursor, _objectRelativePosition);
			}
		}
	} else {
		ui->walkTo(getScreenMousePosition());
	}
}

void GameWindow::updateItems() {
	// Check for game world mouse overs
	UserInterface *ui = StarkServices::instance().userInterface;
	Common::Point pos = getMousePosition();


	_objectUnderCursor = nullptr;

	// Render entries are sorted from the farthest to the camera to the nearest
	// Loop in reverse order
	for (int i = _renderEntries.size() - 1; i >= 0; i--) {
		if (_renderEntries[i]->containsPoint(pos, _objectRelativePosition)) {
			_objectUnderCursor = _renderEntries[i]->getOwner();
			break;
		}
	}

	Resources::ActionArray actionsPossible;
	if (_objectUnderCursor) {
		actionsPossible = ui->getActionsPossibleForObject(_objectUnderCursor, _objectRelativePosition);
	}

	if (actionsPossible.empty()) {
		// Only consider items with runnable scripts
		_objectUnderCursor = nullptr;
	}

	Common::String mouseHint;
	if (_objectUnderCursor) {
		setCursorDependingOnActionsAvailable(actionsPossible);

		mouseHint = ui->getItemTitle(_objectUnderCursor, true, _objectRelativePosition);
	} else {
		// Not an object
		_cursor->setCursorType(Cursor::kPassive);
	}
	_cursor->setMouseHint(mouseHint);
}

void GameWindow::setCursorDependingOnActionsAvailable(Resources::ActionArray actionsAvailable) {
	if (actionsAvailable.empty()) {
		_cursor->setCursorType(Cursor::kPassive);
		return;
	}

	uint32 count = 0;
	Cursor::CursorType cursorType;
	for (uint i = 0; i < actionsAvailable.size(); i++) {
		switch (actionsAvailable[i]) {
			case Resources::PATTable::kActionLook:
				cursorType = Cursor::kEye;
		        count++;
		        break;
			case Resources::PATTable::kActionTalk:
				cursorType = Cursor::kMouth;
		        count++;
		        break;
			case Resources::PATTable::kActionUse:
				cursorType = Cursor::kHand;
		        count++;
		        break;
		}
	}

	if (count == 1) {
		_cursor->setCursorType(cursorType);
	} else {
		_cursor->setCursorType(Cursor::kActive);
	}
}

} // End of namespace Stark
