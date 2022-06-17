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

struct MarketData {
	const byte FOOD_COST[4] = { 5, 10, 20, 200 };
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

} // namespace MM1
} // namespace MM

#endif
