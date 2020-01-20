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
#include "ultima/shared/gfx/text_cursor.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

BEGIN_MESSAGE_MAP(Drop, Dialog)
	ON_MESSAGE(KeypressMsg)
END_MESSAGE_MAP()

Drop::Drop(Ultima1Game *game) : FullScreenDialog(game), _mode(SELECT) {
}

bool Drop::KeypressMsg(CKeypressMsg &msg) {
	switch (_mode) {
	case SELECT:
		_game->_textCursor->setVisible(false);

		switch (msg._keyState.keycode) {
		case Common::KEYCODE_p:
			setMode(DROP_PENCE);
			setDirty();
			break;
		case Common::KEYCODE_w:
			setMode(DROP_WEAPON);
			setDirty();
			break;
		case Common::KEYCODE_a:
			setMode(DROP_ARMOR);
			setDirty();
			break;
		default:
			nothing();
			break;
		}
		break;

	default:
		break;
	}

	return true;
}

void Drop::setMode(Mode mode) {
	_mode = mode;
	const Shared::Character &c = *_game->_party._currentCharacter;

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
	s.writeString(_game->_res->DROP_PENCE_WEAPON_ARMOR, TextPoint(1, 24), _game->_textColor);

	_game->_textCursor->setPosition(TextPoint(1 + strlen(_game->_res->DROP_PENCE_WEAPON_ARMOR), 24));
	_game->_textCursor->setVisible(true);
}

void Drop::drawDropPence() {
	Shared::Gfx::VisualSurface s = getSurface();
	Common::String text = Common::String::format("%s %s: ", _game->_res->ACTION_NAMES[3], _game->_res->DROP_PENCE);
	s.fillRect(TextRect(1, 24, 28, 24), _game->_bgColor);
	s.writeString(_game->_res->DROP_ARMOR, TextPoint(1, 24), _game->_textColor);

	_game->_textCursor->setPosition(TextPoint(1 + strlen(_game->_res->DROP_PENCE), 24));
	_game->_textCursor->setVisible(true);
}

void Drop::drawDropWeapon() {
	Shared::Gfx::VisualSurface s = getSurface();
	drawFrame(_game->_res->ACTION_NAMES[3]);

	// Count the number of different types of weapons
	const Shared::Character &c = *_game->_party._currentCharacter;
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
			s.writeString(text, TextPoint(15, yp++), _game->_textColor);
		}
	}

	// Draw drop weapon text at the bottom and enable cursor
	s.fillRect(TextRect(1, 24, 28, 24), _game->_bgColor);
	s.writeString(_game->_res->DROP_WEAPON, TextPoint(1, 24), _game->_textColor);

	// Show cursor in the info area
	_game->_textCursor->setPosition(TextPoint(1 + strlen(_game->_res->DROP_WEAPON), 24));
	_game->_textCursor->setVisible(true);
}

void Drop::drawDropArmor() {
	Shared::Gfx::VisualSurface s = getSurface();
	drawFrame(_game->_res->ACTION_NAMES[3]);

	// Count the number of different types of armor
	const Shared::Character &c = *_game->_party._currentCharacter;
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
			s.writeString(text, TextPoint(13, yp++), _game->_textColor);
		}
	}

	// Draw drop armor text at the bottom and enable cursor
	s.fillRect(TextRect(1, 24, 28, 24), _game->_bgColor);
	s.writeString(_game->_res->DROP_ARMOR, TextPoint(1, 24), _game->_textColor);

	// Show cursor in the info area
	_game->_textCursor->setPosition(TextPoint(1 + strlen(_game->_res->DROP_ARMOR), 24));
	_game->_textCursor->setVisible(true);
}

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima
