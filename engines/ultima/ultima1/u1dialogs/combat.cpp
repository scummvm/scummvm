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

#include "ultima/ultima1/u1dialogs/combat.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/shared/gfx/text_cursor.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

BEGIN_MESSAGE_MAP(Combat, Dialog)
	ON_MESSAGE(KeypressMsg)
END_MESSAGE_MAP()

Combat::Combat(Ultima1Game *game, Shared::Maps::Direction direction, int weaponType,
	const Common::String weaponName) : FullScreenDialog(game), _direction(direction) {
}

bool Combat::KeypressMsg(CKeypressMsg &msg) {
	if (_direction == Shared::Maps::DIR_NONE) {
		switch (msg._keyState.keycode) {
		case Common::KEYCODE_LEFT:
		case Common::KEYCODE_KP4:
			_direction = Shared::Maps::DIR_LEFT;
			break;
		case Common::KEYCODE_RIGHT:
		case Common::KEYCODE_KP6:
			_direction = Shared::Maps::DIR_RIGHT;
			break;
		case Common::KEYCODE_UP:
		case Common::KEYCODE_KP8:
			_direction = Shared::Maps::DIR_UP;
			break;
		case Common::KEYCODE_DOWN:
		case Common::KEYCODE_KP2:
			_direction = Shared::Maps::DIR_DOWN;
			break;
		default:
			nothing();
			return true;
		}
	}

	setDirty(true);
	return true;
}


void Combat::draw() {
	if (_direction == Shared::Maps::DIR_NONE)
		drawSelection();
}

void Combat::drawSelection() {

}

void Combat::nothing() {
	addInfoMsg(Common::String::format(" %s", _game->_res->NOTHING));
	hide();
}

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima
