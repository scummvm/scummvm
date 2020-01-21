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
	ON_MESSAGE(KeypressMsg)
END_MESSAGE_MAP()

Ready::Ready(Ultima1Game *game) : FullScreenDialog(game), _mode(SELECT) {
}

bool Ready::KeypressMsg(CKeypressMsg &msg) {
	Shared::Character &c = *_game->_party;

	switch (_mode) {
	case SELECT:
		_game->_textCursor->setVisible(false);

		switch (msg._keyState.keycode) {
		case Common::KEYCODE_w:
			setMode(READY_WEAPON);
			break;
		case Common::KEYCODE_a:
			setMode(READY_ARMOR);
			break;
		case Common::KEYCODE_s:
			setMode(READY_SPELL);
			break;
		default:
			nothing();
			break;
		}
		break;

	case READY_WEAPON:
		if (msg._keyState.keycode >= Common::KEYCODE_a && msg._keyState.keycode < (Common::KEYCODE_a + (int)c._weapons.size()))
			c._equippedWeapon = msg._keyState.keycode - Common::KEYCODE_a;

		addInfoMsg(Common::String::format(" %s: %s", _game->_res->WEAPON_ARMOR_SPELL[0],
			c._weapons[c._equippedWeapon]._longName.c_str()));
		hide();
		break;

	case READY_ARMOR:
		if (msg._keyState.keycode >= Common::KEYCODE_a && msg._keyState.keycode < (Common::KEYCODE_a + (int)c._armor.size()))
			c._equippedArmor = msg._keyState.keycode - Common::KEYCODE_a;

		addInfoMsg(Common::String::format(" %s: %s", _game->_res->WEAPON_ARMOR_SPELL[1],
			c._armor[c._equippedArmor]._name.c_str()));
		hide();
		break;

	case READY_SPELL:
		if (msg._keyState.keycode >= Common::KEYCODE_a && msg._keyState.keycode < (Common::KEYCODE_a + (int)c._spells.size()))
			c._equippedSpell = msg._keyState.keycode - Common::KEYCODE_a;

		addInfoMsg(Common::String::format(" %s: %s", _game->_res->WEAPON_ARMOR_SPELL[2],
			c._spells[c._equippedSpell]->_name.c_str()));
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
		}
		break;

	case READY_ARMOR:
		if (c._armor.hasNothing()) {
			nothing();
		}
		break;

	default:
		break;
	}
}

void Ready::nothing() {
	addInfoMsg(Common::String::format(" %s", _game->_res->NOTHING));
	hide();
}

void Ready::none() {
	addInfoMsg(Common::String::format(" %s", _game->_res->NONE));
	hide();
}

void Ready::draw() {
	switch (_mode) {
	case SELECT:
		drawSelection();
		break;
	case READY_WEAPON:
		drawReadyWeapon();
		break;
	case READY_ARMOR:
		drawReadyArmor();
		break;
	case READY_SPELL:
		drawReadySpell();
		break;
	}
}

void Ready::drawSelection() {
	Shared::Gfx::VisualSurface s = getSurface();
	s.writeString(_game->_res->READY_WEAPON_ARMOR_SPELL, TextPoint(1, 24));

	_game->_textCursor->setPosition(TextPoint(1 + strlen(_game->_res->READY_WEAPON_ARMOR_SPELL), 24));
	_game->_textCursor->setVisible(true);
}

void Ready::drawReadyWeapon() {
	Shared::Gfx::VisualSurface s = getSurface();
	drawFrame(_game->_res->ACTION_NAMES[17]);

	// Count the number of different types of weapons
	const Shared::Character &c = *_game->_party;
	int numLines = 0;
	for (uint idx = 0; idx < c._weapons.size(); ++idx) {
		if (c._weapons[idx]._quantity)
			++numLines;
	}

	// Draw lines for weapons the player has
	int yp = 10 - (numLines / 2);
	for (uint idx = 0; idx < c._weapons.size(); ++idx) {
		if (c._weapons[idx]._quantity) {
			Common::String text = Common::String::format("%c) %s", 'a' + idx, c._weapons[idx]._longName.c_str());
			s.writeString(text, TextPoint(15, yp++), (int)idx == c._equippedWeapon ? _game->_highlightColor : _game->_textColor);
		}
	}

	// Draw Ready weapon text at the bottom and enable cursor
	s.fillRect(TextRect(1, 24, 28, 24), _game->_bgColor);
	Common::String line = Common::String::format("%s %s: ", _game->_res->ACTION_NAMES[17],
		_game->_res->WEAPON_ARMOR_SPELL[0]);
	s.writeString(line, TextPoint(1, 24));

	// Show cursor in the info area
	_game->_textCursor->setPosition(TextPoint(1 + line.size(), 24));
	_game->_textCursor->setVisible(true);
}

void Ready::drawReadyArmor() {
	Shared::Gfx::VisualSurface s = getSurface();
	drawFrame(_game->_res->ACTION_NAMES[17]);

	// Count the number of different types of weapons
	const Shared::Character &c = *_game->_party;
	int numLines = 0;
	for (uint idx = 0; idx < c._armor.size(); ++idx) {
		if (c._armor[idx]._quantity)
			++numLines;
	}

	// Draw lines for armor the player has
	int yp = 10 - (numLines / 2);
	for (uint idx = 0; idx < c._armor.size(); ++idx) {
		if (c._armor[idx]._quantity) {
			Common::String text = Common::String::format("%c) %s", 'a' + idx, c._armor[idx]._name.c_str());
			s.writeString(text, TextPoint(15, yp++), (int)idx == c._equippedArmor ? _game->_highlightColor : _game->_textColor);
		}
	}

	// Draw Ready weapon text at the bottom and enable cursor
	s.fillRect(TextRect(1, 24, 28, 24), _game->_bgColor);
	Common::String line = Common::String::format("%s %s: ", _game->_res->ACTION_NAMES[17],
		_game->_res->WEAPON_ARMOR_SPELL[1]);
	s.writeString(line, TextPoint(1, 24));

	// Show cursor in the info area
	_game->_textCursor->setPosition(TextPoint(1 + line.size(), 24));
	_game->_textCursor->setVisible(true);
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

	// Draw Ready weapon text at the bottom and enable cursor
	s.fillRect(TextRect(1, 24, 28, 24), _game->_bgColor);
	Common::String line = Common::String::format("%s %s: ", _game->_res->ACTION_NAMES[17],
		_game->_res->WEAPON_ARMOR_SPELL[2]);
	s.writeString(line, TextPoint(1, 24));

	// Show cursor in the info area
	_game->_textCursor->setPosition(TextPoint(1 + line.size(), 24));
	_game->_textCursor->setVisible(true);
}

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima
