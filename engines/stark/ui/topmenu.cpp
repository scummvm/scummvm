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

#include "engines/stark/ui/topmenu.h"
#include "engines/stark/ui/button.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/userinterface.h"

#include "engines/stark/visual/image.h"

namespace Stark {

TopMenu::TopMenu(Gfx::Driver *gfx, Cursor *cursor) :
	Window(gfx, cursor),
	_widgetsVisible(false) {

	_position = Common::Rect(Gfx::Driver::kOriginalWidth, Gfx::Driver::kTopBorderHeight);
	_visible = true;

	_inventoryButton = new Button("Inventory", StaticProvider::kInventory, Common::Point(0, 0));
	_exitButton = new Button("Exit", StaticProvider::kQuit, Common::Point(600, 0));
	_diaryButton = new Button("Diary",StaticProvider::kDiaryNormal, Common::Point(560, 0));
}

TopMenu::~TopMenu() {
	delete _exitButton;
	delete _inventoryButton;
	delete _diaryButton;
}

void TopMenu::onRender() {
	_widgetsVisible = isMouseInside() && StarkUserInterface->isInteractive();

	if (!_widgetsVisible) {
		return;
	}

	_inventoryButton->render();
	_exitButton->render();
	_diaryButton->render();
}

void TopMenu::onMouseMove(const Common::Point &pos) {
	if (_widgetsVisible && StarkUserInterface->isInteractive()) {
		if (_exitButton->containsPoint(pos) || _inventoryButton->containsPoint(pos)) {
			_cursor->setCursorType(Cursor::kActive);
			_cursor->setMouseHint(getMouseHintAtPosition(pos));
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
		StarkUserInterface->notifyShouldOpenInventory();
	}
}

Common::String TopMenu::getMouseHintAtPosition(Common::Point point) {
	if (_exitButton->containsPoint(point)) {
		return _exitButton->getText();
	}
	if (_diaryButton->containsPoint(point)) {
		return _diaryButton->getText();
	}
	if (_inventoryButton->containsPoint(point)) {
		return _inventoryButton->getText();
	}
	return "";
}

} // End of namespace Stark
