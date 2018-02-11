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

#include "engines/stark/ui/world/actionmenu.h"

#include "engines/stark/ui/cursor.h"
#include "engines/stark/ui/world/inventorywindow.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/knowledgeset.h"
#include "engines/stark/resources/level.h"
#include "engines/stark/resources/pattable.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/gameinterface.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/global.h"

#include "engines/stark/scene.h"

#include "engines/stark/visual/image.h"

namespace Stark {

ActionMenu::ActionMenu(Gfx::Driver *gfx, Cursor *cursor) :
		Window(gfx, cursor) {
	_background = StarkStaticProvider->getUIElement(StaticProvider::kActionMenuBg);

	_unscaled = true;
	_item = nullptr;

	_buttons[kActionHand].action = Resources::PATTable::kActionUse;
	_buttons[kActionHand].rect = Common::Rect(90, 15, 126, 63);
	_buttons[kActionEye].action = Resources::PATTable::kActionLook;
	_buttons[kActionEye].rect = Common::Rect(5, 77, 51, 110);
	_buttons[kActionMouth].action = Resources::PATTable::kActionTalk;
	_buttons[kActionMouth].rect = Common::Rect(42, 35, 83, 74);

	clearActions();
}

ActionMenu::~ActionMenu() {
}

void ActionMenu::open(Resources::ItemVisual *item, const Common::Point &itemRelativePos) {
	_visible = true;

	Common::Point screenMousePos = _cursor->getMousePosition(true);

	// TODO: tidy up logic for preventing actionMenu overlapping viewport
	Common::Rect viewportPosition = Common::Rect(_gfx->scaleWidthOriginalToCurrent(Gfx::Driver::kGameViewportWidth),
						     _gfx->scaleHeightOriginalToCurrent(Gfx::Driver::kGameViewportHeight));

	viewportPosition.translate(0, _gfx->scaleHeightOriginalToCurrent(Gfx::Driver::kTopBorderHeight));

	_position = Common::Rect::center(screenMousePos.x, screenMousePos.y, 160, 111);

	if (_position.top < 0) {
	  _position.translate(0, 0 - _position.top);
	}

	if (_position.left < 0) {
	  _position.translate(0 - _position.left, 0);
	}

	if (_position.bottom > viewportPosition.bottom) {
	  _position.translate(0, viewportPosition.bottom - _position.bottom);
	}

	if (_position.right > viewportPosition.right) {
	  _position.translate(viewportPosition.right - _position.right, 0);
	}

	_itemRelativePos = itemRelativePos;
	_item = item;
	_fromInventory = item->getSubType() == Resources::Item::kItemInventory;

	clearActions();

	Resources::ActionArray possible;
	if (_fromInventory) {
		possible = StarkGameInterface->listActionsPossibleForObject(_item);
	} else {
		possible = StarkGameInterface->listActionsPossibleForObjectAt(_item, _itemRelativePos);
	}

	for (uint i = 0; i < possible.size(); i++) {
		enableAction(possible[i]);
	}

	if (_fromInventory) {
		// All inventory items can be picked up
		enableAction(Resources::PATTable::kActionUse);
	}
}

void ActionMenu::close() {
	_visible = false;
	_item = nullptr;
}

void ActionMenu::onRender() {
	Common::Point mousePos = getRelativeMousePosition();

	_background->render(Common::Point(0, 0), false);

	for (uint i = 0; i < ARRAYSIZE(_buttons); i++) {
		if (_buttons[i].enabled) {
			bool active = _buttons[i].rect.contains(mousePos);
			VisualImageXMG *visual = StarkGameInterface->getActionImage(_buttons[i].action, active);
			visual->render(Common::Point(_buttons[i].rect.left, _buttons[i].rect.top), false);
		}
	}
}

void ActionMenu::clearActions() {
	for (uint i = 0; i < ARRAYSIZE(_buttons); i++) {
		_buttons[i].enabled = false;
	}
}

void ActionMenu::enableAction(uint32 action) {
	for (uint j = 0; j < ARRAYSIZE(_buttons); j++) {
		if (_buttons[j].action == action) {
			_buttons[j].enabled = true;
			break;
		}
	}
}

void ActionMenu::onMouseMove(const Common::Point &pos) {
	bool hoveringAction = false;
	for (uint i = 0; i < ARRAYSIZE(_buttons); i++) {
		if (_buttons[i].enabled && _buttons[i].rect.contains(pos)) {
			hoveringAction = true;
		}
	}

	if (hoveringAction) {
		_cursor->setCursorType(Cursor::kActive);
	} else {
		_cursor->setCursorType(Cursor::kDefault);
	}
}

void ActionMenu::onClick(const Common::Point &pos) {
	assert(_item);

	for (uint i = 0; i < ARRAYSIZE(_buttons); i++) {
		if (_buttons[i].enabled && _buttons[i].rect.contains(pos)) {

			if (_fromInventory && i == kActionHand) {
				_inventory->setSelectedInventoryItem(_item->getIndex());
			} else {
				if (_fromInventory) {
					StarkGameInterface->itemDoAction(_item, _buttons[i].action);
				} else {
					StarkGameInterface->itemDoActionAt(_item, _buttons[i].action, _itemRelativePos);
				}
			}

			close();
		}
	}
}

void ActionMenu::setInventory(InventoryWindow *inventory) {
	_inventory = inventory;
}

} // End of namespace Stark
