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

#include "engines/stark/ui/gamewindow.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/knowledgeset.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/location.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/userinterface.h"

#include "engines/stark/actionmenu.h"

#include "engines/stark/visual/image.h"

namespace Stark {

GameWindow::GameWindow(Gfx::Driver *gfx, Cursor *cursor, ActionMenu *actionMenu) :
	Window(gfx, cursor),
	_actionMenu(actionMenu) {
	_position = Common::Rect(Gfx::Driver::kGameViewportWidth, Gfx::Driver::kGameViewportHeight);
	_position.translate(0, Gfx::Driver::kTopBorderHeight);
	_visible = true;
}

void GameWindow::onRender() {
	Global *global = StarkServices::instance().global;

	// List the items to render
	_renderEntries = global->getCurrent()->getLocation()->listRenderEntries();

	// Render all the scene items
	Gfx::RenderEntryArray::iterator element = _renderEntries.begin();
	while (element != _renderEntries.end()) {
		// Draw the current element
		(*element)->render(_gfx);

		// Go for the next one
		element++;
	}
}

void GameWindow::onMouseMove(const Common::Point &pos) {
	Global *global = StarkServices::instance().global;
	_renderEntries = global->getCurrent()->getLocation()->listRenderEntries();

	updateItems();
}

void GameWindow::onClick(const Common::Point &pos) {
	UserInterface *ui = StarkServices::instance().userInterface;

	_actionMenu->close();

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
			Resources::ActionArray actions = ui->getStockActionsPossibleForObject(_objectUnderCursor, _objectRelativePosition);
			if (actions.size() == 1) {
				for (uint i = 0; i < actions.size(); i++) {
					if (actions[i] == Resources::PATTable::kActionLook) {
						ui->itemDoActionAt(_objectUnderCursor, Resources::PATTable::kActionLook, _objectRelativePosition);
						break;
					} else if (actions[i] == Resources::PATTable::kActionTalk) {
						ui->itemDoActionAt(_objectUnderCursor, Resources::PATTable::kActionTalk, _objectRelativePosition);
						break;
					} else if (actions[i] == Resources::PATTable::kActionUse) {
						ui->itemDoActionAt(_objectUnderCursor, Resources::PATTable::kActionUse, _objectRelativePosition);
						break;
					}
				}
			} else if (actions.size() > 1) {
				_actionMenu->open(_objectUnderCursor, _objectRelativePosition);
			}
		}
	} else {
		ui->walkTo(getScreenMousePosition());
	}
}

} // End of namespace Stark
