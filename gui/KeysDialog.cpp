/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "stdafx.h"
#include "gui/KeysDialog.h"
#include "gui/Actions.h"
#include <sdl_keyboard.h>

#ifdef _WIN32_WCE
#include "CEDevice.h"
#endif

namespace GUI {

/*
using GUI::ListWidget;
using GUI::kListNumberingZero;
using GUI::WIDGET_CLEARBG;
using GUI::kListSelectionChangedCmd;
using GUI::kCloseCmd;
using GUI::StaticTextWidget;
using GUI::kTextAlignCenter;
using GUI::CommandSender;
*/

enum {
	kMapCmd					= 'map ',
	kOKCmd					= 'ok  '
};

KeysDialog::KeysDialog(const Common::String &title)
	: GUI::Dialog(30, 20, 260, 160) {

	GUI::WidgetSize ws = GUI::kNormalWidgetSize;

//tmp
//	addButton(this, _w - (buttonWidth + 10), _h - buttonHeight - 8, "Choose", kChooseCmd, 0, ws);
//tmp
	addButton(this, 160, 20, "Map", kMapCmd, 0, ws);					// Map
	addButton(this, 160, 40, "OK", kOKCmd, 0, ws);						// OK
	addButton(this, 160, 60, "Cancel", kCloseCmd, 0, ws);				// Cancel

	_actionsList = new ListWidget(this, 10, 20, 140, 90);
	_actionsList->setNumberingMode(kListNumberingZero);

	_actionTitle = new StaticTextWidget(this, 10, 120, 240, 16, title, kTextAlignCenter);
	_keyMapping = new StaticTextWidget(this, 10, 140, 240, 16, "", kTextAlignCenter);

	_actionTitle->setFlags(WIDGET_CLEARBG);
	_keyMapping->setFlags(WIDGET_CLEARBG);

	// Get actions names
	Common::StringList l;

	for (int i = 0; i < Actions::Instance()->size(); i++)
		l.push_back(Actions::Instance()->actionName((ActionType)i));

	_actionsList->setList(l);

	_actionSelected = -1;
	Actions::Instance()->beginMapping(false);
}

void KeysDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch(cmd) {

	case kListSelectionChangedCmd:
		if (_actionsList->getSelected() >= 0) {
				char selection[100];
#ifdef __SYMBIAN32__
				uint16 key = Actions::Instance()->getMapping(_actionsList->getSelected());
				if(key != 0)
					sprintf(selection, "Associated key : %s", SDL_GetKeyName((SDLKey)key));
				else
					sprintf(selection, "Associated key : none");
#else
				sprintf(selection, "Associated key : %s", CEDevice::getKeyName(Actions::Instance()->getMapping((ActionType)(_actionsList->getSelected()))).c_str());
#endif
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
#ifdef __SYMBIAN32__
				uint16 key = Actions::Instance()->getMapping(_actionSelected);
				if(key != 0)
					sprintf(selection, "Associated key : %s", SDL_GetKeyName((SDLKey)key));
				else
					sprintf(selection, "Associated key : none");
#else
				sprintf(selection, "Associated key : %s", CEDevice::getKeyName(Actions::Instance()->getMapping((ActionType)_actionSelected)).c_str());
#endif
				_actionTitle->setLabel("Press the key to associate");
				_keyMapping->setLabel(selection);
				_keyMapping->draw();
				Actions::Instance()->beginMapping(true);
				_actionsList->setEnabled(false);
		}
		_actionTitle->draw();
		break;
	case kOKCmd:
		Actions::Instance()->saveMapping();
		close();
		break;
	case kCloseCmd:
		Actions::Instance()->loadMapping();
		close();
		break;
	}
}

void KeysDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers){
		if (!Actions::Instance()->mappingActive()) {
			Dialog::handleKeyDown(ascii,keycode,modifiers);
		}
}

void KeysDialog::handleKeyUp(uint16 ascii, int keycode, int modifiers) {
#ifdef __SYMBIAN32__
	if (Actions::Instance()->mappingActive()) {
#else
		// GAPI key was selected
		if (modifiers == 0xff  && Actions::Instance()->mappingActive()) {
#endif
		char selection[100];

		Actions::Instance()->setMapping((ActionType)_actionSelected, ascii);
#ifdef __SYMBIAN32__
		if(ascii != 0)
			sprintf(selection, "Associated key : %s", SDL_GetKeyName((SDLKey)ascii));
		else
			sprintf(selection, "Associated key : none");
#else
		sprintf(selection, "Associated key : %s", CEDevice::getKeyName(Actions::Instance()->getMapping((ActionType)_actionSelected)).c_str());
#endif
		_actionTitle->setLabel("Choose an action to map");
		_keyMapping->setLabel(selection);
		_keyMapping->draw();
		_actionTitle->draw();
		_actionSelected = -1;
		_actionsList->setEnabled(true);
		Actions::Instance()->beginMapping(false);
	}
	else {
		Dialog::handleKeyUp(ascii,keycode,modifiers);
	}
}

} // namespace GUI
