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

const char *const SRC_STATUS_TEXT[4] = { "Hits:", "Food:",  "Exp.:", "Coin:" };

const char *const SRC_DIRECTION_NAMES[4] = { "West", "East", "North", "South" };

const char *const SRC_LOCATION_NAMES[LOCATION_COUNT] = {
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

const byte SRC_LOCATION_X[LOCATION_COUNT] = {
	39, 66, 25, 46, 52, 18, 70, 64, 126, 128, 148, 115, 150, 121,
	150, 109, 42, 44, 64, 31, 66, 37, 66, 25, 128, 101, 142, 121,
	115, 149, 97, 103, 40, 32, 125, 114, 41, 30, 127, 135, 36, 69,
	96, 97, 13, 12, 131, 98, 48, 18, 53, 59, 29, 13, 62, 39,
	38, 130, 100, 124, 155, 147, 98, 109, 116, 136, 52, 32, 25, 14,
	63, 71, 40, 16, 46, 119, 149, 114, 108, 138, 154, 105, 128, 129
};
const byte SRC_LOCATION_Y[LOCATION_COUNT] = {
	39, 41, 61, 28, 63, 34, 63, 22, 36, 63, 22, 43, 49, 15,
	67, 61, 119, 92, 133, 112, 106, 140, 88, 94, 117, 119, 139, 106,
	141, 112, 141, 100, 38, 27, 37, 29, 118, 126, 116, 105, 9, 10,
	33, 66, 89, 122, 87, 88, 11, 13, 22, 29, 37, 43, 49, 60,
	68, 10, 15, 26, 35, 36, 45, 50, 56, 59, 96, 99, 105, 110,
	119, 120, 129, 140, 145, 89, 91, 100, 107, 115, 121, 127, 138, 146
};

const LocationPerson SRC_LOCATION_PEOPLE[150] = {
	{ 20, 33, 4, 2000 },
	{ 22, 35, 12, 1 },
	{ 17, 1, 6, 500 },
	{ 17, 17, 6, 500 },
	{ 17, 17, 11, 500 },
	{ 17, 25, 7, 500 },
	{ 17, 25, 12, 500 },
	{ 17, 36, 9, 500 },
	{ 19, 35, 6, 1 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ 20, 25, 4, 2000 },
	{ 22, 31, 2, 1 },
	{ 17, 28, 12, 500 },
	{ 17, 34, 12, 500 },
	{ 17, 0, 8, 500 },
	{ 17, 11, 3, 500 },
	{ 17, 22, 1, 500 },
	{ 17, 28, 1, 500 },
	{ 17, 33, 5, 500 },
	{ 19, 22, 14, 1 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ 19, 15, 5, 1 },
	{ 50, 34, 6, 1 },
	{ 17, 2, 9, 500 },
	{ 17, 16, 16, 500 },
	{ 17, 18, 2, 500 },
	{ 17, 20, 8, 500 },
	{ 17, 21, 16, 500 },
	{ 17, 36, 8, 500 },
	{ 21, 6, 3, 1 },
	{ 21, 6, 14, 1 },
	{ 21, 13, 14, 1 },
	{ 21, 25, 3, 1 },
	{ 21, 25, 12, 1 },
	{ 21, 32, 3, 1 },
	{ -1, 0, 0, 0 },
	{ 19, 15, 9, 1 },
	{ 50, 27, 3, 1 },
	{ 17, 1, 10, 500 },
	{ 17, 9, 8, 500 },
	{ 17, 16, 1, 500 },
	{ 17, 21, 16, 500 },
	{ 17, 36, 7, 500 },
	{ 21, 4, 16, 1 },
	{ 21, 5, 3, 1 },
	{ 21, 12, 3, 1 },
	{ 21, 30, 1, 1 },
	{ 21, 34, 12, 1 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ 19, 17, 3, 1 },
	{ 50, 21, 5, 1 },
	{ 17, 10, 10, 500 },
	{ 17, 15, 15, 500 },
	{ 17, 22, 15, 500 },
	{ 17, 23, 9, 500 },
	{ 21, 3, 12, 1 },
	{ 21, 6, 3, 1 },
	{ 21, 19, 1, 1 },
	{ 21, 34, 1, 1 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ 19, 19, 12, 1 },
	{ 50, 24, 3, 1 },
	{ 17, 1, 7, 500 },
	{ 17, 1, 11, 500 },
	{ 17, 8, 16, 500 },
	{ 17, 14, 7, 500 },
	{ 17, 29, 16, 500 },
	{ 17, 36, 11, 500 },
	{ 21, 4, 1, 1 },
	{ 21, 12, 1, 1 },
	{ 21, 17, 1, 1 },
	{ 21, 22, 1, 1 },
	{ 21, 28, 1, 1 },
	{ 21, 34, 1, 1 },
	{ -1, 0, 0, 0 },
	{ 17, 17, 2, 500 },
	{ 17, 17, 15, 500 },
	{ 17, 20, 2, 500 },
	{ 17, 20, 15, 500 },
	{ 21, 12, 8, 1 },
	{ 21, 25, 8, 1 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ 50, 20, 4, 1 },
	{ 17, 1, 1, 500 },
	{ 17, 1, 16, 500 },
	{ 17, 8, 9, 500 },
	{ 17, 16, 15, 500 },
	{ 17, 18, 10, 500 },
	{ 17, 21, 15, 500 },
	{ 17, 29, 9, 500 },
	{ 17, 36, 1, 500 },
	{ 17, 36, 16, 500 },
	{ 21, 8, 4, 1 },
	{ 21, 9, 13, 1 },
	{ 21, 18, 2, 1 },
	{ 21, 28, 13, 1 },
	{ 21, 29, 4, 1 },
	{ 19, 31, 14, 1 },
	{ 50, 27, 6, 1 },
	{ 17, 4, 11, 500 },
	{ 17, 17, 14, 500 },
	{ 17, 20, 14, 500 },
	{ 21, 7, 4, 1 },
	{ 21, 18, 4, 1 },
	{ 21, 29, 3, 1 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ 19, 19, 10, 5 },
	{ 17, 6, 12, 500 },
	{ 17, 6, 15, 500 },
	{ 17, 7, 7, 500 },
	{ 17, 9, 5, 500 },
	{ 17, 28, 17, 500 },
	{ 17, 37, 8, 500 },
	{ 21, 1, 13, 1 },
	{ 21, 4, 1, 1 },
	{ 21, 16, 1, 1 },
	{ 21, 24, 1, 1 },
	{ 21, 33, 16, 1 },
	{ 21, 34, 1, 1 },
	{ -1, 0, 0, 0 },
	{ -1, 0, 0, 0 }
};

const char *const SRC_BLOCKED = "Blocked!";

const char *const SRC_ENTER_QUESTION = "Enter?";

const char *const SRC_ENTERING = "Entering...";

const char *const SRC_THE_CITY_OF = "The city of ";

/*-------------------------------------------------------------------*/

GameResources::GameResources() : LocalResourceFile("ULTIMA1/DATA") {
} 

GameResources::GameResources(Shared::Resources *resManager) : LocalResourceFile(resManager, "ULTIMA1/DATA") {
	Common::copy(SRC_STATUS_TEXT, SRC_STATUS_TEXT + 4, STATUS_TEXT);
	Common::copy(SRC_DIRECTION_NAMES, SRC_DIRECTION_NAMES + 4, DIRECTION_NAMES);
	Common::copy(SRC_LOCATION_NAMES, SRC_LOCATION_NAMES + 85, LOCATION_NAMES);
	Common::copy(SRC_LOCATION_X, SRC_LOCATION_X + 84, LOCATION_X);
	Common::copy(SRC_LOCATION_Y, SRC_LOCATION_Y + 84, LOCATION_Y);
	Common::copy(SRC_LOCATION_PEOPLE, SRC_LOCATION_PEOPLE + 150, LOCATION_PEOPLE);
	BLOCKED = SRC_BLOCKED;
	ENTER_QUESTION = SRC_ENTER_QUESTION;
	ENTERING = SRC_ENTERING;
	THE_CITY_OF = SRC_THE_CITY_OF;
}

void GameResources::synchronize() {
	syncStrings(STATUS_TEXT, 4);
	syncStrings(DIRECTION_NAMES, 4);
	syncStrings(LOCATION_NAMES, LOCATION_COUNT);
	syncBytes(LOCATION_X, 84);
	syncBytes(LOCATION_Y, 84);
	syncString(BLOCKED);
	syncString(ENTER_QUESTION);
	syncString(ENTERING);
	syncString(THE_CITY_OF);
}

} // End of namespace Ultima1
} // End of namespace Ultima
