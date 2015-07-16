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

#include "engines/stark/ui/inventorywindow.h"

#include "engines/stark/ui/actionmenu.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/knowledgeset.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/pattable.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/gameinterface.h"

#include "engines/stark/visual/image.h"

namespace Stark {

InventoryWindow::InventoryWindow(Gfx::Driver *gfx, Cursor *cursor, ActionMenu *actionMenu) :
	Window(gfx, cursor),
	_actionMenu(actionMenu),
	_selectedInventoryItem(-1) {
	// The window has the same size as the game window
	_position = Common::Rect(Gfx::Driver::kGameViewportWidth, Gfx::Driver::kGameViewportHeight);
	_position.translate(0, Gfx::Driver::kTopBorderHeight);

	Resources::Anim *backgroundAnim = StarkStaticProvider->getUIItem(StaticProvider::kInventoryBg);
	_backgroundTexture = backgroundAnim->getVisual()->get<VisualImageXMG>();

	// Center the background in the window
	_backgroundRect = Common::Rect(_backgroundTexture->getWidth(), _backgroundTexture->getHeight());
	_backgroundRect.translate((_position.width() - _backgroundRect.width()) / 2,
			(_position.height() - _backgroundRect.height()) / 2);
}

void InventoryWindow::open() {
	_visible = true;
}

void InventoryWindow::close() {
	_visible = false;
}

void InventoryWindow::setSelectedInventoryItem(uint16 selectedInventoryItem) {
	_selectedInventoryItem = selectedInventoryItem;
}

int16 InventoryWindow::getSelectedInventoryItem() const {
	return _selectedInventoryItem;
}

Common::Rect InventoryWindow::getSlotRect(uint32 slot) const {
	Common::Rect rect = Common::Rect(64, 64);
	rect.translate(
			96 * (slot % 5) + _backgroundRect.left + 24,
			96 * (slot / 5) + _backgroundRect.left + 8); // The original uses left here as well
	return rect;
}

Common::Rect InventoryWindow::getItemRect(uint32 slot, VisualImageXMG *image) const {
	Common::Rect rect = getSlotRect(slot);

	// Center the image in the inventory slot
	rect.translate((rect.width() - image->getWidth()) / 2,
	              (rect.height() - image->getHeight()) / 2);

	return rect;
}

void InventoryWindow::onRender() {
	_renderEntries = StarkGlobal->getInventory()->getInventoryRenderEntries();

	_backgroundTexture->render(Common::Point(_backgroundRect.left, _backgroundRect.top), false);
	
	for (uint i = 0; i < _renderEntries.size(); i++) {
		VisualImageXMG *image = _renderEntries[i]->getImage();

		// Get the item rect
		Common::Rect pos = getItemRect(i, image);

		image->render(Common::Point(pos.left, pos.top), false);
	}
}

void InventoryWindow::checkObjectAtPos(Common::Point pos, Resources::ItemVisual **item, int16 selectedInventoryItem, int16 &singlePossibleAction) {
	*item = nullptr;
	singlePossibleAction = -1;

	// Check for inventory mouse overs
	for (uint i = 0; i < _renderEntries.size(); i++) {
		VisualImageXMG *image = _renderEntries[i]->getImage();
		Common::Rect itemRect = getItemRect(i, image);

		if (itemRect.contains(pos)) {
			*item = _renderEntries[i]->getOwner();
			break;
		}
	}

	if (!*item) {
		// No item at specified position
		return;
	}

	if (selectedInventoryItem == -1) {
		Resources::ActionArray actionsPossible;
		actionsPossible = StarkGameInterface->getStockActionsPossibleForObject(*item);

		if (actionsPossible.empty()) {
			// The item can still be taken
			singlePossibleAction = Resources::PATTable::kActionUse;
		}
	} else {
		if (StarkGameInterface->itemHasAction(*item, selectedInventoryItem)) {
			singlePossibleAction = selectedInventoryItem;
		}
	}
}

void InventoryWindow::onMouseMove(const Common::Point &pos) {
	Resources::ItemVisual *hoveredItem = nullptr;
	int16 hoveredItemAction = -1;

	checkObjectAtPos(pos, &hoveredItem, _selectedInventoryItem, hoveredItemAction);

	if (_selectedInventoryItem == -1) {
		if (hoveredItem) {
			_cursor->setCursorType(Cursor::kActive);
		} else {
			_cursor->setCursorType(Cursor::kDefault);
		}
	} else {
        VisualImageXMG *cursorImage = StarkGameInterface->getCursorImage(_selectedInventoryItem);
		_cursor->setCursorImage(cursorImage);
	}
}

void InventoryWindow::onClick(const Common::Point &pos) {
	_actionMenu->close();

	Resources::ItemVisual *clickedItem = nullptr;
	int16 clickedItemAction = -1;
	checkObjectAtPos(pos, &clickedItem, _selectedInventoryItem, clickedItemAction);

	if (clickedItem) {
		// An item was clicked
		if (clickedItemAction != -1) {
			// A single action is possible
			if (clickedItemAction == Resources::PATTable::kActionUse) {
				setSelectedInventoryItem(clickedItem->getIndex());
			} else {
				StarkGameInterface->itemDoAction(clickedItem, clickedItemAction);
			}
		} else {
			// Multiple actions are possible
			if (_selectedInventoryItem == -1) {
				_actionMenu->open(clickedItem, Common::Point());
			}
		}
	} else {
		// Nothing was under the mouse cursor, close the inventory
		close();
	}
}

void InventoryWindow::onRightClick(const Common::Point &pos) {
	if (_selectedInventoryItem == -1) {
		close();
	} else {
		setSelectedInventoryItem(-1);
	}
}

} // End of namespace Stark
