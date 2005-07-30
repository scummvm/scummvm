/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "CEKeysDialog.h"
#include "CEActions.h"

using GUI::ListWidget;
using GUI::kListNumberingZero;
using GUI::WIDGET_CLEARBG;
using GUI::kListSelectionChangedCmd;
using GUI::kCloseCmd;
using GUI::StaticTextWidget;
using GUI::kTextAlignCenter;
using GUI::CommandSender;

enum {
	kMapCmd					= 'map ',
	kOKCmd					= 'ok  '
};


CEKeysDialog::CEKeysDialog(const Common::String &title)
	: GUI::Dialog(30, 20, 260, 160) {
	addButton(160, 20, "Map", kMapCmd, 'M');	// Map
	addButton(160, 40, "OK", kOKCmd, 'O');						// OK
	addButton(160, 60, "Cancel", kCloseCmd, 'C');				// Cancel

	_actionsList = new ListWidget(this, 10, 20, 140, 90);
	_actionsList->setNumberingMode(kListNumberingZero);

	_actionTitle = new StaticTextWidget(this, 10, 120, 240, 16, title, kTextAlignCenter);
	_keyMapping = new StaticTextWidget(this, 10, 140, 240, 16, "", kTextAlignCenter);

	_actionTitle->setFlags(WIDGET_CLEARBG);
	_keyMapping->setFlags(WIDGET_CLEARBG);

	// Get actions names
	Common::StringList l;

	for (int i = 0; i < CEActions::Instance()->size(); i++)
		l.push_back(CEActions::Instance()->actionName((ActionType)i));

	_actionsList->setList(l);

	_actionSelected = -1;
	CEActions::Instance()->beginMapping(false);
}

void CEKeysDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch(cmd) {

	case kListSelectionChangedCmd:
		if (_actionsList->getSelected() >= 0) {
				char selection[100];

				sprintf(selection, "Associated key : %s", CEDevice::getKeyName(CEActions::Instance()->getMapping((ActionType)(_actionsList->getSelected()))).c_str());
				_keyMapping->setLabel(selection);
				_keyMapping->draw();
		}
		break;
	case kMapCmd:
		if (_actionsList->getSelected() < 0) {
				_actionTitle->setLabel("Please select an action");
		}
		else {
				char selection[100];

				_actionSelected = _actionsList->getSelected();
				sprintf(selection, "Associated key : %s", CEDevice::getKeyName(CEActions::Instance()->getMapping((ActionType)_actionSelected)).c_str());
				_actionTitle->setLabel("Press the key to associate");
				_keyMapping->setLabel(selection);
				_keyMapping->draw();
				CEActions::Instance()->beginMapping(true);
				_actionsList->setEnabled(false);
		}
		_actionTitle->draw();
		break;
	case kOKCmd:
		CEActions::Instance()->saveMapping();
		close();
		break;
	case kCloseCmd:
		CEActions::Instance()->loadMapping();
		close();
		break;
	}
}

void CEKeysDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	if (modifiers == 0xff  && CEActions::Instance()->mappingActive()) {
		// GAPI key was selected
		char selection[100];

		CEActions::Instance()->setMapping((ActionType)_actionSelected, ascii);

		sprintf(selection, "Associated key : %s", CEDevice::getKeyName(CEActions::Instance()->getMapping((ActionType)_actionSelected)).c_str());
		_actionTitle->setLabel("Choose an action to map");
		_keyMapping->setLabel(selection);
		_keyMapping->draw();
		_actionSelected = -1;
		_actionsList->setEnabled(true);
		CEActions::Instance()->beginMapping(false);
	}
}
