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

#include "ultima/ultima1/u1dialogs/ready.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/shared/gfx/text_cursor.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

BEGIN_MESSAGE_MAP(Ready, Dialog)
	ON_MESSAGE(ShowMsg)
	ON_MESSAGE(CharacterInputMsg)
END_MESSAGE_MAP()

Ready::Ready(Ultima1Game *game) : FullScreenDialog(game), _mode(SELECT) {
}

bool Ready::ShowMsg(CShowMsg &msg) {
	addInfoMsg(_game->_res->READY_WEAPON_armour_SPELL, false);
	getKeypress();
	return true;
}

bool Ready::CharacterInputMsg(CCharacterInputMsg &msg) {
	Shared::Character &c = *_game->_party;

	switch (_mode) {
	case SELECT:
		switch (msg._keyState.keycode) {
		case Common::KEYCODE_w:
			setMode(READY_WEAPON);
			break;
		case Common::KEYCODE_a:
			setMode(READY_armour);
			break;
		case Common::KEYCODE_s:
			setMode(READY_SPELL);
			break;
		default:
			addInfoMsg(Common::String::format("%s ", _game->_res->ACTION_NAMES[17]), false, true);
			nothing();
			break;
		}
		break;

	case READY_WEAPON:
		if (msg._keyState.keycode >= Common::KEYCODE_a && msg._keyState.keycode < (Common::KEYCODE_a + (int)c._weapons.size())) {
			int index = msg._keyState.keycode - Common::KEYCODE_a;
			if (!c._weapons[index]->empty())
				c._equippedWeapon = index;
		}

		addInfoMsg(Common::String::format("%s %s: %s", _game->_res->ACTION_NAMES[17],
			_game->_res->WEAPON_armour_SPELL[0], c.equippedWeapon()->_longName.c_str()),
			true, true);
		hide();
		break;

	case READY_armour:
		if (msg._keyState.keycode >= Common::KEYCODE_a && msg._keyState.keycode < (Common::KEYCODE_a + (int)c._armour.size())) {
			int index = msg._keyState.keycode - Common::KEYCODE_a;
			if (!c._armour[index]->empty())
				c._equippedArmour = index;
		}

		addInfoMsg(Common::String::format("%s %s: %s", _game->_res->ACTION_NAMES[17],
			_game->_res->WEAPON_armour_SPELL[1], c.equippedArmour()->_name.c_str()),
			true, true);
		hide();
		break;

	case READY_SPELL:
		if (msg._keyState.keycode >= Common::KEYCODE_a && msg._keyState.keycode < (Common::KEYCODE_a + (int)c._spells.size())) {
			int index = msg._keyState.keycode - Common::KEYCODE_a;
			if (!c._spells[index]->empty())
				c._equippedSpell = index;
		}

		addInfoMsg(Common::String::format("%s %s: %s", _game->_res->ACTION_NAMES[17],
			_game->_res->WEAPON_armour_SPELL[2], c._spells[c._equippedSpell]->_name.c_str()),
			true, true);
		hide();
		break;

	default:
		break;
	}

	return true;
}

void Ready::setMode(Mode mode) {
	setDirty();
	_mode = mode;

	const Shared::Character &c = *_game->_party;
	switch (mode) {
	case READY_WEAPON:
		if (c._weapons.hasNothing()) {
			nothing();
		} else {
			addInfoMsg(Common::String::format("%s %s: ", _game->_res->ACTION_NAMES[17],
				_game->_res->WEAPON_armour_SPELL[0]), false, true);
			getKeypress();
		}
		break;

	case READY_armour:
		if (c._armour.hasNothing()) {
			nothing();
		} else {
			addInfoMsg(Common::String::format("%s %s: ", _game->_res->ACTION_NAMES[17],
				_game->_res->WEAPON_armour_SPELL[1]), false, true);
			getKeypress();
		}
		break;

	case READY_SPELL:
		addInfoMsg(Common::String::format("%s %s: ", _game->_res->ACTION_NAMES[17],
			_game->_res->WEAPON_armour_SPELL[2]), false, true);
		getKeypress();
		break;

	default:
		break;
	}
}

void Ready::nothing() {
	addInfoMsg(_game->_res->NOTHING);
	hide();
}

void Ready::none() {
	addInfoMsg(Common::String::format(" %s", _game->_res->NONE));
	hide();
}

void Ready::draw() {
	Dialog::draw();

	switch (_mode) {
	case READY_WEAPON:
		drawReadyWeapon();
		break;
	case READY_armour:
		drawReadyArmor();
		break;
	case READY_SPELL:
		drawReadySpell();
		break;
	default:
		break;
	}
}

void Ready::drawReadyWeapon() {
	Shared::Gfx::VisualSurface s = getSurface();
	drawFrame(_game->_res->ACTION_NAMES[17]);

	// Count the number of different types of weapons
	const Shared::Character &c = *_game->_party;
	int numLines = 0;
	for (uint idx = 0; idx < c._weapons.size(); ++idx) {
		if (!c._weapons[idx]->empty())
			++numLines;
	}

	// Draw lines for weapons the player has
	int yp = 10 - (numLines / 2);
	for (uint idx = 0; idx < c._weapons.size(); ++idx) {
		if (!c._weapons[idx]->empty()) {
			Common::String text = Common::String::format("%c) %s", 'a' + idx, c._weapons[idx]->_longName.c_str());
			s.writeString(text, TextPoint(15, yp++), (int)idx == c._equippedWeapon ? _game->_highlightColor : _game->_textColor);
		}
	}
}

void Ready::drawReadyArmor() {
	Shared::Gfx::VisualSurface s = getSurface();
	drawFrame(_game->_res->ACTION_NAMES[17]);

	// Count the number of different types of weapons
	const Shared::Character &c = *_game->_party;
	int numLines = 0;
	for (uint idx = 0; idx < c._armour.size(); ++idx) {
		if (!c._armour[idx]->empty())
			++numLines;
	}

	// Draw lines for armor the player has
	int yp = 10 - (numLines / 2);
	for (uint idx = 0; idx < c._armour.size(); ++idx) {
		if (!c._armour[idx]->empty()) {
			Common::String text = Common::String::format("%c) %s", 'a' + idx, c._armour[idx]->_name.c_str());
			s.writeString(text, TextPoint(15, yp++), (int)idx == c._equippedArmour ? _game->_highlightColor : _game->_textColor);
		}
	}
}

void Ready::drawReadySpell() {
	Shared::Gfx::VisualSurface s = getSurface();
	drawFrame(_game->_res->ACTION_NAMES[17]);

	// Count the number of different types of spells
	const Shared::Character &c = *_game->_party;
	int numLines = 0;
	for (uint idx = 0; idx < c._spells.size(); ++idx) {
		if (c._spells[idx]->_quantity)
			++numLines;
	}

	// Draw lines for weapons the player has
	int yp = 10 - (numLines / 2);
	for (uint idx = 0; idx < c._spells.size(); ++idx) {
		if (c._spells[idx]->_quantity) {
			Common::String text = Common::String::format("%c) %s", 'a' + idx, c._spells[idx]->_name.c_str());
			s.writeString(text, TextPoint(15, yp++), (int)idx == c._equippedSpell ? _game->_highlightColor : _game->_textColor);
		}
	}
}

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima
