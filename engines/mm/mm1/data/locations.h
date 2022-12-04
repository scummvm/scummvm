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

#ifndef MM1_DATA_LOCATIONS_H
#define MM1_DATA_LOCATIONS_H

#include "common/serializer.h"

namespace MM {
namespace MM1 {

struct BlacksmithData {
	const byte BLACKSMITH_CLASS_USAGE[6] = { 0x20, 0x10, 8, 4, 2, 1 };
};

struct BuyWeaponData {
	const byte WEAPONS_TOWN1[6] = { 2, 3, 5, 61, 62, 86 };
	const byte WEAPONS_TOWN2[6] = { 4, 6, 8, 63, 87, 88 };
	const byte WEAPONS_TOWN3[6] = { 9, 10, 62, 64, 89, 91 };
	const byte WEAPONS_TOWN4[6] = { 23, 67, 69, 93, 97, 99 };
	const byte WEAPONS_TOWN5[6] = { 7, 11, 64, 65, 90, 92 };
	const byte *WEAPONS[5] = {
		WEAPONS_TOWN1, WEAPONS_TOWN2, WEAPONS_TOWN3,
		WEAPONS_TOWN4, WEAPONS_TOWN5
	};
};

struct BuyArmorData {
	const byte ARMOR_TOWN1[6] = { 156, 121, 122, 123, 124, 125 };
	const byte ARMOR_TOWN2[6] = { 156, 157, 121, 122, 123, 124 };
	const byte ARMOR_TOWN3[6] = { 157, 121, 124, 125, 126, 127 };
	const byte ARMOR_TOWN4[6] = { 160, 128, 131, 132, 133, 134 };
	const byte 	ARMOR_TOWN5[6] = { 157, 123, 124, 125, 126, 127 };
	const byte *ARMOR[5] = {
		ARMOR_TOWN1, ARMOR_TOWN2, ARMOR_TOWN3,
		ARMOR_TOWN4, ARMOR_TOWN5
	};
};

struct BuyMiscData {
	const byte MISC_TOWN1[6] = { 172, 171, 175, 178, 185, 192 };
	const byte MISC_TOWN2[6] = { 172, 171, 174, 183, 188, 195 };
	const byte MISC_TOWN3[6] = { 173, 175, 176, 179, 184, 195 };
	const byte MISC_TOWN4[6] = { 180, 196, 211, 215, 219, 223 };
	const byte MISC_TOWN5[6] = { 171, 173, 177, 185, 186, 192 };
	const byte *MISC[5] = {
		MISC_TOWN1, MISC_TOWN2, MISC_TOWN3,
		MISC_TOWN4, MISC_TOWN5
	};
};

#define MAX_FOOD 40

struct MarketData {
	const byte FOOD_COST[5] = { 5, 10, 20, 200, 50 };
};

struct TempleData {
	const uint16 HEAL_COST1[5] = { 2000, 5000, 5000, 2000, 8000 };
	const uint16 HEAL_COST2[5] = { 200, 500, 500, 200, 1000 };
	const uint16 HEAL_COST3[5] = { 25, 50, 50, 25, 100 };
	const uint16 UNCURSE_COST[5] = { 500, 1000, 1000, 1012, 1500 };
	const uint16 ALIGNMENT_COST[5] = { 250, 200, 200, 200, 250 };
	const uint16 DONATE_COST[5] = { 100, 100, 100, 25, 200 };
	const byte ALIGNMENT_VALS[3] = { 8, 0x10, 0x18 };
	const byte DONATE_VALS[5] = { 1, 2, 4, 8, 0x10 };
};

struct TrainingData {
	const int TRAINING_COSTS1[7] = {
		25, 50, 100, 200, 400, 800, 1500
	};
	const int TRAINING_COSTS2[7] = {
		40, 75, 150, 300, 600, 1200, 2500
	};
};

struct TownData {
	static const byte TOWN_MAP_ID1[5];
	static const byte TOWN_MAP_ID2[5];
	static const byte TOWN_MAP_X[5];
	static const byte TOWN_MAP_Y[5];
};

} // namespace MM1
} // namespace MM

#endif
