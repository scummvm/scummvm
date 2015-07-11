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

#include "engines/stark/ui/inventoryinterface.h"

#include "engines/stark/actionmenu.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/knowledgeset.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/pattable.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/userinterface.h"

#include "engines/stark/visual/image.h"

namespace Stark {

InventoryInterface::InventoryInterface(Gfx::Driver *gfx, Cursor *cursor, ActionMenu *actionMenu) :
	Window(gfx, cursor),
	_actionMenu(actionMenu),
	_selectedInventoryItem(-1) {
	StaticProvider *staticProvider = StarkServices::instance().staticProvider;
	_backgroundTexture = staticProvider->getUIItem(StaticProvider::kInventoryBg);

	_position = Common::Rect(526, 315);
	_position.translate(40, 50);
}

void InventoryInterface::open() {
	_visible = true;

	Global *global = StarkServices::instance().global;
	_renderEntries = global->getInventory()->getInventoryRenderEntries();
}

void InventoryInterface::close() {
	_visible = false;
}

void InventoryInterface::setSelectedInventoryItem(uint16 selectedToolId) {
	_selectedInventoryItem = selectedToolId;
}

void InventoryInterface::onRender() {
	_backgroundTexture->getVisual()->get<VisualImageXMG>()->render(Common::Point(0, 0));
	
	Gfx::RenderEntryArray::iterator it = _renderEntries.begin();
	// TODO: Unhardcode positions
	Common::Point pos;
	int width =_backgroundTexture->getVisual()->get<VisualImageXMG>()->getWidth();
	pos.x += 40;
	for (;it != _renderEntries.end(); ++it) {
		(*it)->setPosition(pos);
		(*it)->render(_gfx);

		pos.x += 36;
		if (pos.x > width - 40) {
			pos.x = 20;
			pos.y+= 36;
		}
	}
}

void InventoryInterface::checkObjectAtPos(Common::Point pos, Resources::ItemVisual **item, int16 selectedTool, int16 &possibleTool) {
	UserInterface *ui = StarkServices::instance().userInterface;

	*item = nullptr;
	possibleTool = -1;

	// Check for inventory mouse overs
	// Render entries are sorted from the farthest to the camera to the nearest
	// Loop in reverse order
	for (int i = _renderEntries.size() - 1; i >= 0; i--) {
		Common::Point relativePosition;
		if (_renderEntries[i]->containsPoint(pos, relativePosition)) {
			*item = _renderEntries[i]->getOwner();
			break;
		}
	}

	if (!*item) {
		// No item at specified position
		return;
	}

	if (selectedTool == -1) {
		Resources::ActionArray actionsPossible;
		actionsPossible = ui->getStockActionsPossibleForObject(*item);

		if (actionsPossible.empty()) {
			possibleTool = 1;
		}
	} else {
		if (ui->itemHasAction(*item, selectedTool)) {
			possibleTool = selectedTool;
		}
	}
}

void InventoryInterface::onMouseMove(const Common::Point &pos) {
	Resources::ItemVisual *hoveredItem = nullptr;
	int16 hoveredItemAction = -1;

	checkObjectAtPos(pos, &hoveredItem, _selectedInventoryItem, hoveredItemAction);

	switch (_selectedInventoryItem) {
		case -1:
			_cursor->setCursorType(Cursor::CursorType::kDefault);
			break;
		case 0:
			_cursor->setCursorType(Cursor::CursorType::kHand);
	        break;
		case 1:
			_cursor->setCursorType(Cursor::CursorType::kEye);
	        break;
		case 2:
			_cursor->setCursorType(Cursor::CursorType::kMouth);
	        break;
		default:
			UserInterface *ui = StarkServices::instance().userInterface;
	        VisualImageXMG *cursorImage = ui->getCursorImage(_selectedInventoryItem);
			_cursor->setCursorImage(cursorImage);
			break;
	}
}

void InventoryInterface::onClick(const Common::Point &pos) {
	Resources::ItemVisual *hoveredItem = nullptr;
	int16 hoveredItemAction = -1;

	checkObjectAtPos(pos, &hoveredItem, _selectedInventoryItem, hoveredItemAction);

	if (hoveredItem) {
		// TODO: Should only open the action menu when there are several actions possible
		_actionMenu->open(hoveredItem, Common::Point());
	}
}

} // End of namespace Stark
