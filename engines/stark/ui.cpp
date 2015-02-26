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

#include "engines/stark/ui.h"

#include "engines/stark/cursor.h"

#include "engines/stark/gfx/renderentry.h"

#include "engines/stark/resources/object.h"
#include "engines/stark/resources/item.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/userinterface.h"

#include "engines/stark/ui/topmenu.h"
#include "engines/stark/ui/dialoginterface.h"
#include "engines/stark/ui/inventoryinterface.h"

namespace Stark {

UI::UI(Gfx::Driver *gfx, Cursor *cursor) :
	_gfx(gfx),
	_cursor(cursor),
	_currentObject(nullptr),
	_objectUnderCursor(nullptr),
	_hasClicked(false),
	_inventoryOpen(false),
	_topMenu(nullptr),
	_dialogInterface(nullptr),
	_inventoryInterface(nullptr),
	_selectedInventoryItem(-1),
	_exitGame(false)
	{
}

UI::~UI() {
	delete _topMenu;
	delete _dialogInterface;
	delete _inventoryInterface;
}

void UI::init() {
	_topMenu = new TopMenu();
	_dialogInterface = new DialogInterface();
	_inventoryInterface = new InventoryInterface();
}

void UI::update(Gfx::RenderEntryArray renderEntries, bool keepExisting) {
	Common::Point pos = _cursor->getMousePosition();

	// Check for inventory to avoid mouse-overs from the world poking through.
	if (_inventoryOpen && _inventoryInterface->containsPoint(pos) && keepExisting == false) {
		renderEntries = _inventoryInterface->getRenderEntries();
	}

	// Check for UI mouse overs
	if (_topMenu->containsPoint(pos)) {
		_cursor->setCursorType(Cursor::kActive);
		_cursor->setMouseHint(_selectedInventoryItemText + _topMenu->getMouseHintAtPosition(pos));
		return;
	}

	if (_dialogInterface->containsPoint(pos)) {
		_dialogInterface->handleMouseOver(pos);
		return;
	}

	// Check for game world mouse overs
	UserInterface *ui = StarkServices::instance().userInterface;
	Gfx::RenderEntry *currentEntry = ui->getEntryAtPosition(pos, renderEntries);
	Resources::Object *object = ui->getObjectForRenderEntryAtPosition(pos, currentEntry);
	// So that we can run update multiple times, without resetting (i.e. after drawing the action menu)
	if (!object && keepExisting) {
		return;
	} else {
		// Subsequent runs ignore sort order of items drawn earlier.
		_objectUnderCursor = object;
	}
	Common::String mouseHint = ui->getMouseHintForObject(_objectUnderCursor);

	if (_objectUnderCursor) {
		int actionsPossible = ui->getActionsPossibleForObject(_objectUnderCursor);
		setCursorDependingOnActionsAvailable(actionsPossible);
	} else {
		// Not an object
		_cursor->setCursorType(Cursor::kPassive);
	}
	_cursor->setMouseHint(_selectedInventoryItemText + mouseHint);
}

void UI::setCursorDependingOnActionsAvailable(int actionsAvailable) {
	bool moreThanOneActionPossible = false;
	switch (actionsAvailable) {
		case UserInterface::kActionLookPossible:
			_cursor->setCursorType(Cursor::kEye);
			break;
		case UserInterface::kActionTalkPossible:
			_cursor->setCursorType(Cursor::kMouth);
			break;
		case UserInterface::kActionUsePossible:
			_cursor->setCursorType(Cursor::kHand);
			break;
		case UserInterface::kActionExitPossible:
			_cursor->setCursorType(Cursor::kDefault); // TODO
			break;
		default:
			if (actionsAvailable != 0) {
				_cursor->setCursorType(Cursor::kPassive);
				moreThanOneActionPossible = true;
			}
			break;
	}
	if (moreThanOneActionPossible) {
		_cursor->setCursorType(Cursor::kActive);
	}
}

void UI::handleClick() {
	if (_objectUnderCursor) {
		UserInterface *ui = StarkServices::instance().userInterface;
		if (!ui->performActionOnObject(_objectUnderCursor, _currentObject, _selectedInventoryItem)) {
			_currentObject = _objectUnderCursor;
			ui->activateActionMenuOn(_cursor->getMousePosition(), _currentObject);
		// This currently potentially allows for click-through
		} else {
			if (ui->isActionMenuOpen()) {
				// If the click resulted in a multi-action possibility, then it was outside the action menu.
				ui->deactivateActionMenu();
				_currentObject = nullptr;
				// If we were in the action menu, then retain the selected item.
			} else if (_selectedInventoryItem != -1) {
				_inventoryInterface->update();
				_selectedInventoryItem = -1;
				_selectedInventoryItemText = "";
			}
		}
	}
	// Check this before handling the menu clicks, otherwise it closes again on the same event.
	if (_inventoryOpen && !_inventoryInterface->containsPoint(_cursor->getMousePosition())) {
		_inventoryOpen = false;
	}
	if (_topMenu->containsPoint(_cursor->getMousePosition())) {
		_topMenu->handleClick(_cursor->getMousePosition());
	}
	if (_dialogInterface->containsPoint(_cursor->getMousePosition())) {
		_dialogInterface->handleClick(_cursor->getMousePosition());
	}
	_hasClicked = false;
}

void UI::notifyClick() {
	_hasClicked = true;
}

void UI::notifySubtitle(const Common::String &subtitle) {
	_dialogInterface->notifySubtitle(subtitle);
}

void UI::notifyDialogOptions(const Common::StringArray &options) {
	_dialogInterface->notifyDialogOptions(options);
}

void UI::notifyShouldOpenInventory() {
	_inventoryOpen = true;
	// Make the inventory update it's contents.
	_inventoryInterface->update();
}

void UI::notifySelectedInventoryItem(Resources::Object *selectedItem) {
	_selectedInventoryItem = selectedItem->findParent<Resources::ItemSub2>()->getIndex();
	_selectedInventoryItemText = selectedItem->findParent<Resources::ItemSub2>()->getName() + " -> ";
}

void UI::render() {
	Common::Point pos = _cursor->getMousePosition();
	UserInterface *ui = StarkServices::instance().userInterface;
	update(ui->getRenderEntries(), true);
	// Can't handle clicks before this point, since we need to have updated the mouse-over state to include the UI.
	if (_hasClicked) {
		handleClick();
	}

	// TODO: Unhardcode
	if (_cursor->getMousePosition().y < 40) {
		_topMenu->render();
	}

	if (_inventoryOpen) {
		_inventoryInterface->render();
	}

	_dialogInterface->render();
	ui->render();
}

} // End of namespace Stark

