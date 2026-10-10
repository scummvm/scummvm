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

#include "ultima/ultima3/data/roster.h"

namespace Ultima {
namespace Ultima3 {
namespace Data {

void RosterEntry::synchronize(Common::Serializer &s, bool original) {
	s.syncBytes((byte *)_name, sizeof(_name));
	if (original)
		s.skip(4);

	s.syncAsByte(_marksAndCards);
	s.syncAsByte(_torches);
	s.syncAsByte(_partyMember);
	s.syncAsByte(_status);
	s.syncAsByte(_strength);
	s.syncAsByte(_dexterity);
	s.syncAsByte(_intelligence);
	s.syncAsByte(_wisdom);
	s.syncAsByte(_race);
	s.syncAsByte(_class);
	s.syncAsByte(_sex);
	s.syncAsByte(_magicPoints);
	s.syncAsUint16LE(_hitPoints);
	s.syncAsUint16LE(_maxHitPoints);
	s.syncAsUint16LE(_experience);
	s.syncAsByte(_foodSubCounter);
	s.syncAsUint16LE(_food);
	s.syncAsUint16LE(_gold);
	s.syncAsByte(_gems);
	s.syncAsByte(_keys);
	s.syncAsByte(_powder);
	s.syncAsByte(_armourIndex);
	s.syncBytes(_armourOwned, sizeof(_armourOwned));
	s.syncAsByte(_weaponIndex);
	s.syncBytes(_weaponOwned, sizeof(_weaponOwned));
}

} // namespace Data
} // namespace Ultima3
} // namespace Ultima
