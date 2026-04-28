/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mm/mm1/views_enh/character_manage.h"
#include "mm/mm1/utils/strings.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

CharacterManage::CharacterManage() : CharacterBase("CharacterManage") {
	addButton(&_escSprite, Common::Point(20, 172), 0, Common::KEYCODE_p, true);
	addButton(&_escSprite, Common::Point(90, 172), 0, Common::KEYCODE_r, true);
	addButton(&_escSprite, Common::Point(160, 172), 0, Common::KEYCODE_d, true);
	addButton(&_escSprite, Common::Point(230, 172), 0, KEYBIND_ESCAPE, true);
	addButton(&g_globals->_confirmIcons, Common::Point(240, 163), 0,
		Common::KeyState(Common::KEYCODE_y, 'y'));
	addButton(&g_globals->_confirmIcons, Common::Point(270, 163), 2,
		Common::KeyState(Common::KEYCODE_n, 'n'));

	setButtonEnabled(4, false);
	setButtonEnabled(5, false);
}

bool CharacterManage::msgFocus(const FocusMessage &msg) {
	CharacterBase::msgFocus(msg);
	setMode(DISPLAY);
	_changed = false;
	return true;
}

bool CharacterManage::msgUnfocus(const UnfocusMessage &msg) {
	if (_changed)
		g_globals->_roster.save();
	CharacterBase::msgUnfocus(msg);
	return true;
}

void CharacterManage::abortFunc() {
	CharacterManage *view = static_cast<CharacterManage *>(g_events->focusedView());
	view->setMode(DISPLAY);
}

void CharacterManage::enterFunc(const Common::String &name) {
	CharacterManage *view = static_cast<CharacterManage *>(g_events->focusedView());
	view->setName(name);
}

void CharacterManage::draw() {
	assert(g_globals->_currCharacter);
	setReduced(false);

	CharacterBase::draw();

	switch (_state) {
	case DISPLAY:
		setReduced(true);
		writeString(35, 174, STRING["enhdialogs.character.portrait"]);
		writeString(105, 174, STRING["enhdialogs.character.rename"]);
		writeString(175, 174, STRING["enhdialogs.character.delete"]);
		writeString(245, 174, STRING["enhdialogs.misc.go_back"]);
		break;

	case RENAME:
		_state = DISPLAY;
		writeString(80, 172, STRING["dialogs.view_character.name"]);
		_textEntry.display(130, 180, 15, false, abortFunc, enterFunc);
		break;

	case DELETE:
		writeString(110, 171, STRING["enhdialogs.character.are_you_sure"]);
		break;
	}
}

bool CharacterManage::msgKeypress(const KeypressMessage &msg) {
	Character &c = *g_globals->_currCharacter;

	switch (_state) {
	case DISPLAY:
		switch (msg.keycode) {
		case Common::KEYCODE_p:
			c._portrait = (c._portrait + 1) % NUM_PORTRAITS;
			c.loadFaceSprites();
			redraw();
			break;
		case Common::KEYCODE_r:
			setMode(RENAME);
			break;
		case Common::KEYCODE_d:
			setMode(DELETE);
			break;
		default:
			break;
		}
		break;

	case RENAME:
		break;

	case DELETE:
		switch (msg.keycode) {
		case Common::KEYCODE_y:
			msgAction(ActionMessage(KEYBIND_SELECT));
			break;
		case Common::KEYCODE_n:
			msgAction(ActionMessage(KEYBIND_ESCAPE));
			break;
		default:
			break;
		}
		break;
	}

	return true;
}

bool CharacterManage::msgAction(const ActionMessage &msg) {
	Character &c = *g_globals->_currCharacter;

	if (msg._action == KEYBIND_ESCAPE) {
		switch (_state) {
		case DISPLAY:
			close();
			break;
		default:
			setMode(DISPLAY);
			break;
		}

		return true;
	} else if (msg._action == KEYBIND_SELECT) {
		if (_state == RENAME) {
			Common::strcpy_s(c._name, _newName.c_str());
			c._name[15] = '\0';
			setMode(DISPLAY);
			return true;
		} else if (_state == DELETE) {
			// Removes the character and returns to View All Characters
			g_globals->_roster.remove(g_globals->_currCharacter);
			_changed = true;
			setMode(DISPLAY);
			close();
		}
	}

	return CharacterBase::msgAction(msg);
}

void CharacterManage::setMode(ViewState state) {
	_state = state;
	resetSelectedButton();

	for (int i = 0; i < 4; ++i)
		setButtonEnabled(i, state == DISPLAY);
	setButtonEnabled(4, state == DELETE);
	setButtonEnabled(5, state == DELETE);
	redraw();
}

bool CharacterManage::msgMouseMove(const MouseMoveMessage &msg) {
	int selectedButton = _state == DELETE ? getButtonAt(msg._pos) : -1;
	if (selectedButton != _selectedButton) {
		_selectedButton = selectedButton;
		redraw();
	}

	return true;
}

void CharacterManage::setName(const Common::String &newName) {
	Character &c = *g_globals->_currCharacter;
	Common::strcpy_s(c._name, newName.c_str());
	c._name[15] = '\0';
	_changed = true;

	setMode(DISPLAY);
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
