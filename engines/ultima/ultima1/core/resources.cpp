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

#include "ultima/ultima1/core/resources.h"
#include "common/algorithm.h"

namespace Ultima {
namespace Ultima1 {

const char *const SRC_LOCATION_NAMES[85] = {
	"?",
	"Britian",
	"Moon",
	"Fawn",
	"Paws",
	"Montor",
	"Yew",
	"Tune",
	"Grey",
	"Arnold",
	"Linda",
	"Helen",
	"Owen",
	"John",
	"Gerry",
	"Wolf",
	"The Snake",
	"Nassau",
	"Clear Lagoon",
	"Stout",
	"Gauntlet",
	"Imagination",
	"Ponder",
	"Wealth",
	"Poor",
	"Gorlab",
	"Dextron",
	"Magic",
	"Wheeler",
	"Bulldozer",
	"The Brother",
	"Turtle",
	"Lost Friends",

	"The Castle of Lord British",
	"The Castle of the Lost King",
	"The Castle Barataria",
	"The Castle Rondorin",
	"The Castle of Olympus",
	"The Black Dragon's Castle",
	"The White Dragon's Castle",
	"The Castle of Shamino",

	"The Pillars of Protection",
	"The Tower of Knowledge",
	"The Pillars of the Argonauts",
	"The Pillar of Ozymandias",
	"The Sign Post",
	"The Southern Sign Post",
	"The Eastern Sign Post",

	"The Grave of the Lost Soul",
	"The Unholy Hole",
	"The Dungeon of Perinia",
	"The Dungeon of Montor",
	"The Mines of Mt. Drash",
	"Mondain's Gate to Hell",
	"The Lost Caverns",
	"The Dungeon of Doubt",
	"The Mines of Mt. Drash II",
	"Death's Awakening",
	"The Savage Place",
	"Scorpion Hole",
	"Advari's Hole",
	"The Dead Warrior's Fight",
	"The Horror of the Harpies",
	"The Labyrinth",
	"Where Hercules Died",
	"The Horror of the Harpies II",
	"The Gorgon Hole",
	"The Tramp of Doom",
	"The Viper's Pit",
	"The Long Death",
	"The End...",
	"The Viper's Pit II",
	"The Slow Death",
	"The Guild of Death",
	"The Metal Twister",
	"The Troll's Hole",
	"The Skull Smasher",
	"The Spine Breaker",
	"The Dungeon of Doom",
	"The Dead Cat's Life",
	"The Morbid Adventure",
	"Free Death Hole",
	"Dead Man's Walk",
	"The Dead Cat's Life II",
	"The Hole to Hades"
};

/*-------------------------------------------------------------------*/

GameResources::GameResources() : LocalResourceFile("ULTIMA1/DATA") {
} 

GameResources::GameResources(Shared::Resources *resManager) :
		Shared::LocalResourceFile(resManager, "ULTIMA1/DATA") {
	Common::copy(SRC_LOCATION_NAMES, SRC_LOCATION_NAMES + 85, LOCATION_NAMES);
}

void GameResources::synchronize() {
	syncStrings(LOCATION_NAMES, 32);
}

} // End of namespace Ultima1
} // End of namespace Ultima
