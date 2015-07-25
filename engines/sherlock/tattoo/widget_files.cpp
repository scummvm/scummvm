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

WidgetFiles::WidgetFiles(SherlockEngine *vm, const Common::String &target) :
		SaveManager(vm, target), WidgetBase(vm), _vm(vm) {
	_fileMode = SAVEMODE_NONE;
}

void WidgetFiles::show(SaveMode mode) {
	Events &events = *_vm->_events;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();

	if (_vm->_showOriginalSavesDialog) {
		// Render and display the file dialog
		_fileMode = mode;
		render();

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

void WidgetFiles::render() {
	Events &events = *_vm->_events;
	Common::Point mousePos = events.mousePos();

	createSavegameList();

	// Set up the display area
	_bounds = Common::Rect(_surface.stringWidth(FIXED(AreYouSureYou)) + _surface.widestChar() * 2,
		(_surface.fontHeight() + 7) * 4);
	_bounds.moveTo(mousePos.x - _bounds.width() / 2, mousePos.y - _bounds.height() / 2);

	// Create the surface
	_surface.create(_bounds.width(), _bounds.height());
	_surface.fill(TRANSPARENCY);
	makeInfoArea();
}

void WidgetFiles::handleEvents() {
	//Events &events = *_vm->_events;

}

void WidgetFiles::close() {
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	banishWindow();
	ui._menuMode = scene._labTableScene ? LAB_MODE : STD_MODE;
}

} // End of namespace Tattoo

} // End of namespace Sherlock
