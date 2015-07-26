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
	savegameIndex = 0;
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
				_surface.fillRect(Common::Rect(4, yp, _surface.w() - BUTTON_SIZE - 9, yp + _surface.fontHeight() - 1), TRANSPARENCY);
			
			Common::String numStr = Common::String::format("%d.", idx + 1);
			_surface.writeString(numStr, Common::Point(_surface.widestChar(), yp), color);
			_surface.writeString(_savegames[idx], Common::Point(xp, yp), color);
		}

		yp += _surface.fontHeight() + 1;
	}

	// Draw the Scrollbar if neccessary
	if (mode != RENDER_NAMES) {
		Common::Rect scrollRect(BUTTON_SIZE, _bounds.height() - _surface.fontHeight() - 16);
		scrollRect.moveTo(_bounds.width() - BUTTON_SIZE - 3, _surface.fontHeight() + 13);
		drawScrollBar(_savegameIndex, FILES_LINES_COUNT, _savegames.size(), scrollRect);
	}
}

void WidgetFiles::handleEvents() {
	//Events &events = *_vm->_events;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	// Handle scrollbar events
	ScrollHighlight oldHighlight = ui._scrollHighlight;	
	Common::Rect scrollRect(BUTTON_SIZE, _bounds.height() - _surface.fontHeight() - 16);
	scrollRect.moveTo(_bounds.right - BUTTON_SIZE - 3, _bounds.top + _surface.fontHeight() + 13);
	handleScrollbarEvents(_savegameIndex, FILES_LINES_COUNT, _savegames.size(), scrollRect);

	// If the highlight has changed, redraw the scrollbar
	if (ui._scrollHighlight != oldHighlight)
		render(RENDER_NAMES_AND_SCROLLBAR);

	// TODO
}

} // End of namespace Tattoo

} // End of namespace Sherlock
