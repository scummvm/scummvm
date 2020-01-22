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

#include "ultima/ultima1/spells/kill_magic_missile.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/party.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map_tile.h"
#include "ultima/ultima1/widgets/dungeon_monster.h"
#include "ultima/shared/maps/map_widget.h"

namespace Ultima {
namespace Ultima1 {
namespace Spells {

BEGIN_MESSAGE_MAP(KillMagicMIssile, Spell)
	ON_MESSAGE(CharacterInputMsg)
END_MESSAGE_MAP()

void KillMagicMIssile::cast(Maps::MapBase *map) {
	// Prompt for a direction
	addInfoMsg(": ", false);
	Shared::CInfoGetKeypress keyMsg(this);
	keyMsg.execute(_game);
}

bool KillMagicMIssile::CharacterInputMsg(CCharacterInputMsg &msg) {
	Shared::Maps::Direction dir = Shared::Maps::MapWidget::directionFromKey(msg._keyState.keycode);
	Character &c = *static_cast<Party *>(_game->_party);

	if (dir == Shared::Maps::DIR_NONE) {
		addInfoMsg(_game->_res->NONE);
		_game->endOfTurn();
	} else {
		addInfoMsg(_game->_res->DIRECTION_NAMES[(int)dir - 1]);
		addInfoMsg(_game->_res->SPELL_PHRASES[_spellId == SPELL_MAGIC_MISSILE ? 12 : 13], false);
		//uint damage = _spellId == SPELL_MAGIC_MISSILE ?
		//	c.equippedWeapon()->getMagicDamage() : 9999;

		if (c._class == CLASS_CLERIC || _game->getRandomNumber(1, 100) < c._wisdom) {
			_game->playFX(5);
			addInfoMsg("");

			// TODO: Non-dungeon damage
			// damage(dir,  7, 3, damage, 101, "SpellEffect");
		} else {
			addInfoMsg(_game->_res->FAILED);
			_game->playFX(6);
			_game->endOfTurn();
		}
	}

	return true;
}

/*-------------------------------------------------------------------*/

Kill::Kill(Ultima1Game *game, Character *c) : KillMagicMIssile(game, c, SPELL_KILL) {
}

void Kill::dungeonCast(Maps::MapDungeon *map) {
	Point newPos;
	Maps::U1MapTile tile;

	newPos = map->getPosition() + map->getDirectionDelta();
	map->getTileAt(newPos, &tile);

	Widgets::DungeonMonster *monster = dynamic_cast<Widgets::DungeonMonster *>(tile._widget);
	if (monster) {
		monster->attackMonster(5, 101, Widgets::ITS_OVER_9000);
		_game->endOfTurn();
	} else {
		// Failed
		KillMagicMIssile::dungeonCast(map);
	}
}

/*-------------------------------------------------------------------*/

MagicMissile::MagicMissile(Ultima1Game *game, Character *c) : KillMagicMIssile(game, c, SPELL_MAGIC_MISSILE) {
}

void MagicMissile::dungeonCast(Maps::MapDungeon *map) {
	Widgets::DungeonMonster *monster = map->findCreatureInCurrentDirection();

	if (monster) {
		Character *c = *static_cast<Party *>(_game->_party);
		uint damage = c->equippedWeapon()->getMagicDamage();
		monster->attackMonster(5, 101, damage);
	} else {
		KillMagicMIssile::dungeonCast(map);
	}
}

} // End of namespace Spells
} // End of namespace Ultima1
} // End of namespace Ultima
