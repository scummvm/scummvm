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

#include "ultima/ultima1/spells/prayer.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/widgets/overworld_monster.h"

namespace Ultima {
namespace Ultima1 {
namespace Spells {

Prayer::Prayer(Ultima1Game *game, Character *c) : Spell(game, c, SPELL_PRAYER) {
	_quantity = 0xffff;			// Prayer has unlimited uses
}

void Prayer::cast(Maps::MapBase *map) {
	Shared::Character &c = *_game->_party;
	addInfoMsg("");
	addInfoMsg(_game->_res->SPELL_PHRASES[0], false);

	bool flag = false;
	if (c._hitPoints < 15) {
		// Add hit points
		c._hitPoints = 15;
		addInfoMsg(Common::String::format(" %s", _game->_res->SPELL_PHRASES[11]));
		_game->playFX(5);
		flag = true;
	} else if (c._food < 15) {
		// Add food
		c._food = 15;
		addInfoMsg(Common::String::format(" %s", _game->_res->SPELL_PHRASES[11]));
		_game->playFX(5);
		flag = true;
	} else if (_game->getRandomNumber(1, 100) < 25) {
		for (uint idx = 0; idx < map->_widgets.size(); ++idx) {
			Widgets::OverworldMonster *monster = dynamic_cast<Widgets::OverworldMonster *>(map->_widgets[idx].get());
			if (monster && monster->attackDistance() != 0) {
				map->removeWidget(monster);
				addInfoMsg(_game->_res->MONSTER_REMOVED);
				flag = true;
				break;
			}
		}
	}

	if (flag) {
		addInfoMsg(_game->_res->NO_EFFECT);
		_game->playFX(6);
	}

	_game->endOfTurn();
}

void Prayer::dungeonCast(Maps::MapDungeon *map) {
	addInfoMsg("");
	addInfoMsg(_game->_res->SPELL_PHRASES[0]);

	// When cast within the dungeon, cast a random spell without cost
	SpellId spellId = (SpellId)_game->getRandomNumber(SPELL_OPEN, SPELL_KILL);
	if (spellId == SPELL_STEAL)
		spellId = SPELL_LADDER_DOWN;

	const Shared::Character &c = *_game->_party;
	static_cast<Spell *>(c._spells[spellId])->dungeonCast(map);
}

} // End of namespace Spells
} // End of namespace Ultima1
} // End of namespace Ultima
