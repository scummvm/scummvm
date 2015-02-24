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

#include "engines/stark/ui.h"

#include "engines/stark/cursor.h"

#include "engines/stark/gfx/renderentry.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/userinterface.h"
namespace Stark {

UI::UI(Gfx::Driver *gfx, Cursor *cursor) :
	_gfx(gfx),
	_cursor(cursor),
	_currentObject(nullptr),
	_objectUnderCursor(nullptr),
	_hasClicked(false)
	{
}

void UI::update(Gfx::RenderEntryArray renderEntries, bool keepExisting) {
	Common::Point pos = _cursor->getMousePosition();
	UserInterface *ui = StarkServices::instance().userInterface;
	Gfx::RenderEntry *currentEntry = ui->getEntryAtPosition(pos, renderEntries);
	Resources::Object *object = ui->getObjectForRenderEntryAtPosition(pos, currentEntry);
	// So that we can run update multiple times, without resetting (i.e. after drawing the action menu)
	if (!object && keepExisting) {
		return;
	} else {
		// Subsequent runs ignore sort order of items drawn earlier.
		_objectUnderCursor = object;
	}
	Common::String mouseHint = ui->getMouseHintForObject(_objectUnderCursor);

	if (_objectUnderCursor) {
		int actionsPossible = ui->getActionsPossibleForObject(_objectUnderCursor);

		bool moreThanOneActionPossible = false;
		switch (actionsPossible) {
			case UserInterface::kActionLookPossible:
				_cursor->setCursorType(Cursor::kEye);
				break;
			case UserInterface::kActionTalkPossible:
				_cursor->setCursorType(Cursor::kMouth);
				break;
			case UserInterface::kActionUsePossible:
				_cursor->setCursorType(Cursor::kHand);
				break;
			case UserInterface::kActionExitPossible:
				_cursor->setCursorType(Cursor::kDefault); // TODO
				break;
			default:
				if (actionsPossible != 0) {
					_cursor->setCursorType(Cursor::kPassive);
					moreThanOneActionPossible = true;
				}
				break;
		}
		if (moreThanOneActionPossible) {
			_cursor->setCursorType(Cursor::kActive);
		}
	} else {
		// Not an object
		_cursor->setCursorType(Cursor::kPassive);
	}
	_cursor->setMouseHint(mouseHint);
}

void UI::handleClick() {
	if (_objectUnderCursor) {
		UserInterface *ui = StarkServices::instance().userInterface;
		if (!ui->performActionOnObject(_objectUnderCursor, _currentObject)) {
			_currentObject = _objectUnderCursor;
			ui->activateActionMenuOn(_cursor->getMousePosition(), _currentObject);
		// This currently potentially allows for click-through
		} else if (ui->isActionMenuOpen()) {
			// If the click resulted in a multi-action possibility, then it was outside the action menu.
			ui->deactivateActionMenu();
			_currentObject = nullptr;
		}
	}
	_hasClicked = false;
}

void UI::notifyClick() {
	_hasClicked = true;
}


void UI::render() {
	Common::Point pos = _cursor->getMousePosition();
	UserInterface *ui = StarkServices::instance().userInterface;
	update(ui->getRenderEntries(), true);
	// Can't handle clicks before this point, since we need to have updated the mouse-over state to include the UI.
	if (_hasClicked) {
		handleClick();
	}
	ui->render();
}

} // End of namespace Stark

