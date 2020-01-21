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

#include "ultima/ultima1/u1dialogs/drop.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/shared/gfx/text_cursor.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

BEGIN_MESSAGE_MAP(Drop, Dialog)
	ON_MESSAGE(KeypressMsg)
	ON_MESSAGE(TextInputMsg)
END_MESSAGE_MAP()

Drop::Drop(Ultima1Game *game) : FullScreenDialog(game), _mode(SELECT), _textInput(game) {
}

bool Drop::KeypressMsg(CKeypressMsg &msg) {
	Shared::Character &c = *_game->_party;

	switch (_mode) {
	case SELECT:
		_game->_textCursor->setVisible(false);

		switch (msg._keyState.keycode) {
		case Common::KEYCODE_p:
			setMode(DROP_PENCE);
			break;
		case Common::KEYCODE_w:
			setMode(DROP_WEAPON);
			break;
		case Common::KEYCODE_a:
			setMode(DROP_ARMOR);
			break;
		default:
			nothing();
			break;
		}
		break;

	case DROP_WEAPON:
		if (msg._keyState.keycode >= Common::KEYCODE_b && msg._keyState.keycode < (Common::KEYCODE_b + (int)c._weapons.size())
			&& c._weapons[msg._keyState.keycode - Common::KEYCODE_a]._quantity > 0) {
			// Drop the weapon
			int weaponNum = msg._keyState.keycode - Common::KEYCODE_a;
			if (--c._weapons[weaponNum]._quantity == 0 && c._equippedWeapon == weaponNum)
				c._equippedWeapon = 0;

			addInfoMsg(Common::String::format(" %s", _game->_res->WEAPON_NAMES_UPPERCASE[weaponNum]));
			hide();
		} else {
			none();
		}
		break;

	case DROP_ARMOR:
		if (msg._keyState.keycode >= Common::KEYCODE_b && msg._keyState.keycode < (Common::KEYCODE_b + (int)c._armor.size())
			&& c._armor[msg._keyState.keycode - Common::KEYCODE_a]._quantity > 0) {
			// Drop the armor
			int armorNum = msg._keyState.keycode - Common::KEYCODE_a;
			if (--c._armor[armorNum]._quantity == 0 && c._equippedArmor == armorNum)
				c._equippedArmor = 0;

			addInfoMsg(Common::String::format(" %s", _game->_res->ARMOR_NAMES[armorNum]));
			hide();
		} else {
			none();
		}
		break;

	default:
		break;
	}

	return true;
}

bool Drop::TextInputMsg(CTextInputMsg &msg) {
	Shared::Character &c = *_game->_party;
	assert(_mode == DROP_PENCE);
	Ultima1Game *game = _game;
	Maps::Ultima1Map *map = getMap();

	uint amount = atoi(msg._text.c_str());

	if (msg._escaped || !amount) {
		none();

	} else {
		addInfoMsg(Common::String::format(" %u", amount));

		if (amount > c._coins) {
			addInfoMsg(game->_res->NOT_THAT_MUCH);
			game->playFX(1);
		} else {
			c._coins -= amount;
			hide();

			map->dropCoins(amount);
		}
	}

	return true;
}

void Drop::setMode(Mode mode) {
	setDirty();
	_mode = mode;

	const Shared::Character &c = *_game->_party;
	switch (mode) {
	case DROP_WEAPON:
		if (c._weapons.hasNothing()) {
			nothing();
		}
		break;

	case DROP_ARMOR:
		if (c._armor.hasNothing()) {
			nothing();
		}
		break;

	default:
		break;
	}
}

void Drop::nothing() {
	addInfoMsg(Common::String::format(" %s", _game->_res->NOTHING));
	hide();
}

void Drop::none() {
	addInfoMsg(Common::String::format(" %s", _game->_res->NONE));
	hide();
}

void Drop::draw() {
	switch (_mode) {
	case SELECT:
		drawSelection();
		break;
	case DROP_PENCE:
		drawDropPence();
		break;
	case DROP_WEAPON:
		drawDropWeapon();
		break;
	case DROP_ARMOR:
		drawDropArmor();
		break;
	}
}

void Drop::drawSelection() {
	Shared::Gfx::VisualSurface s = getSurface();
	s.writeString(_game->_res->DROP_PENCE_WEAPON_ARMOR, TextPoint(1, 24));

	_game->_textCursor->setPosition(TextPoint(1 + strlen(_game->_res->DROP_PENCE_WEAPON_ARMOR), 24));
	_game->_textCursor->setVisible(true);
}

void Drop::drawDropPence() {
	Shared::Gfx::VisualSurface s = getSurface();
	Common::String text = Common::String::format("%s %s: ", _game->_res->ACTION_NAMES[3], _game->_res->DROP_PENCE);
	s.fillRect(TextRect(1, 24, 28, 24), _game->_bgColor);
	s.writeString(_game->_res->DROP_PENCE, TextPoint(1, 24));

	_textInput.show(TextPoint(13, 24), true, 4, _game->_textColor);
}

void Drop::drawDropWeapon() {
	Shared::Gfx::VisualSurface s = getSurface();
	drawFrame(_game->_res->ACTION_NAMES[3]);

	// Count the number of different types of weapons
	const Shared::Character &c = *_game->_party;
	int numLines = 0;
	for (uint idx = 1; idx < c._weapons.size(); ++idx) {
		if (c._weapons[idx]._quantity)
			++numLines;
	}

	// Draw lines for weapons the player has
	int yp = 10 - (numLines / 2);
	for (uint idx = 1; idx < c._weapons.size(); ++idx) {
		if (c._weapons[idx]._quantity) {
			Common::String text = Common::String::format("%c) %s", 'a' + idx,
				_game->_res->WEAPON_NAMES_UPPERCASE[idx]);
			s.writeString(text, TextPoint(15, yp++));
		}
	}

	// Draw drop weapon text at the bottom and enable cursor
	s.fillRect(TextRect(1, 24, 28, 24), _game->_bgColor);
	s.writeString(_game->_res->DROP_WEAPON, TextPoint(1, 24));

	// Show cursor in the info area
	_game->_textCursor->setPosition(TextPoint(1 + strlen(_game->_res->DROP_WEAPON), 24));
	_game->_textCursor->setVisible(true);
}

void Drop::drawDropArmor() {
	Shared::Gfx::VisualSurface s = getSurface();
	drawFrame(_game->_res->ACTION_NAMES[3]);

	// Count the number of different types of armor
	const Shared::Character &c = *_game->_party;
	int numLines = 0;
	for (uint idx = 1; idx < c._armor.size(); ++idx) {
		if (c._armor[idx]._quantity)
			++numLines;
	}

	// Draw lines for armor the player has
	int yp = 10 - (numLines / 2);
	for (uint idx = 1; idx < c._armor.size(); ++idx) {
		if (c._armor[idx]._quantity) {
			Common::String text = Common::String::format("%c) %s", 'a' + idx,
				_game->_res->ARMOR_NAMES[idx]);
			s.writeString(text, TextPoint(13, yp++));
		}
	}

	// Draw drop armor text at the bottom and enable cursor
	s.fillRect(TextRect(1, 24, 28, 24), _game->_bgColor);
	s.writeString(_game->_res->DROP_ARMOR, TextPoint(1, 24));

	// Show cursor in the info area
	_game->_textCursor->setPosition(TextPoint(1 + strlen(_game->_res->DROP_ARMOR), 24));
	_game->_textCursor->setVisible(true);
}

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima
