/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2007 The ScummVM project
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

#include "common/stdafx.h"
#include "gui/KeysDialog.h"
#include "gui/Actions.h"
#include <SDL_keyboard.h>

#ifdef _WIN32_WCE
#include "CEDevice.h"
#endif

namespace GUI {

enum {
	kMapCmd	= 'map ',
	kOKCmd	= 'ok  '
};

KeysDialog::KeysDialog(const Common::String &title)
	: GUI::Dialog("keysdialog") {

	new ButtonWidget(this, "keysdialog_map", "Map", kMapCmd, 0);
	new ButtonWidget(this, "keysdialog_ok", "OK", kOKCmd, 0);
	new ButtonWidget(this, "keysdialog_cancel", "Cancel", kCloseCmd, 0);

	_actionsList = new ListWidget(this, "keysdialog_list");
	_actionsList->setNumberingMode(kListNumberingZero);

	_actionTitle = new StaticTextWidget(this, "keysdialog_action", title);
	_keyMapping = new StaticTextWidget(this, "keysdialog_mapping", "Select an action and click 'Map'");

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
	switch (cmd) {

	case kListSelectionChangedCmd:
		if (_actionsList->getSelected() >= 0) {
			char selection[100];

			uint16 key = Actions::Instance()->getMapping(_actionsList->getSelected());
#ifdef __SYMBIAN32__
			// ScummVM mappings for F1-F9 are different from SDL so remap back to sdl
			if (key >= 315 && key <= 323)
				key = key - 315 + SDLK_F1;
#endif
			if (key != 0)
				sprintf(selection, "Associated key : %s", SDL_GetKeyName((SDLKey)key));
			else
				sprintf(selection, "Associated key : none");

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
			uint16 key = Actions::Instance()->getMapping(_actionSelected);
#ifdef __SYMBIAN32__
			// ScummVM mappings for F1-F9 are different from SDL so remap back to sdl
			if (key >= 315 && key <= 323)
				key = key - 315 + SDLK_F1;
#endif
			if (key != 0) 
				sprintf(selection, "Associated key : %s", SDL_GetKeyName((SDLKey)key));
			else
				sprintf(selection, "Associated key : none");

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
	if (!Actions::Instance()->mappingActive())
		Dialog::handleKeyDown(ascii,keycode,modifiers);
}

void KeysDialog::handleKeyUp(uint16 ascii, int keycode, int modifiers) {
#ifdef __SYMBIAN32__
	if (Actions::Instance()->mappingActive()) {
#else
	if (modifiers == 0xff  && Actions::Instance()->mappingActive()) {	// GAPI key was selected
#endif
		char selection[100];

		Actions::Instance()->setMapping((ActionType)_actionSelected, ascii);

		if (ascii != 0)
			sprintf(selection, "Associated key : %s", SDL_GetKeyName((SDLKey) keycode));
		else
			sprintf(selection, "Associated key : none");

		_actionTitle->setLabel("Choose an action to map");
		_keyMapping->setLabel(selection);
		_keyMapping->draw();
		_actionTitle->draw();
		_actionSelected = -1;
		_actionsList->setEnabled(true);
		Actions::Instance()->beginMapping(false);
	} else 
		Dialog::handleKeyUp(ascii,keycode,modifiers);
}

} // namespace GUI
