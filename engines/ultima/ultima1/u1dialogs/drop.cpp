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

Drop::Drop(Ultima1Game *game) : Dialog(game), _mode(SELECT) {
	// The drop dialog covers the entire screen, but doesn't erase what's under it
	_bounds = Common::Rect(0, 0, 320, 200);
}

bool Drop::KeypressMsg(CKeypressMsg &msg) {
	switch (_mode) {
	case SELECT:
		_game->_textCursor->setVisible(false);

		switch (msg._keyState.keycode) {
		case Common::KEYCODE_p:
			_mode = DROP_PENCE;
			setDirty();
			break;
		case Common::KEYCODE_w:
			_mode = DROP_WEAPON;
			setDirty();
			break;
		case Common::KEYCODE_a:
			_mode = DROP_ARMOR;
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

void Drop::nothing() {
	addInfoMsg(Common::String::format(" %s", _game->_res->NOTHING));
	hide();
	delete this;
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
	// TODO
}

void Drop::drawDropWeapon() {
	// TODO
}

void Drop::drawDropArmor() {
	// TODO
}

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima
