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

#include "engines/stark/services/services.h"
#include "engines/stark/services/userinterface.h"

#include "engines/stark/visual/image.h"

namespace Stark {

TopMenu::TopMenu(Gfx::Driver *gfx, Cursor *cursor) :
		Window(gfx, cursor),
	_widgetsVisible(false) {

	_position = Common::Rect(Gfx::Driver::kOriginalWidth, Gfx::Driver::kTopBorderHeight);
	_visible = true;

	_inventoryButton = new Button("Inventory", StaticProvider::kInventory, Common::Point(32, 2), Common::Point(64, 20));
	_optionsButton = new Button("Options", StaticProvider::kDiaryNormal, Common::Point(560, 2), Common::Point(523, 20));
	_exitButton = new Button("Quit", StaticProvider::kQuit, Common::Point(608, 2), Common::Point(587, 20));
}

TopMenu::~TopMenu() {
	delete _exitButton;
	delete _inventoryButton;
	delete _optionsButton;
}

void TopMenu::onRender() {
	_widgetsVisible = isMouseInside() && StarkUserInterface->isInteractive();

	if (!_widgetsVisible) {
		return;
	}

	_inventoryButton->render();
	_exitButton->render();
	_optionsButton->render();
}

void TopMenu::onMouseMove(const Common::Point &pos) {
	if (_widgetsVisible && StarkUserInterface->isInteractive()) {
		Button *hoveredButton = getButtonAtPosition(pos);
		if (hoveredButton) {
			_cursor->setCursorType(Cursor::kActive);
			_cursor->setMouseHint(hoveredButton->getText(), &hoveredButton->getHintPosition());
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

} // End of namespace Stark
