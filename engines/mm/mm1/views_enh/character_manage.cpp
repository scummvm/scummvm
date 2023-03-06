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
}

bool CharacterManage::msgFocus(const FocusMessage &msg) {
	CharacterBase::msgFocus(msg);
	_changed = false;
	return true;
}

bool CharacterManage::msgUnfocus(const UnfocusMessage &msg) {
	if (_changed)
		g_globals->_roster.save();
	CharacterBase::msgUnfocus(msg);
	return true;
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
		writeString(80, 172, STRING["dialogs.view_character.name"]);
		_textEntry.display(130, 180, 15, false,
			[]() {
				CharacterManage *view = static_cast<CharacterManage *>(g_events->focusedView());
				view->setMode(DISPLAY);
			},
			[](const Common::String &name) {
				CharacterManage *view = static_cast<CharacterManage *>(g_events->focusedView());
				view->setName(name);
			}
		);
		break;

	case DELETE:
		writeString(120, 174, STRING["enhdialogs.character.are_you_sure"]);
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
	} else if (msg._action == KEYBIND_SELECT && _state == RENAME) {
		Common::strcpy_s(c._name, _newName.c_str());
		c._name[15] = '\0';
		setMode(DISPLAY);
		return true;
	}

	return CharacterBase::msgAction(msg);
}

void CharacterManage::setMode(ViewState state) {
	_state = state;

	for (int i = 0; i < 4; ++i)
		setButtonEnabled(i, state == DISPLAY);
	redraw();
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
