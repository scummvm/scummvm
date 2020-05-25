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

static const char *const SRC_TITLE_MESSAGES[13] = {
	"Ultima I",
	"The First Age of Darkness",
	"Copyright (C) 1987 Origin Systems, Inc.",
	"Presents...",
	"...a new release of the best",
	"   selling personal computer",
	"   role-playing adventure...",
	"...Lord British's original",
	"   fantasy masterpiece...",
	"ULTIMA and LORD BRITISH are registered",
	"trademarks of Richard Garriott and",
	"Origin Systems, Inc.",
	"Conversion by: John Fachini"
};

static const char *const SRC_MAIN_MENU_TEXT[7] = {
	"*** Ultima I ***",  "from darkest", "dungeons, to", "deepest space!",
	"a) Generate new character", "b) Continue previous game", "Thy choice: "
};

static const char *const SRC_CHAR_GEN_TEXT[14] = {
	" Character Generation ",
	"Points left to distribute: %2d",
	"Strength........%d\n"
		"Agility.........%d\n"
		"Stamina.........%d\n"
		"Charisma........%d\n"
		"Wisdom..........%d\n"
		"Intelligence....%d",
	"Move cursor with up and down arrows;\n"
		"increase and decrease attributes\n"
		"with left and right arrows.  Press\n"
		"space bar when finished, or escape\n"
		"to return to the main menu.",
	"a) %s\n"
		"b) %s\n"
		"c) %s\n"
		"d) %s",
	"a) %s\n"
		"b) %s",
	"Select thy race:",
	"Select thy sex:",
	"Select thy class:",
	"Race: ",
	"Sex: ",
	"Class: ",
	"Enter thy name:",
	"Save this character? (Y-N)"
};

static const char *const SRC_RACE_NAMES[4] = { "Human", "Elf", "Dwarf", "Bobbit" };

static const char *const SRC_SEX_NAMES[3] = { "Male", "Female", "Yes Please" };

static const char *const SRC_CLASS_NAMES[4] = { "Fighter", "Cleric", "Wizard", "Thief" };

static const char *const SRC_TRANSPORT_NAMES[8] = { "Foot", "Horse", "Cart", "Raft", "Frigate", "Aircar", "Shuttle", "Time Machine" };

static const char *const SRC_STAT_NAMES[11] = {
	"Hit Points", "Strength", "Agility", "Stamina", "Charisma", "Wisdom", "Intelligence",
	"Copper pence", "Silver pieces", "Gold crowns", "Enemy vessels"
};

static const char *const SRC_STATUS_TEXT[4] = { "Hits:", "Food:",  "Exp.:", "Coin:" };

static const char *const SRC_DIRECTION_NAMES[4] = { "West", "East", "North", "South" };

static const char *const SRC_DUNGEON_MOVES[4] = { "Turn left", "Turn right", "Forward", "Turn around" };

static const char *const SRC_LOCATION_NAMES[LOCATION_COUNT] = {
	"Britain",
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

static const char *const SRC_DUNGEON_ITEM_NAMES[2] = { "Chest", "Coffin" };

static const char *SRC_WEAPON_NAMES_UPPERCASE[16] = {
	"Hands", "Dagger", "Mace", "Axe", "Rope & Spikes", "Sword", "Great Sword", "Bow & Arrows",
	"Amulet", "Wand", "Staff", "Triangle", "Pistol", "Light Sword", "Phazor", "Blaster"
};

static const char *SRC_WEAPON_NAMES_LOWERCASE[16] = {
	"hands", "dagger", "mace", "axe", "rope", "sword", "g sword", "bow",
	"amulet", "wand", "staff", "triangle", "pistol", "L sword", "phazor", "blaster"
};

static const char *SRC_WEAPON_NAMES_ARTICLE[16] = {
	"a Hands", "a Dagger", "a Mace", "Axe", "a Rope & Spikes", "a Sword", "a Great Sword", "a Bow & Arrows",
	"an Amulet", "a Wand", "a Staff", "a Triangle", "a Pistol", "a Light Sword", "a Phazor", "a Blaster"
};

static const byte SRC_WEAPON_DISTANCES[16] = { 1, 1, 1, 1, 0, 1, 1, 3, 0, 0, 0, 1, 3, 1, 3, 3 };

static const char *SRC_ARMOUR_NAMES[6] = {
	"Skin", "Leather armor", "Chain mail", "Plate mail", "Vacuum suit", "Reflect suit"
};

static const char *SRC_SPELL_NAMES[11] = {
	"Prayer", "Open", "Unlock", "Magic Missile", "Steal", "Ladder Down", "Ladder Up",
	"Blink", "Create", "Destroy", "Kill"
};

static const char *SRC_SPELL_PHRASES[14] = {
	"\"POTENTIS-LAUDIS!\"", "\"APERTUS!\"", "\"PECUNIA!\"", "\"VASTO!\"", "\"NUDO!\"",
	"\"INFERUS!\"", "\"ASCENDO!\"", "\"DUCIS-EDUCO!\"", "\"STRUXI!\"", "\"DELIO!\"",
	"\"INTERFICIO!\"", " Shazam!", "\"DELCIO-ERE-UI\" ", "\"INTERFICIO-NUNC!\" "
};

static const char *SRC_GEM_NAMES[4] = { "Red Gem", "Green Gem", "Blue Gem", "White Gem" };

static const byte SRC_LOCATION_X[LOCATION_COUNT] = {
	39, 66, 25, 46, 52, 18, 70, 64, 126, 128, 148, 115, 150, 121,
	150, 109, 42, 44, 64, 31, 66, 37, 66, 25, 128, 101, 142, 121,
	115, 149, 97, 103, 40, 32, 125, 114, 41, 30, 127, 135, 36, 69,
	96, 97, 13, 12, 131, 98, 48, 18, 53, 59, 29, 13, 62, 39,
	38, 130, 100, 124, 155, 147, 98, 109, 116, 136, 52, 32, 25, 14,
	63, 71, 40, 16, 46, 119, 149, 114, 108, 138, 154, 105, 128, 129
};
static const byte SRC_LOCATION_Y[LOCATION_COUNT] = {
	39, 41, 61, 28, 63, 34, 63, 22, 36, 63, 22, 43, 49, 15,
	67, 61, 119, 92, 133, 112, 106, 140, 88, 94, 117, 119, 139, 106,
	141, 112, 141, 100, 38, 27, 37, 29, 118, 126, 116, 105, 9, 10,
	33, 66, 89, 122, 87, 88, 11, 13, 22, 29, 37, 43, 49, 60,
	68, 10, 15, 26, 35, 36, 45, 50, 56, 59, 96, 99, 105, 110,
	119, 120, 129, 140, 145, 89, 91, 100, 107, 115, 121, 127, 138, 146
};

const int SRC_LOCATION_PEOPLE[150][4] = {
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

#define OFFSET(x) (x & 0xff), (x >> 8)

static const byte SRC_DUNGEON_DRAW_DATA[1964] = {
	OFFSET(60),			// Ranger
	OFFSET(168),		// Skeleton
	OFFSET(271),		// Thief
	OFFSET(319),		// Giant Rat
	OFFSET(374),		// Rat
	OFFSET(424),		// Spider
	OFFSET(531),		// Viper
	OFFSET(619),		// Orc
	OFFSET(706),		// Cyclops
	OFFSET(782),		// Gelatinous Cube
	OFFSET(794),		// Ettin
	OFFSET(871),		// Mimic
	OFFSET(899),		// Lizard Man
	OFFSET(993),		// Minotaur
	OFFSET(1081),		// Carrion Creeper
	OFFSET(1123),		// Tangler
	OFFSET(1209),		// Gremlin
	OFFSET(1254),		// Wandering Eyes
	OFFSET(1397),		// Wraith
	OFFSET(1471),		// Lich
	OFFSET(1552),		// Invisible Seeker
	OFFSET(1553),		// Wind Whipper
	OFFSET(1623),		// Zorn
	OFFSET(1706),		// Daemon
	OFFSET(1791),		// Balron
	OFFSET(1869),		// Coffin
	OFFSET(1903),		// Unknown 1
	OFFSET(1915),		// Unknown 2
	OFFSET(1927),		// Unknown 3
	OFFSET(1958),		// Unknown 4
	0x7E, 0xE1, 0xFC, 0x1E, 0xFC, 0x0C, 0xE0, 0x0C, 0xDB, 0x12, 0xD4, 0x2A, 0xDB, 0x2A, 0xCF, 0x12,
	0xCB, 0x06, 0xC3, 0x0C, 0xBB, 0x0C, 0xB6, 0x06, 0xB3, 0xF9, 0xB3, 0xF3, 0xB6, 0xF3, 0xBB, 0xF9,
	0xC3, 0xED, 0xCB, 0xD5, 0xCF, 0xD5, 0xDB, 0xED, 0xD4, 0xF3, 0xDB, 0xF3, 0xE0, 0xE1, 0xFC, 0x7E,
	0xED, 0xFC, 0xE7, 0x00, 0xF3, 0x00, 0xF3, 0xFC, 0x7E, 0x12, 0xFC, 0x18, 0x00, 0x0C, 0x00, 0x0C,
	0xFC, 0x7E, 0xF3, 0xDB, 0x0C, 0xDB, 0x7E, 0xF3, 0xE0, 0x0C, 0xE0, 0x7E, 0xF3, 0xC7, 0xF9, 0xCB,
	0x06, 0xCB, 0x0C, 0xC7, 0x7E, 0x00, 0xD4, 0x00, 0xCB, 0x7E, 0xFB, 0xBF, 0x04, 0xBF, 0x7E, 0x00,
	0xBB, 0x06, 0xB6, 0x06, 0xBB, 0xF9, 0xBB, 0xF9, 0xB6, 0x00, 0xBB, 0x7F, 0x7E, 0xDB, 0x00, 0xE7,
	0x00, 0xE7, 0xEC, 0xF3, 0xE2, 0x0C, 0xE2, 0x18, 0xEC, 0x18, 0x00, 0x24, 0x00, 0x7E, 0x00, 0xE7,
	0x00, 0xB6, 0x7E, 0xFC, 0xD8, 0x03, 0xD8, 0x7E, 0xF8, 0xCF, 0x07, 0xCF, 0x7E, 0xF5, 0xC5, 0x0A,
	0xC5, 0x7E, 0xE7, 0xCF, 0xF3, 0xBB, 0x0C, 0xBB, 0x18, 0xCF, 0x7E, 0xDB, 0xC5, 0xDB, 0xCF, 0xCF,
	0xC5, 0xDB, 0xC5, 0xF3, 0xD8, 0x7E, 0x18, 0xD4, 0x18, 0xC0, 0x7E, 0x12, 0xCF, 0x1E, 0xCF, 0x7E,
	0xF9, 0xB6, 0x06, 0xB6, 0x0C, 0xAD, 0x0C, 0xA8, 0x06, 0xA3, 0xF9, 0xA3, 0xF3, 0xA8, 0xF3, 0xAD,
	0xF9, 0xB6, 0x7E, 0xF9, 0xA3, 0x00, 0xA8, 0x7E, 0xF9, 0xAD, 0x7E, 0x06, 0xAD, 0x7E, 0xF9, 0xB1,
	0x06, 0xB1, 0x7F, 0x7E, 0x00, 0xBC, 0x00, 0xF7, 0x0C, 0x00, 0x24, 0x00, 0x24, 0xC0, 0x0C, 0xB3,
	0x00, 0xBC, 0xF3, 0xB3, 0xDB, 0xC0, 0xDB, 0x00, 0xF3, 0x00, 0x00, 0xF7, 0x7E, 0x0C, 0xB3, 0x0C,
	0xA4, 0x00, 0x9B, 0xF3, 0xA4, 0xF3, 0xB3, 0x7E, 0xF3, 0xA4, 0x00, 0xB6, 0x0C, 0xA4, 0x00, 0xA0,
	0xF3, 0xA4, 0x7F, 0x7E, 0xF9, 0xCF, 0xE7, 0xF3, 0xF3, 0x00, 0x0C, 0x00, 0x18, 0xF3, 0x06, 0xCF,
	0x7E, 0x00, 0xDB, 0xF3, 0xC3, 0x00, 0xC9, 0x0C, 0xC3, 0x00, 0xDB, 0x7E, 0x06, 0xC9, 0x02, 0xCF,
	0x7E, 0xF9, 0xC9, 0xFD, 0xCF, 0x7E, 0xFB, 0xE7, 0xFB, 0xE1, 0x7E, 0xF6, 0xE7, 0xF6, 0xE1, 0x7E,
	0x09, 0xE7, 0x09, 0xE1, 0x7E, 0x04, 0xE7, 0x04, 0xE1, 0x7F, 0x7E, 0x03, 0xC7, 0x15, 0xBE, 0x2B,
	0xBE, 0x36, 0xC3, 0x2B, 0xB4, 0x15, 0xAF, 0x03, 0xB6, 0x07, 0xB1, 0x07, 0xAA, 0x03, 0xAD, 0xFD,
	0xAD, 0xF8, 0xAA, 0xF8, 0xB1, 0xFC, 0xB6, 0xEA, 0xAF, 0xD4, 0xB4, 0xC9, 0xC3, 0xD4, 0xBE, 0xEA,
	0xBE, 0xFC, 0xC7, 0x03, 0xC7, 0x7E, 0xFD, 0xB0, 0x7E, 0x02, 0xB0, 0x7F, 0x7E, 0xE7, 0xDB, 0x18,
	0xE7, 0x30, 0xDB, 0x18, 0xC3, 0x00, 0xB6, 0xCF, 0xAA, 0xB6, 0xB6, 0xCF, 0xCF, 0xE7, 0xDB, 0xE7,
	0xE1, 0xC3, 0xE7, 0xDB, 0xE1, 0xDB, 0xD5, 0xC3, 0xD5, 0xB6, 0xDB, 0xC3, 0xCF, 0x00, 0xCF, 0x00,
	0xDB, 0xE7, 0xDB, 0xE7, 0xC3, 0xCF, 0xC3, 0xCF, 0xCF, 0x7E, 0xE7, 0xC3, 0x00, 0xCF, 0x7E, 0x18,
	0xE7, 0x3C, 0xF3, 0x00, 0x01, 0x7E, 0x20, 0xE3, 0x4F, 0xE7, 0x79, 0x00, 0x7E, 0x29, 0xDF, 0x4F,
	0xDB, 0x79, 0xED, 0x7E, 0x30, 0xDB, 0x4F, 0xC9, 0x79, 0xDB, 0x7E, 0xB6, 0xB6, 0x92, 0xC9, 0x86,
	0xE1, 0x7E, 0xBE, 0xB3, 0xAA, 0x9E, 0x86, 0xB6, 0x7E, 0xC6, 0xAE, 0xC3, 0x98, 0x9E, 0x86, 0x7E,
	0xCF, 0xAA, 0xE7, 0x98, 0xB6, 0x86, 0x7F, 0x7E, 0xE7, 0xED, 0xE7, 0xE7, 0xDB, 0xDB, 0xDB, 0x00,
	0x24, 0x00, 0x24, 0xED, 0xDB, 0xED, 0x7E, 0x18, 0xED, 0x0C, 0xE7, 0x0C, 0xD5, 0xF3, 0xD5, 0xF3,
	0xE7, 0x00, 0xED, 0x7E, 0xE7, 0xF9, 0x24, 0xF9, 0x24, 0xF3, 0xDB, 0xF3, 0x7E, 0xF3, 0xE7, 0x0C,
	0xE7, 0x0C, 0xE1, 0xF3, 0xE1, 0xF3, 0xDB, 0x0C, 0xDB, 0x0C, 0xD5, 0xF3, 0xD5, 0x7E, 0x0C, 0xCC,
	0xF3, 0xCC, 0xE7, 0xDB, 0x00, 0xCC, 0x18, 0xDB, 0x0C, 0xCC, 0x7E, 0xED, 0xD5, 0xED, 0xCC, 0xF3,
	0xC3, 0x0C, 0xC3, 0x11, 0xCC, 0x12, 0xD5, 0x7E, 0xED, 0xCC, 0x00, 0xC3, 0x0F, 0xCC, 0x7F, 0x7E,
	0xE7, 0x00, 0x18, 0x00, 0x0C, 0xF7, 0x0C, 0xED, 0x18, 0xE5, 0x18, 0xED, 0x24, 0xE5, 0x24, 0xC9,
	0x18, 0xC0, 0xE7, 0xC0, 0x7E, 0xE7, 0x00, 0xF3, 0xF7, 0xF3, 0xED, 0xE7, 0xE5, 0xE7, 0xED, 0xDB,
	0xE5, 0xDB, 0xC9, 0xE7, 0xC0, 0x7E, 0x0C, 0xC0, 0x18, 0xAE, 0x0C, 0xA4, 0xF3, 0xA4, 0xE7, 0xAE,
	0xF3, 0xC0, 0x7E, 0x18, 0xAE, 0xE7, 0xAE, 0x7E, 0xDB, 0xED, 0x0C, 0xC9, 0x18, 0xC9, 0x00, 0xDB,
	0x00, 0xD2, 0xF3, 0xD2, 0x0C, 0xC0, 0x0C, 0xC9, 0x7E, 0x00, 0xB6, 0x0C, 0xAE, 0x0C, 0xB6, 0xF3,
	0xB6, 0xF3, 0xAE, 0x00, 0xB6, 0x7F, 0x7E, 0x00, 0xE1, 0x06, 0x00, 0x1E, 0x00, 0x12, 0xF9, 0x08,
	0xE1, 0x12, 0xC9, 0x1E, 0xE1, 0x24, 0xE1, 0x1E, 0xC3, 0x06, 0xBC, 0x12, 0xB6, 0x12, 0xB0, 0x06,
	0xAA, 0x7E, 0x00, 0xE1, 0xF9, 0x00, 0xE1, 0x00, 0xED, 0xF9, 0xF7, 0xE1, 0xED, 0xC9, 0xE1, 0xE1,
	0xDB, 0xE1, 0xE1, 0xC3, 0xF9, 0xBC, 0xED, 0xB6, 0xED, 0xB0, 0xF9, 0xAA, 0x7E, 0x06, 0xAA, 0xF9,
	0xAA, 0x7E, 0x0C, 0xB0, 0x00, 0xAD, 0xF3, 0xB0, 0x00, 0xB4, 0x0C, 0xB0, 0x7E, 0xF3, 0xB6, 0x0C,
	0xB6, 0x7F, 0x7E, 0xB5, 0xF1, 0x4B, 0xF1, 0x4B, 0xAB, 0xB5, 0xAB, 0xB5, 0xF1, 0x7F, 0x7E, 0x00,
	0xE1, 0x18, 0x00, 0x30, 0x00, 0x24, 0xF9, 0x12, 0xD5, 0x18, 0xC9, 0x24, 0xCF, 0x24, 0xDB, 0x30,
	0xDB, 0x30, 0xC9, 0x12, 0xB6, 0x24, 0xB6, 0x24, 0xA5, 0x0C, 0xA5, 0x0C, 0xB3, 0xF3, 0xB3, 0x7E,
	0x00, 0xE1, 0xE7, 0x00, 0xCF, 0x00, 0xDB, 0xF9, 0xED, 0xD5, 0xE7, 0xC6, 0xCF, 0xCC, 0xBA, 0xBE,
	0xC3, 0xB6, 0xCF, 0xBF, 0xED, 0xB6, 0xDB, 0xB6, 0xDB, 0xA5, 0xF3, 0xA5, 0xF3, 0xB3, 0x7E, 0x1C,
	0xB1, 0x7E, 0x15, 0xAB, 0x7E, 0xE3, 0xB1, 0x7E, 0xEA, 0xAB, 0x7F, 0x7E, 0xE7, 0x05, 0xE7, 0xE7,
	0x18, 0xE7, 0x18, 0x05, 0xE7, 0x05, 0x7E, 0xE7, 0xE7, 0xF3, 0xDB, 0x24, 0xDB, 0x24, 0xF3, 0x18,
	0x05, 0x7E, 0x18, 0xE7, 0x24, 0xDB, 0x7F, 0x7E, 0x06, 0xC9, 0x0E, 0xD5, 0x06, 0xE7, 0x06, 0xED,
	0x14, 0x00, 0x1E, 0x00, 0x18, 0xF9, 0x12, 0xE7, 0x12, 0xDB, 0x7E, 0x06, 0xC9, 0xF9, 0xC9, 0xF1,
	0xD5, 0xF9, 0xE7, 0xF9, 0xED, 0xEB, 0x00, 0xE1, 0x00, 0xE7, 0xF9, 0xED, 0xE7, 0xED, 0xDB, 0x7E,
	0x12, 0xDB, 0x1E, 0xC9, 0x1E, 0xB6, 0x18, 0xBC, 0x18, 0xC9, 0x12, 0xC9, 0x06, 0xC3, 0x12, 0xBC,
	0x06, 0xB6, 0xF9, 0xB6, 0xE7, 0xBC, 0xE7, 0xBF, 0xF9, 0xC3, 0xE7, 0xCF, 0xE7, 0xE7, 0xED, 0xE1,
	0x7E, 0xE7, 0xBE, 0xF9, 0xBE, 0x7E, 0x02, 0xBA, 0x7E, 0xF9, 0xED, 0x0F, 0xF9, 0x7E, 0x18, 0xF9,
	0x2A, 0xF3, 0x1E, 0x00, 0x7F, 0x7E, 0x00, 0xE1, 0x12, 0xE7, 0x12, 0xF9, 0x1E, 0x00, 0x2A, 0x00,
	0x1E, 0xF9, 0x1E, 0xE1, 0x12, 0xD5, 0x12, 0xCF, 0x2A, 0xBC, 0x2A, 0xAA, 0x1E, 0xB6, 0x12, 0xBC,
	0x06, 0xB6, 0x7E, 0x00, 0xE1, 0xED, 0xE7, 0xED, 0xF9, 0xE1, 0x00, 0xD5, 0x00, 0xE1, 0xF9, 0xE1,
	0xE1, 0xED, 0xD5, 0xED, 0xCF, 0xD5, 0xBC, 0xD5, 0xAA, 0xE1, 0xB6, 0xED, 0xBC, 0xF9, 0xB6, 0x7E,
	0x0C, 0xAE, 0x06, 0xB6, 0x06, 0xBA, 0xF9, 0xBA, 0xF9, 0xB6, 0xF3, 0xAE, 0xED, 0xA4, 0xF9, 0xAA,
	0x06, 0xAA, 0x12, 0xA4, 0x0C, 0xAE, 0x7E, 0xFB, 0xAF, 0x7E, 0x04, 0xAF, 0x7F, 0x7E, 0xE7, 0x86,
	0xE7, 0x92, 0xF3, 0x9E, 0x0C, 0x9E, 0x18, 0x92, 0x18, 0x86, 0x7E, 0xE7, 0x92, 0xD5, 0xA4, 0xD5,
	0xC3, 0x7E, 0xF3, 0x9E, 0xE7, 0xB6, 0xF3, 0xCF, 0x7E, 0x0C, 0x9E, 0x18, 0xB6, 0x0C, 0xDB, 0x7E,
	0x18, 0x92, 0x2A, 0xAA, 0x30, 0xC3, 0x7F, 0x7E, 0xE7, 0x00, 0xF3, 0xED, 0xF3, 0xB6, 0x00, 0xAA,
	0x06, 0xAA, 0x0C, 0xB0, 0x0C, 0xE7, 0x18, 0x00, 0x7E, 0xF3, 0xDB, 0xE7, 0xED, 0xDB, 0xE7, 0xE1,
	0xE1, 0x7E, 0xF3, 0xC9, 0xE1, 0xD5, 0xDB, 0xC9, 0xE1, 0xBC, 0x7E, 0xF3, 0xBC, 0xE7, 0xB6, 0xE1,
	0xAA, 0xED, 0x9E, 0x7E, 0x0C, 0xB6, 0x18, 0xB0, 0x12, 0xA4, 0x06, 0x9E, 0x7E, 0x0C, 0xC3, 0x18,
	0xC9, 0x24, 0xC3, 0x24, 0xB0, 0x7E, 0x0C, 0xD5, 0x18, 0xE1, 0x1E, 0xE1, 0x24, 0xD5, 0x7E, 0x00,
	0xB6, 0xF9, 0xBC, 0x00, 0xC3, 0x06, 0xBC, 0x00, 0xB6, 0x7E, 0x00, 0xBC, 0x7F, 0x7E, 0xE7, 0x00,
	0xED, 0x00, 0xF3, 0xF9, 0xED, 0xF3, 0xF9, 0xE7, 0x18, 0xE1, 0x0C, 0xED, 0x12, 0xF3, 0x0C, 0xF9,
	0x12, 0x00, 0x18, 0x00, 0x7E, 0xF3, 0xED, 0x0C, 0xED, 0x7E, 0xF3, 0xF9, 0x0C, 0xF9, 0x7E, 0xF9,
	0xF1, 0x7E, 0x06, 0xF1, 0x7E, 0xF9, 0xF6, 0x06, 0xF6, 0x7F, 0x7E, 0x06, 0xDB, 0x0E, 0xD2, 0x0E,
	0xC6, 0x06, 0xBC, 0xF9, 0xBC, 0x7E, 0x06, 0xDB, 0xF9, 0xDB, 0xF1, 0xD2, 0xF1, 0xC6, 0xF9, 0xBC,
	0x7E, 0x00, 0xC3, 0x06, 0xCC, 0x00, 0xD5, 0xF9, 0xCC, 0x00, 0xC3, 0x7E, 0x00, 0xC9, 0x00, 0xCF,
	0x7E, 0x0E, 0xD2, 0x14, 0xD5, 0x18, 0xD2, 0x18, 0xCF, 0x1E, 0xCF, 0x1E, 0xD5, 0x18, 0xD5, 0x18,
	0xCF, 0x7E, 0xF1, 0xD2, 0xEB, 0xCF, 0xE7, 0xD2, 0xE7, 0xCF, 0xE1, 0xCF, 0xE1, 0xD5, 0xE7, 0xD5,
	0xE7, 0xCF, 0x7E, 0xF1, 0xC6, 0xEB, 0xC6, 0xE7, 0xC0, 0xE7, 0xC0, 0xE7, 0xC3, 0xE1, 0xC3, 0xE1,
	0xBC, 0xE7, 0xBC, 0xE7, 0xC3, 0x7E, 0x0E, 0xC6, 0x12, 0xC0, 0x18, 0xC0, 0x18, 0xC3, 0x1E, 0xC3,
	0x1E, 0xBC, 0x18, 0xBC, 0x18, 0xC3, 0x7E, 0x06, 0xBC, 0x08, 0xB4, 0x0C, 0xB4, 0x0C, 0xB6, 0x12,
	0xB6, 0x12, 0xB0, 0x0C, 0xB0, 0x0C, 0xB6, 0x7E, 0xF9, 0xBC, 0xF7, 0xB6, 0xF3, 0xB4, 0xF3, 0xB6,
	0xED, 0xB6, 0xED, 0xB0, 0xF3, 0xB0, 0xF3, 0xB6, 0x7F, 0x7E, 0xCF, 0x00, 0xDB, 0xDB, 0xE7, 0xAA,
	0x00, 0x9E, 0x0C, 0x9E, 0x18, 0xAA, 0x12, 0xB6, 0x12, 0xC3, 0x18, 0xDB, 0x18, 0xDB, 0x24, 0x00,
	0x7E, 0x00, 0xAA, 0x0C, 0xA4, 0x12, 0xAA, 0x0C, 0xB6, 0x06, 0xB6, 0x00, 0xB0, 0x00, 0xAA, 0x7E,
	0xE1, 0xC3, 0xF3, 0xD5, 0x7E, 0xF3, 0xB6, 0xF3, 0xC3, 0xF9, 0xCF, 0x7E, 0x06, 0xCF, 0x12, 0xDB,
	0x7E, 0x0C, 0xC3, 0x12, 0xCF, 0x7E, 0x00, 0xDB, 0x00, 0xF3, 0xF9, 0x00, 0x7E, 0x06, 0xAB, 0x7E,
	0x0C, 0xAB, 0x7F, 0x7E, 0x0C, 0xA4, 0x12, 0xA9, 0x18, 0xB3, 0x18, 0xBC, 0x12, 0xC0, 0x06, 0xD7,
	0xF9, 0xD7, 0x7E, 0x0C, 0xA4, 0xF3, 0xA4, 0xED, 0xA9, 0xE7, 0xB3, 0xE7, 0xBC, 0xED, 0xC0, 0xF9,
	0xD7, 0x7E, 0xED, 0xB3, 0xF3, 0xAE, 0xF9, 0xAE, 0x00, 0xB3, 0x7E, 0x12, 0xB3, 0x0C, 0xAE, 0x06,
	0xAE, 0x00, 0xB3, 0x7E, 0x12, 0xB3, 0x0C, 0xB6, 0x06, 0xB6, 0x00, 0xB3, 0x7E, 0xED, 0xB3, 0xF3,
	0xB6, 0xF9, 0xB6, 0x00, 0xB3, 0x7E, 0xF9, 0xC9, 0x06, 0xC9, 0x7E, 0xF9, 0xBC, 0x06, 0xBC, 0x00,
	0xC0, 0xF9, 0xBC, 0x7F, 0x7F, 0x7E, 0x09, 0xAD, 0x0E, 0xAD, 0x09, 0xBB, 0x18, 0xC0, 0x27, 0x00,
	0xD8, 0x00, 0xE7, 0xC0, 0xF6, 0xBB, 0xF1, 0xAD, 0xF6, 0xAD, 0x7E, 0xEC, 0xE2, 0xF1, 0xC5, 0xFB,
	0xBB, 0xF6, 0xA8, 0x00, 0xA3, 0x09, 0xA8, 0x04, 0xBB, 0x0E, 0xC5, 0x13, 0xE2, 0x7E, 0xFD, 0xB9,
	0xFD, 0xBB, 0xF6, 0xCF, 0xF6, 0xE7, 0x7E, 0x02, 0xB9, 0x02, 0xBB, 0x09, 0xCF, 0x09, 0xE7, 0x7E,
	0x00, 0xB6, 0x00, 0xEC, 0x7E, 0x03, 0xAA, 0x7E, 0xFC, 0xAA, 0x7F, 0x7E, 0xF3, 0xF3, 0x0C, 0xF3,
	0x18, 0xE7, 0x24, 0xF3, 0x24, 0x00, 0x30, 0x00, 0x30, 0xE7, 0x24, 0xDB, 0x24, 0xCF, 0x36, 0xC9,
	0x43, 0xAA, 0x36, 0xB0, 0x2A, 0xAA, 0x30, 0xB6, 0x24, 0xC3, 0x1E, 0xAA, 0x0C, 0x9E, 0x00, 0xB6,
	0xF3, 0x9E, 0xE1, 0xAA, 0x7E, 0xF3, 0xF3, 0xE7, 0xE7, 0xDB, 0xF3, 0xDB, 0x00, 0xCF, 0x00, 0xCF,
	0xE7, 0xDB, 0xDB, 0xDB, 0xCF, 0xC9, 0xC9, 0xBC, 0xAA, 0xC9, 0xB0, 0xD5, 0xAA, 0xCF, 0xB6, 0xDB,
	0xC3, 0xE1, 0xAA, 0x7E, 0xE7, 0xBC, 0x0C, 0xBC, 0x7E, 0x18, 0xBC, 0x0C, 0xBC, 0x7F, 0x7E, 0x00,
	0xD5, 0x12, 0xE1, 0x12, 0x00, 0x36, 0x00, 0x2A, 0xF9, 0x2A, 0xE1, 0x1E, 0xCF, 0x2A, 0xD5, 0x36,
	0xB6, 0x12, 0xAA, 0x06, 0xB6, 0x00, 0xAA, 0x7E, 0x00, 0xD5, 0xED, 0xE1, 0xED, 0x00, 0xC9, 0x00,
	0xD5, 0xF9, 0xD5, 0xE1, 0xE1, 0xCF, 0xD5, 0xD5, 0xC9, 0xB6, 0xED, 0xAA, 0xF9, 0xB6, 0x00, 0xAA,
	0x7E, 0x12, 0xAA, 0x14, 0xA4, 0x0C, 0xA2, 0x12, 0x98, 0x06, 0x9E, 0x7E, 0xED, 0xAA, 0xEB, 0xA4,
	0xF3, 0xA2, 0xED, 0x98, 0xF9, 0x9E, 0x06, 0x9E, 0x7E, 0x0A, 0xA4, 0x06, 0xA8, 0x7E, 0xF5, 0xA4,
	0xF9, 0xA8, 0x7F, 0x7E, 0x00, 0xDB, 0x00, 0x01, 0x18, 0x00, 0x0C, 0xF3, 0x0C, 0xCF, 0x18, 0xDB,
	0x18, 0xB6, 0x0C, 0xAA, 0x0C, 0x98, 0x00, 0x92, 0x7E, 0x00, 0x01, 0xE7, 0x00, 0xF3, 0xF3, 0xF3,
	0xCF, 0xE7, 0xDB, 0xE7, 0xB6, 0xF3, 0xAA, 0xF3, 0x98, 0x00, 0x92, 0x7E, 0x0C, 0xAA, 0x18, 0x9E,
	0x30, 0x9E, 0x55, 0xE7, 0x30, 0xE7, 0x18, 0xCF, 0x7E, 0xF3, 0xAA, 0xE7, 0x9E, 0xCF, 0x9E, 0xAA,
	0xE7, 0xCF, 0xE7, 0xE7, 0xCF, 0x7E, 0x06, 0x9E, 0x7E, 0xF9, 0x9E, 0x7E, 0x06, 0xA8, 0xF9, 0xA8,
	0x7F, 0x7E, 0xCE, 0x05, 0x32, 0x05, 0x32, 0xF9, 0xCE, 0xF9, 0xCE, 0x05, 0xBF, 0xFB, 0xBF, 0xEF,
	0xCE, 0xFB, 0x7E, 0x32, 0x05, 0x39, 0x01, 0x39, 0xF4, 0x32, 0xF9, 0x7E, 0xBF, 0xEF, 0xD3, 0xEA,
	0x39, 0xF4, 0x7F, 0x7E, 0xBA, 0x88, 0x46, 0x88, 0x28, 0x97, 0xD8, 0x97, 0xBA, 0x88, 0x7F, 0x7E,
	0xBA, 0x00, 0x46, 0x00, 0x28, 0xF1, 0xD8, 0xF1, 0xBA, 0x00, 0x7F, 0x7E, 0xEC, 0x88, 0xEC, 0x00,
	0x7E, 0x14, 0x88, 0x14, 0x00, 0x7E, 0xEC, 0x9C, 0x14, 0x9C, 0x7E, 0xEC, 0xB5, 0x14, 0xB5, 0x7E,
	0xEC, 0xD3, 0x14, 0xD3, 0x7E, 0xEC, 0xF1, 0x14, 0xF1, 0x7F, 0x7E, 0x00, 0x88, 0x00, 0x00, 0x7F
};

static const byte SRC_OVERWORLD_MONSTER_DAMAGE[15] = {
	10, 5, 4, 3, 2, 1, 4, 6, 8, 10, 1, 2, 4, 6, 8
};

static const char *const SRC_OVERWORLD_MONSTER_NAMES[15] = {
	"Ness creature", "Giant squid", "Dragon turtle", "Pirate ship", "Hood", "Bear", "Hidden archer",
	"Dark knight", "Evil trent", "Thief", "Orc", "Knight", "Necromancer", "Evil ranger", "Wandering warlock"
};

static const char *const SRC_DUNGEON_MONSTER_NAMES[25] = {
	"Ranger", "Skeleton", "Thief", "Giant rat", "Bat", "Spider", "Viper", "Orc", "Cyclops", "Gelatinous cube",
	"Ettin", "Chest", "Lizard man", "Minotour", "Carrion creeper", "Tangler", "Gremlin", "Wandering eyes",
	"Wraith", "Lich", "Invisible seeker", "Mind whipper", "Zorn", "Daemon", "Balron"
};

static const char *const SRC_LAND_NAMES[4] = {
	"of Lord British",  "of the Feudal Lords", "of the Dark Unknown", "of Danger and Despair"
};

static const char *const SRC_BLOCKED = "Blocked!";
static const char *const SRC_ENTERING = "ing...";
static const char *const SRC_THE_CITY_OF = "The city of";
static const char *const SRC_DUNGEON_LEVEL = " Level    ";
static const char *const SRC_ATTACKED_BY = "Attacked by %s!";
static const char *const SRC_armour_DESTROYED = "Armor destroyed!";
static const char *const SRC_GREMLIN_STOLE = "A gremlin stole some food!";
static const char *const SRC_MENTAL_ATTACK = "Mental attack!";
static const char *const SRC_MISSED = "Missed!";
static const char *const SRC_KILLED = "killed!";
static const char *const SRC_DESTROYED = "destroyed!";
static const char *const SRC_THIEF_STOLE = "Thief stole %s %s";
static const char *const SRC_A = "a";
static const char *const SRC_AN = "an";
static const char *const SRC_HIT = "Hit!";
static const char *const SRC_HIT_CREATURE = "Hit %s! ";
static const char *const SRC_ATTACKS = "attacks!";
static const char *const SRC_DAMAGE = "damage";
static const char *const SRC_BARD_SPEECH1 = "Iolo the Bard sings:";
static const char *const SRC_BARD_SPEECH2 = "Ho eyoh he hum!";
static const char *const SRC_JESTER_SPEECH1 = "Gwino the jester sings:";
static const char *const SRC_JESTER_SPEECH2 = "I've got the key!";
static const char *const SRC_FOUND_KEY = "Thou hast found a key!";
static const char *const SRC_BARD_STOLEN = "Iolo stole something!";
static const char *const SRC_JESTER_STOLEN = "The jester stole something!";
static const char *const SRC_YOU_ARE_AT_SEA = "You are at sea";
static const char *const SRC_YOU_ARE_IN_WOODS = "You are in the woods";
static const char *const SRC_YOU_ARE_IN_LANDS = "You are in the lands";
static const char *const SRC_FIND = "Thou dost find %s";
static const char *const SRC_A_SECRET_DOOR = "a secret door!";
static const char *const SRC_GAIN_HIT_POINTS = "Thou dost gain %u hit points";
static const char *const SRC_OPENED = "opened!";

static const char *const SRC_ACTION_NAMES[26] = {
	"Attack with", "Board", "Cast", "Drop", "Enter", "Fire", "Get", "HyperJump", "Inform and search",
	nullptr, "K-Limb", nullptr, nullptr, nullptr, "Open", "Pass", "Quit", "Ready", "Steal", "Transact",
	"Unlock", "View", nullptr, "X-it", nullptr, "Ztats"
};
static const char *const SRC_HUH = "Huh?";
static const char *const SRC_WHAT = " what?";
static const char *const SRC_FACE_THE_LADDER = "Thou must face the ladder!";
static const char *const SRC_CAUGHT = "Oh no!  Thou wert caught!";
static const char *const SRC_NONE_WILL_TALK = "None will talk to thee!";
static const char *const SRC_NOT_BY_COUNTER = "Thou art not by a counter!";

static const char *const SRC_BUY_SELL = "-Buy, Sell: ";
static const char *const SRC_BUY = "-Buy: ";
static const char *const SRC_SELL = "-Sell: ";
static const char *const SRC_NOTHING = "nothing";
static const char *const SRC_NONE = "none";
static const char *const SRC_NOTHING_HERE = " - nothing here!";
static const char *const SRC_NONE_HERE = " - none here!";
static const char *const SRC_SOLD = "Sold!";
static const char *const SRC_CANT_AFFORD = "Thou canst not afford it!";
static const char *const SRC_DONE = "Done!";
static const char *const SRC_DROP_PENCE_WEAPON_armour = " Pence,Weapon,Armor:";
static const char *const SRC_DROP_PENCE = "Drop pence: ";
static const char *const SRC_DROP_WEAPON = "Drop weapon: ";
static const char *const SRC_DROP_armour = "Drop armor: ";
static const char *const SRC_NOT_THAT_MUCH = "Thou hast not that much!";
static const char *const SRC_OK = "Ok!";
static const char *const SRC_SHAZAM = "Shazam!";
static const char *const SRC_ALAKAZOT = "Alakazot!";
static const char *const SRC_NO_KINGS_PERMISSION = "Thou hast not the king's permission!";
static const char *const SRC_SET_OFF_TRAP = "Thou hast set off a trap!";
static const char *const SRC_THOU_DOST_FIND = "Thou dost find: ";
static const char *const SRC_NO_KEY = "Thou hast not a key!";
static const char *const SRC_INCORRECT_KEY = "Thou has not\x1F""the correct key!";
static const char *const SRC_DOOR_IS_OPEN = "The door is open";
static const char *const SRC_CANT_LEAVE_IT_HERE = "Thy canst not leave it here!";
static const char *const SRC_INVENTORY = "Inventory";
static const char *const SRC_PLAYER = "Player: %s";
static const char *const SRC_PLAYER_DESC = "A Level %u %s %s %s";
static const char *const SRC_PRESS_SPACE_TO_CONTINUE = "Press Space to continue: ";
static const char *const SRC_MORE = " More ";
static const char *const SRC_READY_WEAPON_armour_SPELL = " Weapon,Armor,Spell:";
static const char *const SRC_WEAPON_armour_SPELL[3] = { "weapon", "armor", "spell" };
static const char *const SRC_TRANSPORT_WEAPONS[2] = { "cannons", "lasers" };
static const char *const SRC_NO_EFFECT = "Hmmmm... no effect!";
static const char *const SRC_USED_UP_SPELL = "You've used up that spell!";
static const char *const SRC_DUNGEON_SPELL_ONLY = "Failed, dungeon spell only!";
static const char *const SRC_MONSTER_REMOVED = "Monster removed!";
static const char *const SRC_FAILED = "Failed!";
static const char *const SRC_TELEPORTED = "Teleported!";
static const char *const SRC_FIELD_CREATED = "Field created!";
static const char *const SRC_FIELD_DESTROYED = "Field destroyed!";
static const char *const SRC_LADDER_CREATED = "Ladder created!";
static const char *const SRC_QUEST_COMPLETED = "A quest has been completed!";
static const char *const SRC_EXIT_CRAFT_FIRST = "X-it thy craft first!";
static const char *const SRC_NOTHING_TO_BOARD = "Nothing to Board!";
static const char *const SRC_CANNOT_OPERATE = "Thou canst not determine how\rto operate the craft at this\rtime";

static const char *const SRC_GROCERY_NAMES[8] = {
	"Li'l Karelia's Finnish Grocery", "Adventurer's Supply Post", "", "The Brown Bag", "Fresh Food Marketplace",
	"Rations Unlimited", "Fastest Freshest Food Market", "Exploration Provisioners"
};
static const char *const SRC_GROCERY_SELL = "Used food?  No thanks!";
static const char *const SRC_GROCERY_PACKS1 = "Packs of 10 food cost %u pence";
static const char *const SRC_GROCERY_PACKS2 = "each.  How many dost thou";
static const char *const SRC_GROCERY_PACKS3 = "wish to purchase?";
static const char *const SRC_GROCERY_PACKS_FOOD = "%u packs food";
static const char *const SRC_GROCERY_FIND_PACKS = "Thou dost find %d bags of food!";

static const char *const SRC_WEAPONRY_NAMES[8] = {
	"The Weapon Shop of\nLord Eldric D'Charbonneux", "The Tempered Steel",
	"The Razor's Edge", "Cold Steel Creations", "The Bloody Blade",
	"The Duelo Shop", "Weaponry Supply", "Cold Steel Creations"
};
static const char *const SRC_NO_WEAPONRY_TO_SELL = "Thou hast no weaponry to sell!";
static const char *const SRC_ARMOURY_NAMES[8] = {
	"The Armour Shop of\nLord Eldric D'Charbonneux",  "Max's Armoury", "The Iron Fist",
	"The Hammer -n- Anvil", nullptr, "Defense Specialties Unlimited", nullptr,
	"Custom Armor Works"
};
static const char *const SRC_NO_ARMOUR_TO_SELL = "Thou hast no armour to sell!";
static const char *const SRC_MAGIC_NAMES[8] = {
	"Mad Chad's Magic Shoppe", "Mystic Melinda's", "Words of Power",
	"Psychic Sam's Magical Emporium", nullptr, nullptr, nullptr, "The Unleashed Spell"
};
static const char *const SRC_DONT_BUY_SPELLS = "Sorry, we don't buy spells!";
static const char *SRC_TAVERN_NAMES[8] = {
	"Pub de Varg", "Ye Olde Local Pub", "Dr. Cat's Lair", "Teaser's House", nullptr,
	"Mary's Midway", "Kurtstable's Korner", "Dav's House-o-Suds"
};
static const char *SRC_TAVERN_TEXT[4] = {
	"Thou art broke!\nCome back when thou hast\nsome money to spend.",
	"We have plenty of booze\nalready!",
	"The tavern keeper sayeth:\nHere, have a cold one!", "ale"
};
static const char *SRC_TAVERN_TIPS[13] = {
	"Thou had best know",
	"Thou hast been seduced!\nAfter a long night,\nthou art back.",
	"about space travel!\nThou must destroy at\nleast 20 enemy vessels\nto become an ace!",
	"to watch the %s.",
	"that the princess will give\ngreat reward to the one who\nrescues her, and an extra gift\nto an 8th level ace!",
	"thou must go back in time.",
	"thou should destroy\nthe evil gem!",
	"that many lakes and ponds\nhave strong magical powers!",
	"this is a great game!",
	"that over 1000 years ago,\nMondain the Wizard created an\n"
		"evil gem.  With this gem, he\nis immortal and cannot be\ndefeated.",
	"The quest of --Ultima-- is to\ntraverse the lands in search\nof a time machine.  Upon\n"
		"finding such a device, thou\nshould go back in time to the\n"
		"days before Mondain created\nthe evil gem and destroy him!",
	"wench", "lecher"
};
static const char *SRC_TRANSPORTS_NAMES[8] = {
	"Scooter's Super Duper\nTransport, Inc.", nullptr, nullptr, "Quality Transport Ltd.",
	nullptr, "O.K. New and Used Transport", nullptr, "Sly Sam's Transportation\nSpecialists"
};
static const char *SRC_TRANSPORTS_TEXT[2] = {
	"Sorry, we don't deal in\nused stuff.", "Closed for the day."

};
static const char *const SRC_WITH_KING = " with king";
static const char *const SRC_HE_IS_NOT_HERE = "He is not here!";
static const char *const SRC_HE_REJECTS_OFFER = "He rejects thine offer!";
static const char *const SRC_KING_TEXT[12] = {
	"Dost thou offer pence\ror service: ", "neither", "pence", "service", "How much? ",
	"Thou hast not that much!", "In return I give unto\rthee %u hit points",
	"Thou art on a quest\rfor me already!", "Go now and kill a", "Go forth and find",
	"Do not return until", "thy quest is done!",
};

/*-------------------------------------------------------------------*/

GameResources::GameResources() : LocalResourceFile("ULTIMA1/DATA") {
} 

GameResources::GameResources(Shared::Resources *resManager) : LocalResourceFile(resManager, "ULTIMA1/DATA") {
	Common::copy(SRC_TITLE_MESSAGES, SRC_TITLE_MESSAGES + 13, TITLE_MESSAGES);
	Common::copy(SRC_MAIN_MENU_TEXT, SRC_MAIN_MENU_TEXT + 7, MAIN_MENU_TEXT);
	Common::copy(SRC_CHAR_GEN_TEXT, SRC_CHAR_GEN_TEXT + 14, CHAR_GEN_TEXT);
	Common::copy(SRC_RACE_NAMES, SRC_RACE_NAMES + 4, RACE_NAMES);
	Common::copy(SRC_SEX_NAMES, SRC_SEX_NAMES + 3, SEX_NAMES);
	Common::copy(SRC_CLASS_NAMES, SRC_CLASS_NAMES + 4, CLASS_NAMES);
	Common::copy(SRC_TRANSPORT_NAMES, SRC_TRANSPORT_NAMES + 8, TRANSPORT_NAMES);
	Common::copy(SRC_STAT_NAMES, SRC_STAT_NAMES + 11, STAT_NAMES);
	Common::copy(SRC_STATUS_TEXT, SRC_STATUS_TEXT + 4, STATUS_TEXT);
	Common::copy(SRC_DIRECTION_NAMES, SRC_DIRECTION_NAMES + 4, DIRECTION_NAMES);
	Common::copy(SRC_DUNGEON_MOVES, SRC_DUNGEON_MOVES + 4, DUNGEON_MOVES);
	Common::copy(SRC_LOCATION_NAMES, SRC_LOCATION_NAMES + LOCATION_COUNT, LOCATION_NAMES);
	Common::copy(SRC_LOCATION_X, SRC_LOCATION_X + LOCATION_COUNT, LOCATION_X);
	Common::copy(SRC_LOCATION_Y, SRC_LOCATION_Y + LOCATION_COUNT, LOCATION_Y);
	Common::copy(&SRC_LOCATION_PEOPLE[0][0], &SRC_LOCATION_PEOPLE[0][0] + 150 * 4, &LOCATION_PEOPLE[0][0]);
	Common::copy(&SRC_DUNGEON_DRAW_DATA[0], &SRC_DUNGEON_DRAW_DATA[1964], DUNGEON_DRAW_DATA);
	Common::copy(&SRC_DUNGEON_ITEM_NAMES[0], &SRC_DUNGEON_ITEM_NAMES[2], DUNGEON_ITEM_NAMES);
	Common::copy(&SRC_WEAPON_NAMES_UPPERCASE[0], &SRC_WEAPON_NAMES_UPPERCASE[16], WEAPON_NAMES_UPPERCASE);
	Common::copy(&SRC_WEAPON_NAMES_LOWERCASE[0], &SRC_WEAPON_NAMES_LOWERCASE[16], WEAPON_NAMES_LOWERCASE);
	Common::copy(&SRC_WEAPON_NAMES_ARTICLE[0], &SRC_WEAPON_NAMES_ARTICLE[16], WEAPON_NAMES_ARTICLE);
	Common::copy(SRC_WEAPON_DISTANCES, SRC_WEAPON_DISTANCES + 16, WEAPON_DISTANCES);
	Common::copy(SRC_ARMOUR_NAMES, SRC_ARMOUR_NAMES + 6, ARMOR_NAMES);
	Common::copy(SRC_SPELL_NAMES, SRC_SPELL_NAMES + 11, SPELL_NAMES);
	Common::copy(SRC_SPELL_PHRASES, SRC_SPELL_PHRASES + 14, SPELL_PHRASES);
	Common::copy(&SRC_GEM_NAMES[0], &SRC_GEM_NAMES[4], GEM_NAMES);
	Common::copy(&SRC_OVERWORLD_MONSTER_DAMAGE[0], &SRC_OVERWORLD_MONSTER_DAMAGE[15], OVERWORLD_MONSTER_DAMAGE);
	Common::copy(&SRC_OVERWORLD_MONSTER_NAMES[0], &SRC_OVERWORLD_MONSTER_NAMES[15], OVERWORLD_MONSTER_NAMES);
	Common::copy(&SRC_DUNGEON_MONSTER_NAMES[0], &SRC_DUNGEON_MONSTER_NAMES[25], DUNGEON_MONSTER_NAMES);
	Common::copy(&SRC_LAND_NAMES[0], &SRC_LAND_NAMES[4], LAND_NAMES);

	BLOCKED = SRC_BLOCKED;
	ENTERING = SRC_ENTERING;
	THE_CITY_OF = SRC_THE_CITY_OF;
	DUNGEON_LEVEL = SRC_DUNGEON_LEVEL;
	ATTACKED_BY = SRC_ATTACKED_BY;
	ARMOR_DESTROYED = SRC_armour_DESTROYED;
	GREMLIN_STOLE = SRC_GREMLIN_STOLE;
	MENTAL_ATTACK = SRC_MENTAL_ATTACK;
	MISSED = SRC_MISSED;
	KILLED = SRC_KILLED;
	DESTROYED = SRC_DESTROYED;
	THIEF_STOLE = SRC_THIEF_STOLE;
	A = SRC_A;
	AN = SRC_AN;
	HIT = SRC_HIT;
	HIT_CREATURE = SRC_HIT_CREATURE;
	ATTACKS = SRC_ATTACKS;
	DAMAGE = SRC_DAMAGE;
	BARD_SPEECH1 = SRC_BARD_SPEECH1;
	BARD_SPEECH2 = SRC_BARD_SPEECH2;
	JESTER_SPEECH1 = SRC_JESTER_SPEECH1;
	JESTER_SPEECH2 = SRC_JESTER_SPEECH2;
	FOUND_KEY = SRC_FOUND_KEY;
	BARD_STOLEN = SRC_BARD_STOLEN;
	JESTER_STOLEN = SRC_JESTER_STOLEN;
	YOU_ARE_AT_SEA = SRC_YOU_ARE_AT_SEA;
	YOU_ARE_IN_WOODS = SRC_YOU_ARE_IN_WOODS;
	YOU_ARE_IN_LANDS = SRC_YOU_ARE_IN_LANDS;
	FIND = SRC_FIND;
	A_SECRET_DOOR = SRC_A_SECRET_DOOR;
	GAIN_HIT_POINTS = SRC_GAIN_HIT_POINTS;
	OPENED = SRC_OPENED;

	Common::copy(&SRC_ACTION_NAMES[0], &SRC_ACTION_NAMES[26], ACTION_NAMES);
	HUH = SRC_HUH;
	WHAT = SRC_WHAT;
	FACE_THE_LADDER = SRC_FACE_THE_LADDER;
	CAUGHT = SRC_CAUGHT;
	NONE_WILL_TALK = SRC_NONE_WILL_TALK;
	NOT_BY_COUNTER = SRC_NOT_BY_COUNTER;
	BUY_SELL = SRC_BUY_SELL;
	BUY = SRC_BUY;
	SELL = SRC_SELL;
	NOTHING = SRC_NOTHING;
	NONE = SRC_NONE;
	NOTHING_HERE = SRC_NOTHING_HERE;
	NONE_HERE = SRC_NONE_HERE;
	SOLD = SRC_SOLD;
	CANT_AFFORD = SRC_CANT_AFFORD;
	DONE = SRC_DONE;
	DROP_PENCE_WEAPON_armour = SRC_DROP_PENCE_WEAPON_armour;
	DROP_PENCE = SRC_DROP_PENCE;
	DROP_WEAPON = SRC_DROP_WEAPON;
	DROP_armour = SRC_DROP_armour;
	NOT_THAT_MUCH = SRC_NOT_THAT_MUCH;
	OK = SRC_OK;
	SHAZAM = SRC_SHAZAM;
	ALAKAZOT = SRC_ALAKAZOT;
	NO_KINGS_PERMISSION = SRC_NO_KINGS_PERMISSION;
	SET_OFF_TRAP = SRC_SET_OFF_TRAP;
	THOU_DOST_FIND = SRC_THOU_DOST_FIND;
	NO_KEY = SRC_NO_KEY;
	INCORRECT_KEY = SRC_INCORRECT_KEY;
	DOOR_IS_OPEN = SRC_DOOR_IS_OPEN;
	CANT_LEAVE_IT_HERE = SRC_CANT_LEAVE_IT_HERE;
	INVENTORY = SRC_INVENTORY;
	PLAYER = SRC_PLAYER;
	PLAYER_DESC = SRC_PLAYER_DESC;
	PRESS_SPACE_TO_CONTINUE = SRC_PRESS_SPACE_TO_CONTINUE;
	MORE = SRC_MORE;
	READY_WEAPON_armour_SPELL = SRC_READY_WEAPON_armour_SPELL;
	Common::copy(SRC_WEAPON_armour_SPELL, SRC_WEAPON_armour_SPELL + 3, WEAPON_armour_SPELL);
	Common::copy(SRC_TRANSPORT_WEAPONS, SRC_TRANSPORT_WEAPONS + 2, TRANSPORT_WEAPONS);
	NO_EFFECT = SRC_NO_EFFECT;
	USED_UP_SPELL = SRC_USED_UP_SPELL;
	DUNGEON_SPELL_ONLY = SRC_DUNGEON_SPELL_ONLY;
	MONSTER_REMOVED = SRC_MONSTER_REMOVED;
	FAILED = SRC_FAILED;
	TELEPORTED = SRC_TELEPORTED;
	FIELD_CREATED = SRC_FIELD_CREATED;
	FIELD_DESTROYED = SRC_FIELD_DESTROYED;
	LADDER_CREATED = SRC_LADDER_CREATED;
	QUEST_COMPLETED = SRC_QUEST_COMPLETED;
	EXIT_CRAFT_FIRST = SRC_EXIT_CRAFT_FIRST;
	NOTHING_TO_BOARD = SRC_NOTHING_TO_BOARD;
	CANNOT_OPERATE = SRC_CANNOT_OPERATE;

	Common::copy(SRC_GROCERY_NAMES, SRC_GROCERY_NAMES + 8, GROCERY_NAMES);
	GROCERY_SELL = SRC_GROCERY_SELL;
	GROCERY_PACKS1 = SRC_GROCERY_PACKS1;
	GROCERY_PACKS2 = SRC_GROCERY_PACKS2;
	GROCERY_PACKS3 = SRC_GROCERY_PACKS3;
	GROCERY_PACKS_FOOD = SRC_GROCERY_PACKS_FOOD;
	GROCERY_FIND_PACKS = SRC_GROCERY_FIND_PACKS;
	Common::copy(SRC_WEAPONRY_NAMES, SRC_WEAPONRY_NAMES + 8, WEAPONRY_NAMES);
	NO_WEAPONRY_TO_SELL = SRC_NO_WEAPONRY_TO_SELL;
	Common::copy(SRC_ARMOURY_NAMES, SRC_ARMOURY_NAMES + 8, ARMOURY_NAMES);
	NO_ARMOUR_TO_SELL = SRC_NO_ARMOUR_TO_SELL;
	Common::copy(SRC_MAGIC_NAMES, SRC_MAGIC_NAMES + 8, MAGIC_NAMES);
	DONT_BUY_SPELLS = SRC_DONT_BUY_SPELLS;
	Common::copy(SRC_TAVERN_NAMES, SRC_TAVERN_NAMES + 8, TAVERN_NAMES);
	Common::copy(SRC_TAVERN_TEXT, SRC_TAVERN_TEXT + 4, TAVERN_TEXT);
	Common::copy(SRC_TAVERN_TIPS, SRC_TAVERN_TIPS + 13, TAVERN_TIPS);
	Common::copy(SRC_TRANSPORTS_NAMES, SRC_TRANSPORTS_NAMES + 8, TRANSPORTS_NAMES);
	Common::copy(SRC_TRANSPORTS_TEXT, SRC_TRANSPORTS_TEXT + 2, TRANSPORTS_TEXT);
	WITH_KING = SRC_WITH_KING;
	HE_IS_NOT_HERE = SRC_HE_IS_NOT_HERE;
	HE_REJECTS_OFFER = SRC_HE_REJECTS_OFFER;
	Common::copy(&SRC_KING_TEXT[0], &SRC_KING_TEXT[12], KING_TEXT);
}

void GameResources::synchronize() {
	syncStrings(TITLE_MESSAGES, 13);
	syncStrings(MAIN_MENU_TEXT, 7);
	syncStrings(CHAR_GEN_TEXT, 14);
	syncStrings(RACE_NAMES, 4);
	syncStrings(SEX_NAMES, 3);
	syncStrings(CLASS_NAMES, 4);
	syncStrings(TRANSPORT_NAMES, 8);
	syncStrings(STAT_NAMES, 11);
	syncStrings(STATUS_TEXT, 4);
	syncStrings(DIRECTION_NAMES, 4);
	syncStrings(DUNGEON_MOVES, 4);
	syncStrings(LOCATION_NAMES, LOCATION_COUNT);
	syncBytes(LOCATION_X, LOCATION_COUNT);
	syncBytes(LOCATION_Y, LOCATION_COUNT);
	syncNumbers2D((int *)LOCATION_PEOPLE, 150, 4);
	syncBytes(DUNGEON_DRAW_DATA, 1964);
	syncStrings(DUNGEON_ITEM_NAMES, 2);
	syncStrings(WEAPON_NAMES_UPPERCASE, 16);
	syncStrings(WEAPON_NAMES_LOWERCASE, 16);
	syncStrings(WEAPON_NAMES_ARTICLE, 16);
	syncBytes(WEAPON_DISTANCES, 16);
	syncStrings(ARMOR_NAMES, 6);
	syncStrings(SPELL_NAMES, 11);
	syncStrings(SPELL_PHRASES, 14);
	syncStrings(GEM_NAMES, 4);
	syncBytes(OVERWORLD_MONSTER_DAMAGE, 15);
	syncStrings(OVERWORLD_MONSTER_NAMES, 15);
	syncStrings(DUNGEON_MONSTER_NAMES, 25);
	syncStrings(LAND_NAMES, 4);

	syncString(BLOCKED);
	syncString(ENTERING);
	syncString(THE_CITY_OF);
	syncString(DUNGEON_LEVEL);
	syncString(ATTACKED_BY);
	syncString(ARMOR_DESTROYED);
	syncString(GREMLIN_STOLE);
	syncString(MENTAL_ATTACK);
	syncString(MISSED);
	syncString(KILLED);
	syncString(DESTROYED);
	syncString(THIEF_STOLE);
	syncString(A);
	syncString(AN);
	syncString(HIT);
	syncString(HIT_CREATURE);
	syncString(ATTACKS);
	syncString(DAMAGE);
	syncString(BARD_SPEECH1);
	syncString(BARD_SPEECH2);
	syncString(JESTER_SPEECH1);
	syncString(JESTER_SPEECH2);
	syncString(FOUND_KEY);
	syncString(BARD_STOLEN);
	syncString(JESTER_STOLEN);
	syncString(YOU_ARE_AT_SEA);
	syncString(YOU_ARE_IN_WOODS);
	syncString(YOU_ARE_IN_LANDS);
	syncString(FIND);
	syncString(A_SECRET_DOOR);
	syncString(GAIN_HIT_POINTS);
	syncString(OPENED);

	syncStrings(ACTION_NAMES, 26);
	syncString(HUH);
	syncString(WHAT);
	syncString(FACE_THE_LADDER);
	syncString(CAUGHT);
	syncString(NONE_WILL_TALK);
	syncString(NOT_BY_COUNTER);
	syncString(BUY_SELL);
	syncString(BUY);
	syncString(SELL);
	syncString(NOTHING);
	syncString(NONE);
	syncString(NOTHING_HERE);
	syncString(NONE_HERE);
	syncString(SOLD);
	syncString(CANT_AFFORD);
	syncString(DONE);
	syncString(DROP_PENCE_WEAPON_armour);
	syncString(DROP_PENCE);
	syncString(DROP_WEAPON);
	syncString(DROP_armour);
	syncString(NOT_THAT_MUCH);
	syncString(OK);
	syncString(SHAZAM);
	syncString(ALAKAZOT);
	syncString(NO_KINGS_PERMISSION);
	syncString(SET_OFF_TRAP);
	syncString(THOU_DOST_FIND);
	syncString(NO_KEY);
	syncString(INCORRECT_KEY);
	syncString(DOOR_IS_OPEN);
	syncString(CANT_LEAVE_IT_HERE);
	syncString(INVENTORY);
	syncString(PLAYER);
	syncString(PLAYER_DESC);
	syncString(PRESS_SPACE_TO_CONTINUE);
	syncString(MORE);
	syncString(READY_WEAPON_armour_SPELL);
	syncStrings(WEAPON_armour_SPELL, 3);
	syncStrings(TRANSPORT_WEAPONS, 2);
	syncString(NO_EFFECT);
	syncString(USED_UP_SPELL);
	syncString(DUNGEON_SPELL_ONLY);
	syncString(MONSTER_REMOVED);
	syncString(FAILED);
	syncString(TELEPORTED);
	syncString(FIELD_CREATED);
	syncString(FIELD_DESTROYED);
	syncString(LADDER_CREATED);
	syncString(QUEST_COMPLETED);
	syncString(EXIT_CRAFT_FIRST);
	syncString(NOTHING_TO_BOARD);
	syncString(CANNOT_OPERATE);

	syncStrings(GROCERY_NAMES, 8);
	syncString(GROCERY_SELL);
	syncString(GROCERY_PACKS1);
	syncString(GROCERY_PACKS2);
	syncString(GROCERY_PACKS3);
	syncString(GROCERY_PACKS_FOOD);
	syncString(GROCERY_FIND_PACKS);
	syncStrings(WEAPONRY_NAMES, 8);
	syncString(NO_WEAPONRY_TO_SELL);
	syncStrings(ARMOURY_NAMES, 8);
	syncString(NO_ARMOUR_TO_SELL);
	syncStrings(MAGIC_NAMES, 8);
	syncString(DONT_BUY_SPELLS);
	syncStrings(TAVERN_NAMES, 8);
	syncStrings(TAVERN_TEXT, 10);
	syncStrings(TAVERN_TIPS, 13);
	syncStrings(TRANSPORTS_NAMES, 8);
	syncStrings(TRANSPORTS_TEXT, 2);
	syncString(WITH_KING);
	syncString(HE_IS_NOT_HERE);
	syncString(HE_REJECTS_OFFER);
	syncStrings(KING_TEXT, 12);
}

} // End of namespace Ultima1
} // End of namespace Ultima
