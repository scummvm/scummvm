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

#include "engines/stark/ui/world/inventorywindow.h"

#include "engines/stark/ui/cursor.h"
#include "engines/stark/ui/world/actionmenu.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/resources/knowledgeset.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/pattable.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/gameinterface.h"

#include "engines/stark/visual/image.h"

namespace Stark {

static const int kAutoCloseSuspended = -1;
static const int kAutoCloseDisabled  = -2;
static const int kAutoCloseDelay     = 200;

InventoryWindow::InventoryWindow(Gfx::Driver *gfx, Cursor *cursor, ActionMenu *actionMenu) :
		Window(gfx, cursor),
	_actionMenu(actionMenu),
	_firstVisibleSlot(0),
	_selectedInventoryItem(-1),
	_autoCloseTimeRemaining(kAutoCloseDisabled) {
	// The window has the same size as the game window
	_position = Common::Rect(Gfx::Driver::kGameViewportWidth, Gfx::Driver::kGameViewportHeight);
	_position.translate(0, Gfx::Driver::kTopBorderHeight);

	_backgroundTexture = StarkStaticProvider->getUIImage(StaticProvider::kInventoryBg);

	// Center the background in the window
	_backgroundRect = Common::Rect(_backgroundTexture->getWidth(), _backgroundTexture->getHeight());
	_backgroundRect.translate((_position.width() - _backgroundRect.width()) / 2,
			(_position.height() - _backgroundRect.height()) / 2);

	_scrollUpArrowImage = StarkStaticProvider->getUIElement(StaticProvider::kInventoryScrollUpArrow);
	_scrollDownArrowImage = StarkStaticProvider->getUIElement(StaticProvider::kInventoryScrollDownArrow);

	_scrollUpArrowRect = Common::Rect(_scrollUpArrowImage->getWidth(), _scrollUpArrowImage->getHeight());
	_scrollUpArrowRect.translate(_backgroundRect.right - _scrollUpArrowRect.width(),
	                             _backgroundRect.top + 2);
	_scrollDownArrowRect = Common::Rect(_scrollDownArrowImage->getWidth(), _scrollDownArrowImage->getHeight());
	_scrollDownArrowRect.translate(_backgroundRect.right - _scrollDownArrowRect.width(),
	                               _backgroundRect.bottom - _scrollDownArrowRect.height() - 2);
}

void InventoryWindow::open() {
	if (!_visible) {
		_actionMenu->close();
	}

	_visible = true;

	// The user needs to move the mouse over the background at least once
	// before autoclose is enabled.
	_autoCloseTimeRemaining = kAutoCloseDisabled;
}

void InventoryWindow::close() {
	if (_visible) {
		_actionMenu->close();
	}

	_visible = false;
}

void InventoryWindow::setSelectedInventoryItem(int16 selectedInventoryItem) {
	// The first 4 elements are UI elements (Eye, Mouth, Hand, ...)
	// Scripts pass 0 when they want to clear the selected inventory item
	if (selectedInventoryItem < 4) {
		_selectedInventoryItem = -1;
	} else {
		_selectedInventoryItem = selectedInventoryItem;
	}
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
	Common::Rect rect = getSlotRect(slot % _visibleSlotsCount);

	// Center the image in the inventory slot
	rect.translate((rect.width() - image->getWidth()) / 2,
	              (rect.height() - image->getHeight()) / 2);

	return rect;
}

void InventoryWindow::onRender() {
	_renderEntries = StarkGlobal->getInventory()->getInventoryRenderEntries();

	_backgroundTexture->render(Common::Point(_backgroundRect.left, _backgroundRect.top), false);
	drawScrollArrows();

	for (uint i = _firstVisibleSlot; i < _renderEntries.size() && isSlotVisible(i); i++) {
		VisualImageXMG *image = _renderEntries[i]->getImage();

		// Get the item rect
		Common::Rect pos = getItemRect(i, image);

		image->render(Common::Point(pos.left, pos.top), false);
	}
}

void InventoryWindow::drawScrollArrows() const {
	if (canScrollUp()) {
		_scrollUpArrowImage->render(Common::Point(_scrollUpArrowRect.left, _scrollUpArrowRect.top), false);
	}
	if (canScrollDown()) {
		_scrollDownArrowImage->render(Common::Point(_scrollDownArrowRect.left, _scrollDownArrowRect.top), false);
	}
}

void InventoryWindow::checkObjectAtPos(Common::Point pos, Resources::ItemVisual **item, int16 selectedInventoryItem, int16 &singlePossibleAction) {
	*item = nullptr;
	singlePossibleAction = -1;

	// Check for inventory mouse overs
	for (uint i = _firstVisibleSlot; i < _renderEntries.size() && isSlotVisible(i); i++) {
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
		actionsPossible = StarkGameInterface->listStockActionsPossibleForObject(*item);

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
		} else if ((canScrollDown() && _scrollDownArrowRect.contains(pos))
		           || (canScrollUp() && _scrollUpArrowRect.contains(pos))) {
			_cursor->setCursorType(Cursor::kActive);
			_cursor->setItemActive(false);
		} else {
			_cursor->setCursorType(Cursor::kDefault);
		}
		_cursor->setItemActive(false);
	} else {
		VisualImageXMG *cursorImage = StarkGameInterface->getCursorImage(_selectedInventoryItem);
		_cursor->setCursorImage(cursorImage);
		_cursor->setItemActive(hoveredItemAction == _selectedInventoryItem);
	}

	if (hoveredItem) {
		Common::String hint = StarkGameInterface->getItemTitle(hoveredItem);
		_cursor->setMouseHint(hint);
	} else {
		_cursor->setMouseHint("");
	}

	if (!_backgroundRect.contains(pos)) {
		if (_autoCloseTimeRemaining == kAutoCloseSuspended) {
			_autoCloseTimeRemaining = kAutoCloseDelay;
		}
	} else {
		_autoCloseTimeRemaining = kAutoCloseSuspended;
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
	} else if (_scrollDownArrowRect.contains(pos)) {
		if (canScrollDown()) {
			scrollDown();
		}
	} else if (_scrollUpArrowRect.contains(pos)) {
		if (canScrollUp()) {
			scrollUp();
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

void InventoryWindow::reset() {
	_renderEntries.clear();
}

bool InventoryWindow::isSlotVisible(uint32 slot) const {
	return slot < _firstVisibleSlot + _visibleSlotsCount;
}

bool InventoryWindow::canScrollDown() const {
	return _renderEntries.size() - _firstVisibleSlot > _visibleSlotsCount;
}

bool InventoryWindow::canScrollUp() const {
	return _firstVisibleSlot > 0;
}

void InventoryWindow::scrollDown() {
	if (canScrollDown()) {
		_firstVisibleSlot += _visibleSlotsCount;
	}
}

void InventoryWindow::scrollUp() {
	if (canScrollUp()) {
		_firstVisibleSlot -= _visibleSlotsCount;
	}
}

void InventoryWindow::onGameLoop() {
	if (_autoCloseTimeRemaining >= 0 && !_actionMenu->isVisible()) {
		_autoCloseTimeRemaining -= StarkGlobal->getMillisecondsPerGameloop();

		if (_autoCloseTimeRemaining <= 0) {
			_autoCloseTimeRemaining = kAutoCloseSuspended;
			close();
		}
	}
}
} // End of namespace Stark
