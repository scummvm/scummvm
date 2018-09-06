/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#include "mutationofjb/widgets/inventorywidget.h"
#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/gamescreen.h"
#include "mutationofjb/inventory.h"

#include "common/str.h"
#include "common/rect.h"
#include "common/util.h"
#include "common/events.h"

#include "graphics/managed_surface.h"

namespace MutationOfJB {

enum {
	INVENTORY_START_X = 88,
	INVENTORY_START_Y = 149,
	INVENTORY_ITEM_WIDTH = 34,
	INVENTORY_ITEM_HEIGHT = 33,
	INVENTORY_ITEMS_PER_LINE = 8,
	INVENTORY_ITEMS_LINES = 5
};

InventoryWidget::InventoryWidget(GuiScreen &gui, const Common::Array<Graphics::Surface> &inventorySurfaces) :
	Widget(gui, Common::Rect(INVENTORY_START_X, INVENTORY_START_Y, INVENTORY_START_X + Inventory::VISIBLE_ITEMS * INVENTORY_ITEM_WIDTH, INVENTORY_START_Y + INVENTORY_ITEM_HEIGHT)),
	_surfaces(inventorySurfaces),
	_callback(nullptr),
	_hoveredItemPos(-1) {}

void InventoryWidget::drawInventoryItem(Graphics::ManagedSurface &surface, const Common::String &item, int pos) {
	const int index = _gui.getGame().getAssets().getInventoryItemDefList().findItemIndex(item);
	if (index == -1) {
		return;
	}

	const int surfaceNo = index / (INVENTORY_ITEMS_LINES * INVENTORY_ITEMS_PER_LINE);
	const int indexInSurface = index % (INVENTORY_ITEMS_LINES * INVENTORY_ITEMS_PER_LINE);
	const int itemX = indexInSurface % INVENTORY_ITEMS_PER_LINE;
	const int itemY = indexInSurface / INVENTORY_ITEMS_PER_LINE;

	Common::Point destStartPos(INVENTORY_START_X + pos * INVENTORY_ITEM_WIDTH, INVENTORY_START_Y);
	Common::Rect sourceRect(itemX * INVENTORY_ITEM_WIDTH, itemY * INVENTORY_ITEM_HEIGHT, (itemX + 1) * INVENTORY_ITEM_WIDTH, (itemY + 1) * INVENTORY_ITEM_HEIGHT);
	surface.blitFrom(_surfaces[surfaceNo], sourceRect, destStartPos);
}

void InventoryWidget::draw(Graphics::ManagedSurface &surface) {
	Inventory &inventory = _gui.getGame().getGameData().getInventory();
	const Inventory::Items &items = inventory.getItems();
	surface.fillRect(_area, 0x00);
	for (Inventory::Items::size_type i = 0; i < MIN<Inventory::Items::size_type>(items.size(), Inventory::VISIBLE_ITEMS); ++i) {
		drawInventoryItem(surface, items[i], i);
	}
}

void InventoryWidget::handleEvent(const Common::Event &event) {
	if (!_callback)
		return;

	Inventory &inventory = _gui.getGame().getGameData().getInventory();
	const int numItems = inventory.getItems().size();

	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN: {
		const int16 x = event.mouse.x;
		const int16 y = event.mouse.y;
		if (_area.contains(x, y)) {
			int itemPos = (x - INVENTORY_START_X) / INVENTORY_ITEM_WIDTH;
			if (itemPos < numItems) {
				_callback->onInventoryItemClicked(this, itemPos);
			}
		}
		break;
	}
	case Common::EVENT_MOUSEMOVE: {
		const int16 x = event.mouse.x;
		const int16 y = event.mouse.y;

		int newHoveredItemPos = -1;
		if (_area.contains(x, y)) {
			int itemPos = (x - INVENTORY_START_X) / INVENTORY_ITEM_WIDTH;
			if (itemPos < numItems) {
				newHoveredItemPos = itemPos;

				if (_hoveredItemPos != newHoveredItemPos) {
					_callback->onInventoryItemHovered(this, itemPos);
				}
			}
		}

		if (newHoveredItemPos == -1 && _hoveredItemPos != -1) {
			_callback->onInventoryItemHovered(this, -1);
		}

		_hoveredItemPos = newHoveredItemPos;
		break;
	}
	default:
		break;
	}
}

}
