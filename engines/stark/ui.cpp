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

#include "engines/stark/actionmenu.h"
#include "engines/stark/cursor.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/renderentry.h"

#include "engines/stark/resources/object.h"
#include "engines/stark/resources/item.h"

#include "engines/stark/services/fmvplayer.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/userinterface.h"

#include "engines/stark/ui/topmenu.h"
#include "engines/stark/ui/dialoginterface.h"
#include "engines/stark/ui/inventoryinterface.h"

namespace Stark {

UI::UI(Gfx::Driver *gfx, Cursor *cursor) :
	_gfx(gfx),
	_cursor(cursor),
	_objectUnderCursor(nullptr),
	_hasClicked(false),
	_topMenu(nullptr),
	_dialogInterface(nullptr),
	_inventoryInterface(nullptr),
	_selectedInventoryItem(-1),
	_exitGame(false),
	_fmvPlayer(nullptr),
	_actionMenu(nullptr)
	{
}

UI::~UI() {
	delete _actionMenu;
	delete _topMenu;
	delete _dialogInterface;
	delete _inventoryInterface;
	delete _fmvPlayer;
}

void UI::init() {
	_topMenu = new TopMenu();
	_dialogInterface = new DialogInterface();
	_inventoryInterface = new InventoryInterface(_gfx, _cursor);
	_fmvPlayer = new FMVPlayer();
	_actionMenu = new ActionMenu(_gfx, _cursor);
}

void UI::update(Gfx::RenderEntryArray renderEntries) {
	Common::Point pos = _cursor->getMousePosition();

	if (_actionMenu->isVisible() && _actionMenu->isMouseInside()) {
		_actionMenu->handleMouseMove();
		return;
	}

	// Check for inventory to avoid mouse-overs from the world poking through.
	if (_inventoryInterface->isVisible() && _inventoryInterface->isMouseInside()) {
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

	pos -= Common::Point(0, Gfx::Driver::kTopBorderHeight);

	// Check for game world mouse overs
	UserInterface *ui = StarkServices::instance().userInterface;

	_objectUnderCursor = nullptr;

	// Render entries are sorted from the farthest to the camera to the nearest
	// Loop in reverse order
	Common::Point relativePosition;
	for (int i = renderEntries.size() - 1; i >= 0; i--) {
		if (renderEntries[i]->containsPoint(pos, relativePosition)) {
			_objectUnderCursor = renderEntries[i]->getOwner();
			break;
		}
	}

	Common::String mouseHint;
	if (_objectUnderCursor) {
		Resources::ActionArray actionsPossible = ui->getActionsPossibleForObject(_objectUnderCursor, relativePosition);
		setCursorDependingOnActionsAvailable(actionsPossible);

		mouseHint = ui->getItemTitle(_objectUnderCursor, true, relativePosition);
	} else {
		// Not an object
		_cursor->setCursorType(Cursor::kPassive);
	}
	_cursor->setMouseHint(_selectedInventoryItemText + mouseHint);
}

void UI::setCursorDependingOnActionsAvailable(Resources::ActionArray actionsAvailable) {
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

void UI::handleClick() {
	UserInterface *ui = StarkServices::instance().userInterface;

	if (_actionMenu->isVisible()) {
		if (_actionMenu->isMouseInside()) {
			_actionMenu->handleClick();
		} else {
			_actionMenu->close();
		}
	} else if (_objectUnderCursor) {
		Common::Point pos = _cursor->getMousePosition();
		pos -= Common::Point(0, Gfx::Driver::kTopBorderHeight);

		// Possibilites:
		// * Click on something that doesn't take an action
		// * Click on something that takes exactly 1 action.
		// * Click on something that takes more than 1 action (open action menu)
		// * Click in the action menu, which has 0 available actions (TODO)
		if (_selectedInventoryItem != -1) {
			if (!ui->itemDoActionAt(_objectUnderCursor, _selectedInventoryItem, pos)) {
				warning("Could not perform action %d on %s", _selectedInventoryItem, _objectUnderCursor->getName().c_str());
			}
		} else {
			Resources::ActionArray actions = ui->getStockActionsPossibleForObject(_objectUnderCursor, pos);
			if (actions.size() == 1) {
				for (uint i = 0; i < actions.size(); i++) {
					if (actions[i] == Resources::PATTable::kActionLook) {
						ui->itemDoActionAt(_objectUnderCursor, Resources::PATTable::kActionLook, pos);
						break;
					} else if (actions[i] == Resources::PATTable::kActionTalk) {
						ui->itemDoActionAt(_objectUnderCursor, Resources::PATTable::kActionTalk, pos);
						break;
					} else if (actions[i] == Resources::PATTable::kActionUse) {
						ui->itemDoActionAt(_objectUnderCursor, Resources::PATTable::kActionUse, pos);
						break;
					}
				}
			} else if (actions.size() > 1) {
				_actionMenu->open(_objectUnderCursor, pos);
			}
		}
	} else {
		ui->walkTo(g_system->getEventManager()->getMousePos());

//		// Assume all inventory objects need action menu.
//		if (isInventoryObject(item)) {
//			return false;
//		}

//		{
//			if (_selectedInventoryItem != -1) {
//				_inventoryInterface->update();
//				_selectedInventoryItem = -1;
//				_selectedInventoryItemText = "";
//			}
//		}
	}

	// Check this before handling the menu clicks, otherwise it closes again on the same event.
	if (_inventoryInterface->isVisible() && !_inventoryInterface->isMouseInside()) {
		_inventoryInterface->close();
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
	// Make the inventory update it's contents.
	_inventoryInterface->open();
}

void UI::notifyFMVRequest(const Common::String &name) {
	_fmvPlayer->play(name);
}

void UI::notifySelectedInventoryItem(Resources::Item *selectedItem) {
	_selectedInventoryItem = selectedItem->getIndex();
	_selectedInventoryItemText = selectedItem->getName() + " -> ";
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

	// Can't handle clicks before this point, since we need to have updated the mouse-over state to include the UI.
	if (_hasClicked) {
		handleClick();
	}

	// TODO: Unhardcode
	if (_cursor->getMousePosition().y < 40) {
		_topMenu->render();
	}

	_inventoryInterface->render();

	_dialogInterface->render();
	_actionMenu->render();
}

} // End of namespace Stark

