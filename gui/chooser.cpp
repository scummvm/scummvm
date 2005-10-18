/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
 * $Header$
 */

#include "common/stdafx.h"
#include "common/system.h"
#include "gui/chooser.h"
#include "gui/newgui.h"
#include "gui/ListWidget.h"

namespace GUI {

enum {
	kChooseCmd = 'Chos'
};

ChooserDialog::ChooserDialog(const String &title, const String &buttonLabel, int height)
	: Dialog(8, (200 - height) / 2, 320 - 2 * 8, height) {

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	GUI::WidgetSize ws;
	int buttonWidth, buttonHeight;

	if (screenW >= 400 && screenH >= 300) {
		ws = GUI::kBigWidgetSize;
		buttonHeight = kBigButtonHeight;
		buttonWidth = kBigButtonWidth;
	} else {
		ws = GUI::kNormalWidgetSize;
		buttonHeight = kButtonHeight;
		buttonWidth = kButtonWidth;
	}

	// FIXME: This is an ugly hack. The 'height' parameter assumes a 200
	// pixel tall screen, so try to scale that to something sensible.

	_h = (screenH * height) / 200;
	_w = screenW - 2 * 8;

	_x = (screenW - _w) / 2;
	_y = (screenH - _h) / 2;

	int yoffset = 6;

	// Headline
	new StaticTextWidget(this, 10, 6, _w - 2 * 10, kLineHeight, title, kTextAlignCenter, ws);

	yoffset += kLineHeight + 2;

	// Add choice list
	// HACK: Subtracting -12 from the height makes the list look good when
	// it's used to list savegames in the 320x200 version of the GUI.
	_list = new ListWidget(this, 10, yoffset, _w - 2 * 10, _h - yoffset - buttonHeight - 12, ws);
	_list->setNumberingMode(kListNumberingOff);

	// Buttons
	addButton(this, _w - 2 * (buttonWidth + 10), _h - buttonHeight - 8, "Cancel", kCloseCmd, 0, ws);
	_chooseButton = addButton(this, _w - (buttonWidth + 10), _h - buttonHeight - 8, buttonLabel, kChooseCmd, 0, ws);
	_chooseButton->setEnabled(false);
}

void ChooserDialog::setList(const StringList& list) {
	_list->setList(list);
}

void ChooserDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	int item = _list->getSelected();
	switch (cmd) {
	case kChooseCmd:
	case kListItemDoubleClickedCmd:
		_list->endEditMode();
		setResult(item);
		close();
		break;
	case kListSelectionChangedCmd:
		_chooseButton->setEnabled(item >= 0);
		_chooseButton->draw();
		break;
	case kCloseCmd:
		setResult(-1);
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

} // End of namespace GUI
