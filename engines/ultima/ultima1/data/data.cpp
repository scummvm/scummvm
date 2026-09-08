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

#include "ultima/ultima1/data/data.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Data {

const char *SEX_NAMES[] = { "Male", "Female" };
const char *RACE_NAMES[] = { nullptr, "Human", "Elf", "Dwarf", "Bobbit" };
const char *CLASS_NAMES[] = { nullptr, "Fighter", "Cleric", "Wizard", "Thief" };

// Index 0 of each ("Skin"/"Hands"/"Prayer"/"Foot") is the innate/default
// state and is never shown in the stats screen, but is included here to
// keep these arrays indexed identically to the enums/savegame arrays above
const char *ARMOR_NAMES[] = {
	"Skin", "Leather armor", "Chain mail", "Plate mail", "Vacuum suit", "Reflect suit"
};
const char *WEAPON_NAMES[] = {
	"Hands", "Dagger", "Mace", "Axe", "Rope & Spikes", "Sword", "Great Sword",
	"Bow & Arrows", "Amulet", "Wand", "Staff", "Triangle", "Pistol",
	"Light Sword", "Phazor", "Blaster"
};
const char *WEAPON_NAMES_LOWER[] = {
	"hands", "dagger", "mace", "axe", "rope", "sword", "g sword", "bow", "amulet",
	"wand", "staff", "triangle", "pistol", "L sword", "phazor", "blaster"
};
const byte WEAPONS_DISTANCE[] = {
	1, 1, 1, 1, 0, 1, 1, 3, 0, 0, 0, 1, 3, 1, 3, 3
};

const char *SPELL_NAMES[] = {
	"Prayer", "Open", "Unlock", "Magic Missile", "Steal", "Ladder Down",
	"Ladder Up", "Blink", "Create", "Destroy", "Kill"
};
const char *SPELL_PHRASES[] = {
	"\"POTENTIS-LAUDIS!\"", "\"APERTUS!\"", "\"PECUNIA!\"", "\"VASTO!\"", "\"NUDO!\"", "\"INFERUS!\"",
	"\"ASCENDO!\"", "\"DUCIS-EDUCO!\"", "\"STRUXI!\"", "\"DELIO!\"", "\"INTERFICIO!\""
};

const char *TRANSPORT_NAMES[] = {
	"Foot", "Horse", "Cart", "Raft", "Frigate", "Aircar", "Shuttle", "Time Machine"
};
const char *GEM_NAMES[] = { "Red Gem", "Green Gem", "Blue Gem", "White Gem" };

// Indexed by TransportType
const int TRANSPORT_RANDOM_THRESHOLD[] = { 100, 50, 33, 100, 50, 25, 10, 10 };

// Indexed by TransportType
const int FOOD_PER_TRANSPORT[] = { 2, 3, 3, 4, 5, 6, 11, 0 };

} // namespace Data
} // namespace Ultima1
} // namespace Ultima
