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

#include "mm/mm1/data/spells.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {

Spells::SpellFn Spells::SPELLS[SPELLS_COUNT] = {
	// Cleric spells
	placeholder,
	placeholder,
	placeholder,
	spell04_firstAid,
	spell05_light
};

void Spells::cast(int spell, Character *chr) {
	assert(spell < SPELLS_COUNT);
	SPELLS[spell](chr);
}

void Spells::spell04_firstAid(Character *chr) {

}

void Spells::spell05_light(Character *chr) {
	g_globals->_spells._s.light++;
	g_events->send("Game", GameMessage("UPDATE"));
}

} // namespace MM1
} // namespace MM
