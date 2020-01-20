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

#include "ultima/ultima1/spells/spell.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/resources.h"

namespace Ultima {
namespace Ultima1 {
namespace Spells {

void Spell::addInfoMsg(const Common::String &text, bool newLine) {
	Shared::CInfoMsg msg(text, newLine);
	msg.execute(_game->getView());
}

void Spell::cast() {
	addInfoMsg("");
	addInfoMsg(_game->_res->DUNGEON_SPELL_ONLY);
	_game->playFX(6);
}

} // End of namespace Spells
} // End of namespace Ultima1
} // End of namespace Ultima
