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
#include "engines/stark/ui/world/gamewindow.h"
#include "engines/stark/ui/world/inventorywindow.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/knowledgeset.h"
#include "engines/stark/resources/level.h"
#include "engines/stark/resources/pattable.h"
#include "engines/stark/resources/sound.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/gameinterface.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/global.h"

#include "engines/stark/scene.h"

#include "engines/stark/visual/image.h"
#include "engines/stark/visual/text.h"


namespace Stark {

static const int kAutoCloseSuspended = -1;
static const int kAutoCloseDisabled  = -2;
static const int kAutoCloseDelay     = 200;

ActionMenu::ActionMenu(Gfx::Driver *gfx, Cursor *cursor) :
		Window(gfx, cursor),
		_fromInventory(false),
		_itemDescription(nullptr),
		_item(nullptr),
		_inventory(nullptr),
		_activeMenuType(kActionNone),
		_autoCloseTimeRemaining(kAutoCloseDisabled) {

	_background = StarkStaticProvider->getUIElement(StaticProvider::kActionMenuBg);

	_itemDescription = new VisualText(gfx);
	_itemDescription->setColor(Color(0xFF, 0xFF, 0xFF));
	_itemDescription->setBackgroundColor(Color(0x00, 0x00, 0x00, 0x80));
	_itemDescription->setFont(FontProvider::kSmallFont);
	_itemDescription->setTargetWidth(96);

	_buttons[kActionHand].action = Resources::PATTable::kActionUse;
	_buttons[kActionHand].rect = Common::Rect(90, 15, 126, 63);
	_buttons[kActionEye].action = Resources::PATTable::kActionLook;
	_buttons[kActionEye].rect = Common::Rect(5, 77, 51, 110);
	_buttons[kActionMouth].action = Resources::PATTable::kActionTalk;
	_buttons[kActionMouth].rect = Common::Rect(42, 35, 83, 74);

	_actionMouthHoverSound = StarkStaticProvider->getUISound(StaticProvider::kActionMouthHover);
	_actionMouthHoverSound->setLooping(false);
	_actionHoverSound = StarkStaticProvider->getUISound(StaticProvider::kActionHover);

	clearActions();
}

ActionMenu::~ActionMenu() {
	delete _itemDescription;
}

void ActionMenu::open(Resources::ItemVisual *item, const Common::Point &itemRelativePos) {
	_visible = true;

	Common::Point mousePos = _cursor->getMousePosition();

	_position = computePosition(mousePos);

	_itemRelativePos = itemRelativePos;
	_item = item;
	_fromInventory = item->getSubType() == Resources::Item::kItemInventory;

	if (_fromInventory) {
		_itemDescription->setText(StarkGameInterface->getItemTitle(item));
	} else {
		_itemDescription->setText(StarkGameInterface->getItemTitleAt(item, itemRelativePos));
	}

	_cursor->setMouseHint("");

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
	_activeMenuType = kActionNone;
	_actionHoverSound->stop();
}

Common::Rect ActionMenu::computePosition(const Common::Point &mouse) const {
	Common::Rect position = Common::Rect::center(mouse.x, mouse.y, 160, 111);

	Common::Rect gameWindowRect(Gfx::Driver::kGameViewportWidth, Gfx::Driver::kGameViewportHeight);
	gameWindowRect.translate(0, Gfx::Driver::kTopBorderHeight);

	if (position.top < gameWindowRect.top)       position.translate(0, gameWindowRect.top - position.top);
	if (position.left < gameWindowRect.left)     position.translate(gameWindowRect.left - position.left, 0);
	if (position.bottom > gameWindowRect.bottom) position.translate(0, gameWindowRect.bottom - position.bottom);
	if (position.right > gameWindowRect.right)   position.translate(gameWindowRect.right - position.right, 0);

	return position;
}

void ActionMenu::onRender() {
	_background->render(Common::Point(0, 0), false);

	for (uint i = 0; i < ARRAYSIZE(_buttons); i++) {
		if (_buttons[i].enabled) {
			VisualImageXMG *visual = StarkGameInterface->getActionImage(_buttons[i].action, i == _activeMenuType);
			visual->render(Common::Point(_buttons[i].rect.left, _buttons[i].rect.top), false);
		}
	}

	Common::Rect descriptionSize = _itemDescription->getRect();
	int descriptionX = 60 + (_itemDescription->getTargetWidth() - descriptionSize.width()) / 2;
	int descriptionY = _position.height() - descriptionSize.height();

	_itemDescription->render(Common::Point(descriptionX, descriptionY));
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

void ActionMenu::updateActionSound() {
	if (_activeMenuType == kActionNone) {
		_actionHoverSound->stop();
		return;
	}
	_actionHoverSound->play();
	if (_activeMenuType == kActionMouth) {
		_actionMouthHoverSound->play();
	}
}

void ActionMenu::onMouseMove(const Common::Point &pos) {
	int32 prevActive = _activeMenuType;
	int32 newActive  = kActionNone;
	for (uint i = 0; i < ARRAYSIZE(_buttons); i++) {
		if (_buttons[i].enabled && _buttons[i].rect.contains(pos)) {
			newActive = i;
		}
	}

	if (newActive != prevActive) {
		_activeMenuType = newActive;
		if (_activeMenuType == kActionNone) {
			_cursor->setCursorType(Cursor::kDefault);
		} else {
			_cursor->setCursorType(Cursor::kActive);
		}
		updateActionSound();
	}

	_autoCloseTimeRemaining = kAutoCloseSuspended;
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
			break;
		}
	}
}

void ActionMenu::setInventory(InventoryWindow *inventory) {
	_inventory = inventory;
}

void ActionMenu::onGameLoop() {
	if (!isMouseInside() && _autoCloseTimeRemaining == kAutoCloseSuspended) {
		_autoCloseTimeRemaining = kAutoCloseDelay;
	} else if (_autoCloseTimeRemaining >= 0) {
		_autoCloseTimeRemaining -= StarkGlobal->getMillisecondsPerGameloop();

		if (_autoCloseTimeRemaining <= 0) {
			_autoCloseTimeRemaining = kAutoCloseSuspended;
			close();
		}
	}
}

void ActionMenu::onScreenChanged() {
	_itemDescription->resetTexture();
}

} // End of namespace Stark
