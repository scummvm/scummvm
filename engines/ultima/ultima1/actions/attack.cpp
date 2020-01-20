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

#include "ultima/ultima1/actions/attack.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/widgets/transport.h"
#include "ultima/ultima1/core/resources.h"

namespace Ultima {
namespace Ultima1 {
namespace Actions {

BEGIN_MESSAGE_MAP(Attack, Action)
	ON_MESSAGE(AttackMsg)
END_MESSAGE_MAP()

bool Attack::AttackMsg(CAttackMsg &msg) {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	const Shared::Character &c = *getGame()->_party._currentCharacter;
	const Shared::Weapon &weapon = c._weapons[c._equippedWeapon];

	addInfoMsg(Common::String::format("%s %s", game->_res->ACTION_NAMES[0], weapon._shortName.c_str()), false);
	
	if (weapon._distance == 0) {
		addInfoMsg("?");
		game->playFX(1);
	} else {
		getMap()->attack(msg._direction);
	}

	return true;
}

} // End of namespace Actions
} // End of namespace Ultima1
} // End of namespace Ultima
