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

#include "gui/KeysDialog.h"

#ifdef GUI_ENABLE_KEYSDIALOG

#warning The actions system is deprecated. Please use the keymapper instead.

#include "gui/Actions.h"
#include "common/translation.h"
#include <SDL_keyboard.h>

namespace GUI {

enum {
	kMapCmd	= 'map '
};

KeysDialog::KeysDialog(const Common::U32String &title)
	: GUI::Dialog("KeysDialog") {

	new ButtonWidget(this, "KeysDialog.Map", _("Map"), Common::U32String(), kMapCmd);
	new ButtonWidget(this, "KeysDialog.Ok", _("OK"), Common::U32String(), kOKCmd);
	new ButtonWidget(this, "KeysDialog.Cancel", _("Cancel"), Common::U32String(), kCloseCmd);

	_actionsList = new ListWidget(this, "KeysDialog.List");
	_actionsList->setNumberingMode(kListNumberingZero);

	_actionTitle = new StaticTextWidget(this, "KeysDialog.Action", title);
	_keyMapping = new StaticTextWidget(this, "KeysDialog.Mapping", _("Select an action and click 'Map'"));

	_actionTitle->setFlags(WIDGET_CLEARBG);
	_keyMapping->setFlags(WIDGET_CLEARBG);

	// Get actions names
	ListWidget::StringArray l;

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
			Common::U32String selection;

			uint16 key = Actions::Instance()->getMapping(_actionsList->getSelected());
#ifdef __SYMBIAN32__
			// ScummVM mappings for F1-F9 are different from SDL so remap back to sdl
			if (key >= Common::ASCII_F1 && key <= Common::ASCII_F9)
				key = key - Common::ASCII_F1 + SDLK_F1;
#endif
			if (key != 0)
				selection = Common::U32String::format(_("Associated key : %s"), SDL_GetKeyName((SDLKey)key));
			else
				selection = Common::U32String::format(_("Associated key : none"));

			_keyMapping->setLabel(selection);
			_keyMapping->markAsDirty();
		}
		break;
	case kMapCmd:
		if (_actionsList->getSelected() < 0) {
			_actionTitle->setLabel(_("Please select an action"));
		} else {
			Common::U32String selection;

			_actionSelected = _actionsList->getSelected();
			uint16 key = Actions::Instance()->getMapping(_actionSelected);
#ifdef __SYMBIAN32__
			// ScummVM mappings for F1-F9 are different from SDL so remap back to sdl
			if (key >= Common::ASCII_F1 && key <= Common::ASCII_F9)
				key = key - Common::ASCII_F1 + SDLK_F1;
#endif
			if (key != 0)
				selection = Common::U32String::format(_("Associated key : %s"), SDL_GetKeyName((SDLKey)key));
			else
				selection = Common::U32String::format(_("Associated key : none"));

			_actionTitle->setLabel(_("Press the key to associate"));
			_keyMapping->setLabel(selection);
			_keyMapping->markAsDirty();
			Actions::Instance()->beginMapping(true);
			_actionsList->setEnabled(false);
		}
		_actionTitle->markAsDirty();
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

void KeysDialog::handleKeyDown(Common::KeyState state){
	if (!Actions::Instance()->mappingActive())
		Dialog::handleKeyDown(state);
}

void KeysDialog::handleKeyUp(Common::KeyState state) {
	if (Actions::Instance()->mappingActive()) {
		Common::U32String selection;

		Actions::Instance()->setMapping((ActionType)_actionSelected, state.ascii);

		if (state.ascii != 0)
			selection = Common::U32String::format(_("Associated key : %s"), SDL_GetKeyName((SDLKey) state.keycode));
		else
			selection = Common::U32String::format(_("Associated key : none"));

		_actionTitle->setLabel(_("Choose an action to map"));
		_keyMapping->setLabel(selection);
		_keyMapping->markAsDirty();
		_actionTitle->markAsDirty();
		_actionSelected = -1;
		_actionsList->setEnabled(true);
		Actions::Instance()->beginMapping(false);
	} else
		Dialog::handleKeyUp(state);
}

} // namespace GUI

#endif
