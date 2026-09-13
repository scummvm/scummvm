/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ultima/ultima2/data/savegame.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

void Savegame::synchronize(Common::Serializer &s) {
	s.syncBytes((byte *)_name, MAX_NAME_LENGTH + 1);
	s.syncAsByte(_sex);
	s.syncAsByte(_race);
	s.syncAsByte(_class);

	s.syncAsSint16LE(_strength);
	s.syncAsSint16LE(_agility);
	s.syncAsSint16LE(_stamina);
	s.syncAsSint16LE(_charisma);
	s.syncAsSint16LE(_wisdom);
	s.syncAsSint16LE(_intelligence);

	s.syncAsSint16LE(_hp);
	s.syncAsSint16LE(_food);
	s.syncAsSint16LE(_foodTurnCtr);
	s.syncAsSint16LE(_experience);
	s.syncAsSint16LE(_gold);

	s.syncAsByte(_mapNum1);
	s.syncAsByte(_mapNum2);
	s.syncAsByte(_mapX);
	s.syncAsByte(_mapY);
	s.syncAsByte(_overworldReturnX);
	s.syncAsByte(_overworldReturnY);

	s.syncAsByte(_readiedWeapon);
	s.syncAsByte(_readiedArmor);
	s.syncAsByte(_readiedSpell);

	s.syncAsSint16LE(_torches);
	s.syncAsSint16LE(_keys);
	s.syncAsSint16LE(_thievesTools);

	s.syncAsByte(_inSpace);
	s.syncAsByte(_launchMapX);
	s.syncAsByte(_launchMapY);

	s.syncAsByte(_saveDisabled);
	s.syncAsSint16LE(_orbitTarget);

	s.syncAsByte(_ringQuestFlag);
	s.syncAsByte(_enilnoOwned);
	s.syncAsByte(_patrolWaypoint);

	for (int i = 0; i < 9; ++i)
		s.syncAsByte(_offerRewardItems[i]);

	for (int i = 0; i < ARMOR_COUNT; ++i)
		s.syncAsSint16LE(_armorOwned[i]);
	for (int i = 0; i < WEAPON_COUNT; ++i)
		s.syncAsSint16LE(_weaponOwned[i]);
	for (int i = 0; i < SPELL_COUNT; ++i)
		s.syncAsSint16LE(_spellCharges[i]);
	for (int i = 0; i < ITEM_COUNT; ++i)
		s.syncAsSint16LE(_items[i]);

	s.syncAsSint16LE(_legParalysisTurns);
	s.syncAsSint16LE(_armParalysisTurns);
	s.syncAsSint16LE(_sleepTurns);
	s.syncAsSint16LE(_negateTimeTurns);
	s.syncAsSint16LE(_lightTurns);
	s.syncAsByte(_monsterSpawnCounter);
}

} // namespace Data
} // namespace Ultima2
} // namespace Ultima
