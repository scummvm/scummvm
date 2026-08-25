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

#include "ultima/ultima1/data/entity.h"

namespace Ultima {
namespace Ultima1 {
namespace Data {

const char *OVERWORLD_MONSTERS[] = {
	"Ness creature",
	"Giant squid",
	"Dragon turtle",
	"Pirate ship",
	"Hood",
	"Bear",
	"Hidden archer",
	"Dark knight",
	"Evil trent",
	"Thief",
	"Orc",
	"Knight",
	"Necromancer",
	"Evil ranger",
	"Wandering warlock"
};

void OverworldEntity::synchronize(Common::Serializer &s) {
	s.syncMultipleLE(_type, _data, _x, _y, _hits, _unused1, _unused2, _unused3);
}

} // namespace Data
} // namespace Ultima1
} // namespace Ultima
