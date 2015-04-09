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
	_actionMenu(actionMenu) {
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

void InventoryInterface::onMouseMove(const Common::Point &pos) {
	updateItems();
}

void InventoryInterface::onClick(const Common::Point &pos) {
	if (_objectUnderCursor) {
		// Possibilites:
		// * Click on something that doesn't take an action
		// * Click on something that takes exactly 1 action.
		// * Click on something that takes more than 1 action (open action menu)
		// * Click in the action menu, which has 0 available actions (TODO)
//			if (_selectedInventoryItem != -1) {
//				if (!ui->itemDoActionAt(_objectUnderCursor, _selectedInventoryItem, pos)) {
//					warning("Could not perform action %d on %s", _selectedInventoryItem, _objectUnderCursor->getName().c_str());
//				}
//			} else
		{
			// TODO: Is it safe to always open the action menu?
			_actionMenu->open(_objectUnderCursor, _objectRelativePosition);
		}
	}
}

} // End of namespace Stark
