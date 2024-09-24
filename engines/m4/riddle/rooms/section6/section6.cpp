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

#include "m4/riddle/rooms/section6/section6.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

const int16 SECTION6_NORMAL_DIRS[] = { 200, 201, 202, 203, -1 };
const char *SECTION6_NORMAL_NAMES[] = {
	"tt walker 1", "tt walker 2", "tt walker 3",
	"tt walker 4", "tt walker 5"
};
const int16 SECTION6_SHADOW_DIRS[] = { 210, 211, 212, 213, -1 };
const char *SECTION6_SHADOW_NAMES[] = {
	"tt walker shadow 1", "tt walker shadow 2",
	"tt walker shadow 3", "tt walker shadow 4",
	"tt walker shadow 5"
};


Section6::Section6() : Rooms::Section() {
	add(603, &_room603);
	add(604, &_room604);
	add(605, &_room605);
	add(607, &_room607);
	add(608, &_room608);
	add(610, &_room610);
	add(615, &_room615);
	add(620, &_room620);
	add(621, &_room621);
	add(622, &_room622);
	add(623, &_room623);
	add(624, &_room624);
	add(625, &_room625);
	add(626, &_room626);
	add(627, &_room627);
	add(628, &_room628);
	add(629, &_room629);
	add(630, &_room630);
	add(631, &_room631);
	add(632, &_room632);
	add(633, &_room633);
	add(634, &_room634);
	add(635, &_room635);
	add(636, &_room636);
	add(637, &_room637);
	add(638, &_room638);
	add(640, &_room640);
	add(641, &_room641);
	add(642, &_room642);
	add(643, &_room643);
	add(644, &_room644);
	add(645, &_room645);
	add(646, &_room646);
	add(647, &_room647);
	add(648, &_room648);
	add(649, &_room649);
	add(650, &_room650);
	add(651, &_room651);
}

void Section6::daemon() {
	_G(kernel).continue_handling_trigger = true;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
