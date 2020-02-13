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

#include "ultima/shared/core/party.h"

namespace Ultima {
namespace Shared {

template<typename T>
void syncArray(Common::Array<T *> &items, Common::Serializer &s) {
	uint count = items.size();
	s.syncAsByte(count);
	if (s.isLoading())
		assert(count == items.size());

	for (uint idx = 0; idx < items.size(); ++idx)
		items[idx]->synchronize(s);
}

void Character::synchronize(Common::Serializer &s) {
	s.syncString(_name);
	s.syncAsByte(_race);
	s.syncAsByte(_sex);
	s.syncAsByte(_class);
	s.syncAsUint16LE(_strength);
	s.syncAsUint16LE(_agility);
	s.syncAsUint16LE(_stamina);
	s.syncAsUint16LE(_charisma);
	s.syncAsUint16LE(_wisdom);
	s.syncAsUint16LE(_intelligence);
	s.syncAsUint32LE(_hitPoints);
	s.syncAsUint32LE(_experience);
	s.syncAsUint32LE(_food);
	s.syncAsUint32LE(_coins);
	s.syncAsSByte(_equippedWeapon);
	s.syncAsSByte(_equippedArmour);
	s.syncAsSByte(_equippedSpell);

	syncArray<Weapon>(_weapons, s);
	syncArray<Armour>(_armour, s);
	syncArray<Spell>(_spells, s);
}

} // End of namespace Shared
} // End of namespace Ultima
