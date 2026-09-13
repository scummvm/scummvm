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

#include "ultima/ultima2/data/data.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

const char *const WEAPON_NAMES[WEAPON_COUNT] = {
	"HANDS", "DAGGER", "MACE", "AXE", "BOW", "SWORD",
	"GREAT SWORD", "LIGHT SWORD", "PHASER", "QUICK SWORD"
};

const char *const ARMOR_NAMES[ARMOR_COUNT] = {
	"SKIN", "CLOTH", "LEATHER", "CHAIN", "PLATE", "REFLECT", "POWER"
};

const char *const SPELL_NAMES[SPELL_COUNT] = {
	"NONE", "LIGHT", "DOWN LADDER", "UP LADDER", "PASSWALL",
	"SURFACE", "PRAYER", "MAGIC MISSILE", "BLINK", "KILL"
};

const char *const ITEM_NAMES[ITEM_COUNT] = {
	"RING", "WAND", "STAFF", "BOOTS", "CLOAK", "HELM", "GEM", "ANKH",
	"RED GEM", "SKULL KEY", "GREEN GEM", "BRASS BUTTON", "BLUE TASSLE",
	"STRANGE COIN", "GREEN IDOL", "TRI LITHIUM"
};

} // namespace Data
} // namespace Ultima2
} // namespace Ultima
