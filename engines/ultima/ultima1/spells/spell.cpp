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
#include "ultima/ultima1/core/party.h"
#include "ultima/ultima1/core/resources.h"

namespace Ultima {
namespace Ultima1 {
namespace Spells {

Spell::Spell(Ultima1Game *game, Character *c, SpellId spellId) : _game(game),
		_character(c), _spellId(spellId) {
	_name = _game->_res->SPELL_NAMES[spellId];
}

void Spell::addInfoMsg(const Common::String &text, bool newLine, bool replaceLine) {
	Shared::CInfoMsg msg(text, newLine, replaceLine);
	msg.execute("Game");
}

void Spell::cast(Maps::MapBase *map) {
	// Most spells can't be used outside of dungeons
	addInfoMsg("");
	addInfoMsg(_game->_res->DUNGEON_SPELL_ONLY);
	_game->playFX(6);
	_game->endOfTurn();
}

void Spell::dungeonCast(Maps::MapDungeon *map) {
	// This is the fallback the spells call if it fails
	addInfoMsg(_game->_res->FAILED);
	_game->playFX(6);
	_game->endOfTurn();
}

uint Spell::getBuyCost() const {
	return (200 - _character->_wisdom) / 32 * _spellId;
}

} // End of namespace Spells
} // End of namespace Ultima1
} // End of namespace Ultima
