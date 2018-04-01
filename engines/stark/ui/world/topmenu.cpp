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

#include "engines/stark/ui/world/topmenu.h"

#include "engines/stark/ui/cursor.h"
#include "engines/stark/ui/world/button.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/resources/knowledgeset.h"
#include "engines/stark/resources/sound.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/userinterface.h"

#include "engines/stark/visual/image.h"

namespace Stark {

TopMenu::TopMenu(Gfx::Driver *gfx, Cursor *cursor) :
		Window(gfx, cursor),
	_widgetsVisible(false),
	_forceVisibleTimeRemaining(0) {

	_position = Common::Rect(Gfx::Driver::kOriginalWidth, Gfx::Driver::kTopBorderHeight);
	_visible = true;

	_inventoryButton = new Button("Inventory", StaticProvider::kInventory, Common::Point(32, 2), Button::kAlignLeft, Common::Point(64, 20));
	_optionsButton = new Button("Options", StaticProvider::kDiaryNormal, Common::Point(560, 2), Button::kAlignRight, Common::Point(560, 20));
	_exitButton = new Button("Quit", StaticProvider::kQuit, Common::Point(608, 2), Button::kAlignRight, Common::Point(608, 20));

	_inventoryNewItemSound = StarkStaticProvider->getUISound(StaticProvider::kInventoryNewItem);
}

TopMenu::~TopMenu() {
	delete _exitButton;
	delete _inventoryButton;
	delete _optionsButton;
}

void TopMenu::onRender() {
	_widgetsVisible = (isMouseInside() && StarkUserInterface->isInteractive()) || _forceVisibleTimeRemaining > 0;

	if (!_widgetsVisible) {
		return;
	}

	if (_forceVisibleTimeRemaining > 0) {
		_forceVisibleTimeRemaining -= StarkGlobal->getMillisecondsPerGameloop();

		if (_forceVisibleTimeRemaining <= 0) {
			_inventoryButton->stopImageExplosion();
			_inventoryButton->goToAnimStatement(12); // TODO: Closing animation is rendered only when mouse is hovering topmenu
			_optionsButton->stopImageFlashing();
		}
	}

	_inventoryButton->render();
	_optionsButton->render();
	_exitButton->render();
}

void TopMenu::onMouseMove(const Common::Point &pos) {
	if (_widgetsVisible && StarkUserInterface->isInteractive()) {
		Button *hoveredButton = getButtonAtPosition(pos);
		if (hoveredButton) {
			_cursor->setCursorType(Cursor::kActive);
			hoveredButton->showButtonHint();
		} else {
			_cursor->setCursorType(Cursor::kDefault);
			_cursor->setMouseHint("");
		}
	} else {
		_cursor->setCursorType(Cursor::kPassive);
		_cursor->setMouseHint("");
	}
}

void TopMenu::onClick(const Common::Point &pos) {
	if (!_widgetsVisible || !StarkUserInterface->isInteractive()) {
		return;
	}

	if (_exitButton->containsPoint(pos)) {
		// TODO: Ask
		StarkUserInterface->notifyShouldExit();
	}

	if (_inventoryButton->containsPoint(pos)) {
		StarkUserInterface->inventoryOpen(true);
	}

	if (_optionsButton->containsPoint(pos)) {
		StarkUserInterface->optionsOpen();
	}
}

Button *TopMenu::getButtonAtPosition(const Common::Point &point) const {
	if (_exitButton->containsPoint(point)) {
		return _exitButton;
	} else if (_optionsButton->containsPoint(point)) {
		return _optionsButton;
	} else if (_inventoryButton->containsPoint(point)) {
		return _inventoryButton;
	}

	return nullptr;
}

void TopMenu::onScreenChanged() {
	_exitButton->resetHintVisual();
	_inventoryButton->resetHintVisual();
	_optionsButton->resetHintVisual();
}

void TopMenu::notifyInventoryItemEnabled(uint16 itemIndex) {
	_forceVisibleTimeRemaining = 128 * 33; // 128 frames at 30 fps
	_inventoryButton->goToAnimStatement(2);

	Visual *inventoryItemImage = StarkGlobal->getInventory()->getInventoryItemVisual(itemIndex);
	_inventoryButton->startImageExplosion(inventoryItemImage->get<VisualImageXMG>());

	assert(_inventoryNewItemSound);
	_inventoryNewItemSound->stop();
	_inventoryNewItemSound->play();
}

void TopMenu::notifyDiaryEntryEnabled() {
	_forceVisibleTimeRemaining = 150 * 33; // 150 frames at 30 fps
	_optionsButton->setUIElement(StaticProvider::kDiaryTabbed);

	VisualImageXMG *diaryImage = StarkStaticProvider->getUIElement(StaticProvider::kDiaryTabbed);
	_optionsButton->startImageFlashing(diaryImage);
}

} // End of namespace Stark
