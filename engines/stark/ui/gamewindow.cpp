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
	UserInterface *ui = StarkServices::instance().userInterface;
	Global *global = StarkServices::instance().global;

	_renderEntries = global->getCurrent()->getLocation()->listRenderEntries();

	int16 selectedInventoryItem = _inventory->getSelectedInventoryItem();
	int16 singlePossibleAction = -1;

	checkObjectAtPos(pos, selectedInventoryItem, singlePossibleAction);

	Common::String mouseHint;

	if (selectedInventoryItem != -1) {
		VisualImageXMG *cursorImage = ui->getCursorImage(selectedInventoryItem);
		_cursor->setCursorImage(cursorImage);
	} else if (_objectUnderCursor) {
		switch (singlePossibleAction) {
			case -1:
				_cursor->setCursorType(Cursor::kActive);
				break;
			case Resources::PATTable::kActionLook:
				_cursor->setCursorType(Cursor::kEye);
				break;
			case Resources::PATTable::kActionTalk:
				_cursor->setCursorType(Cursor::kMouth);
				break;
			case Resources::PATTable::kActionUse:
				_cursor->setCursorType(Cursor::kHand);
				break;
			default:
				VisualImageXMG *cursorImage = ui->getCursorImage(singlePossibleAction);
				_cursor->setCursorImage(cursorImage);
				break;
		}

		mouseHint = ui->getItemTitle(_objectUnderCursor, true, _objectRelativePosition);
	} else {
		// Not an object
		_cursor->setCursorType(Cursor::kDefault);
	}
	_cursor->setMouseHint(mouseHint);
}

void GameWindow::onClick(const Common::Point &pos) {
	UserInterface *ui = StarkServices::instance().userInterface;

	_actionMenu->close();

	int16 selectedInventoryItem = _inventory->getSelectedInventoryItem();
	int16 singlePossibleAction = -1;

	checkObjectAtPos(pos, selectedInventoryItem, singlePossibleAction);

	if (_objectUnderCursor) {
		if (singlePossibleAction != -1) {
			ui->itemDoActionAt(_objectUnderCursor, singlePossibleAction, _objectRelativePosition);
		} else if (selectedInventoryItem == -1) {
			_actionMenu->open(_objectUnderCursor, _objectRelativePosition);
		}
	} else {
		ui->walkTo(getScreenMousePosition());
	}
}

void GameWindow::checkObjectAtPos(Common::Point pos, int16 selectedTool, int16 &possibleTool) {
	UserInterface *ui = StarkServices::instance().userInterface;

	_objectUnderCursor = nullptr;
	possibleTool = -1;

	// Render entries are sorted from the farthest to the camera to the nearest
	// Loop in reverse order
	for (int i = _renderEntries.size() - 1; i >= 0; i--) {
		if (_renderEntries[i]->containsPoint(pos, _objectRelativePosition)) {
			_objectUnderCursor = _renderEntries[i]->getOwner();
			break;
		}
	}

	if (!_objectUnderCursor || !ui->itemHasActionAt(_objectUnderCursor, _objectRelativePosition, -1)) {
		// Only consider items with runnable scripts
		_objectUnderCursor = nullptr;
		return;
	}

	int32 defaultAction = ui->itemGetDefaultActionAt(_objectUnderCursor, _objectRelativePosition);
	if (defaultAction != -1) {
		// Use the default action if there is one
		possibleTool = defaultAction;
	} else if (selectedTool != -1) {
		// Use the selected inventory item if there is one
		if (ui->itemHasActionAt(_objectUnderCursor, _objectRelativePosition, selectedTool)) {
			possibleTool = selectedTool;
		}
	} else {
		// Otherwise, use stock actions
		Resources::ActionArray actionsPossible = ui->getStockActionsPossibleForObject(_objectUnderCursor, _objectRelativePosition);

		if (actionsPossible.size() == 1) {
			possibleTool = actionsPossible[0];
		}
	}
}

} // End of namespace Stark
