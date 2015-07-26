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

#include "common/translation.h"
#include "gui/saveload.h"
#include "sherlock/tattoo/widget_files.h"
#include "sherlock/tattoo/tattoo.h"
#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo_user_interface.h"

namespace Sherlock {

namespace Tattoo {

#define FILES_LINES_COUNT 5

WidgetFiles::WidgetFiles(SherlockEngine *vm, const Common::String &target) :
		SaveManager(vm, target), WidgetBase(vm), _vm(vm) {
	_fileMode = SAVEMODE_NONE;
	_selector = _oldSelector = -1;
}

void WidgetFiles::show(SaveMode mode) {
	Events &events = *_vm->_events;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();

	if (_vm->_showOriginalSavesDialog) {
		// Render and display the file dialog
		_fileMode = mode;
		ui._menuMode = FILES_MODE;
		_selector = _oldSelector = -1;
		_scroll = true;
		createSavegameList();

		// Set up the display area
		_bounds = Common::Rect(SHERLOCK_SCREEN_WIDTH * 2 / 3, (_surface.fontHeight() + 1) * 
			(FILES_LINES_COUNT + 1) + 17);
		_bounds.moveTo(mousePos.x - _bounds.width() / 2, mousePos.y - _bounds.height() / 2);

		// Create the surface and render it's contents
		_surface.create(_bounds.width(), _bounds.height());
		render(RENDER_ALL);

		summonWindow();
		ui._menuMode = FILES_MODE;
	} else if (mode == SAVEMODE_LOAD) {
		showScummVMRestoreDialog();
	} else {
		showScummVMSaveDialog();
	}
}

void WidgetFiles::showScummVMSaveDialog() {
	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);

	int slot = dialog->runModalWithCurrentTarget();
	if (slot >= 0) {
		Common::String desc = dialog->getResultString();

		if (desc.empty()) {
			// create our own description for the saved game, the user didn't enter it
			desc = dialog->createDefaultSaveDescription(slot);
		}

		_vm->saveGameState(slot, desc);
	}

	close();
	delete dialog;
}

void WidgetFiles::showScummVMRestoreDialog() {
	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
	int slot = dialog->runModalWithCurrentTarget();
	close();
	delete dialog;

	if (slot >= 0) {
		_vm->loadGameState(slot);
	}
}

void WidgetFiles::render(FilesRenderMode mode) {
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	ImageFile &images = *ui._interfaceImages;
	byte color;

	if (mode == OP_ALL) {
		_surface.fill(TRANSPARENCY);
		makeInfoArea();

		switch (_fileMode) {
		case SAVEMODE_LOAD:
			_surface.writeString(FIXED(LoadGame),
				Common::Point((_surface.w() - _surface.stringWidth(FIXED(LoadGame))) / 2, 5), INFO_TOP);
			break;

		case SAVEMODE_SAVE:
			_surface.writeString(FIXED(SaveGame),
				Common::Point((_surface.w() - _surface.stringWidth(FIXED(SaveGame))) / 2, 5), INFO_TOP);
			break;

		default:
			break;
		}

		_surface.hLine(3, _surface.fontHeight() + 7, _surface.w() - 4, INFO_TOP);
		_surface.hLine(3, _surface.fontHeight() + 8, _surface.w() - 4, INFO_MIDDLE);
		_surface.hLine(3, _surface.fontHeight() + 9, _surface.w() - 4, INFO_BOTTOM);
		_surface.transBlitFrom(images[4], Common::Point(0, _surface.fontHeight() + 6));
		_surface.transBlitFrom(images[5], Common::Point(_surface.w() - images[5]._width, _surface.fontHeight() + 6));

		int xp = _surface.w() - BUTTON_SIZE - 6;
		_surface.vLine(xp, _surface.fontHeight() + 10, _bounds.height() - 4, INFO_TOP);
		_surface.vLine(xp + 1, _surface.fontHeight() + 10, _bounds.height() - 4, INFO_MIDDLE);
		_surface.vLine(xp + 2, _surface.fontHeight() + 10, _bounds.height() - 4, INFO_BOTTOM);
		_surface.transBlitFrom(images[6], Common::Point(xp - 1, _surface.fontHeight() + 8));
		_surface.transBlitFrom(images[7], Common::Point(xp - 1, _bounds.height() - 4));
	}

	int xp = _surface.stringWidth("00.") + _surface.widestChar() + 5;
	int yp = _surface.fontHeight() + 14;
	
	for (int idx = _savegameIndex; idx < (_savegameIndex + FILES_LINES_COUNT); ++idx) {
		if (OP_NAMES || idx == _selector || idx == _oldSelector) {
			if (idx == _selector && mode != OP_ALL)
				color = COMMAND_HIGHLIGHTED;
			else
				color = INFO_TOP;

			if (mode == RENDER_NAMES_AND_SCROLLBAR)
				_surface.fillRect(Common::Rect(4, yp, _surface.w() - BUTTON_SIZE - 9, yp + _surface.fontHeight()), TRANSPARENCY);
			
			Common::String numStr = Common::String::format("%d.", idx + 1);
			_surface.writeString(numStr, Common::Point(_surface.widestChar(), yp), color);
			_surface.writeString(_savegames[idx], Common::Point(xp, yp), color);
		}

		yp += _surface.fontHeight() + 1;
	}

	// Draw the Scrollbar if neccessary
	if (mode != RENDER_NAMES)
		drawScrollBar(_savegameIndex, FILES_LINES_COUNT, _savegames.size());
}

void WidgetFiles::handleEvents() {
	Events &events = *_vm->_events;
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();
	Common::KeyState keyState = ui._keyState;

	// Handle scrollbar events
	ScrollHighlight oldHighlight = ui._scrollHighlight;	
	handleScrollbarEvents(_savegameIndex, FILES_LINES_COUNT, _savegames.size());

	int oldScrollIndex = _savegameIndex;
	handleScrolling(_savegameIndex, FILES_LINES_COUNT, _savegames.size());

	// See if the mouse is pointing at any filenames in the window
	if (Common::Rect(_bounds.left, _bounds.top + _surface.fontHeight() + 14,
			_bounds.right - BUTTON_SIZE - 5, _bounds.bottom - 5).contains(mousePos)) {
		_selector = (mousePos.y - _bounds.top - _surface.fontHeight() - 14) / (_surface.fontHeight() + 1) +
			_savegameIndex;
	} else {
		_selector = -1;
	}

	// Check for the Tab key
	if (keyState.keycode == Common::KEYCODE_TAB) {
		// If the mouse is not over any of the filenames, move the mouse so that it points to the first one
		if (_selector == -1) {
			events.warpMouse(Common::Point(_bounds.right - BUTTON_SIZE - 20,
				_bounds.top + _surface.fontHeight() * 2 + 8));
		} else {
			// See if we're doing Tab or Shift Tab
			if (keyState.flags & Common::KBD_SHIFT) {
				// We're doing Shift Tab
				if (_selector == _savegameIndex)
					_selector = _savegameIndex + 4;
				else
					--_selector;
			} else {
				// We're doing Tab
				++_selector;
				if (_selector >= _savegameIndex + 5)
					_selector = _savegameIndex;
			}

			events.warpMouse(Common::Point(mousePos.x, _bounds.top + _surface.fontHeight() * 2
				+ 8 + (_selector - _savegameIndex) * (_surface.fontHeight() + 1)));
		}
	}

	// Only redraw the window if the the scrollbar position has changed
	if (ui._scrollHighlight != oldHighlight || oldScrollIndex != _savegameIndex || _selector != _oldSelector)
		render(RENDER_NAMES_AND_SCROLLBAR);
	_oldSelector = _selector;

	if (events._firstPress && !_bounds.contains(mousePos))
		_outsideMenu = true;

	if (events._released || events._rightReleased || keyState.keycode == Common::KEYCODE_ESCAPE) {
		ui._scrollHighlight = SH_NONE;

		if (_outsideMenu && !_bounds.contains(mousePos)) {
			close();
		} else {
			_outsideMenu = false;

			if (_selector != -1) {
				if (_fileMode = SAVEMODE_LOAD) {
					// We're in Load Mode
					_vm->loadGameState(_selector);
				} else if (_fileMode == SAVEMODE_SAVE) {
					// We're in Save Mode
					if (getFilename())
						_vm->saveGameState(_selector, _savegames[_selector]);
					close();
				}
			}
		}
	}
}

bool WidgetFiles::getFilename() {
	return false;
}

Common::Rect WidgetFiles::getScrollBarBounds() const {
	Common::Rect scrollRect(BUTTON_SIZE, _bounds.height() - _surface.fontHeight() - 16);
	scrollRect.moveTo(_bounds.width() - BUTTON_SIZE - 3, _surface.fontHeight() + 13);

	return scrollRect;
}

} // End of namespace Tattoo

} // End of namespace Sherlock
