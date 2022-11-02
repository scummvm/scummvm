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

#ifndef CHEWY_ROOMS_ROOM12_H
#define CHEWY_ROOMS_ROOM12_H

namespace Chewy {
namespace Rooms {

class Room12 {
private:
	static void bork_ok();
	static void init_bork();

public:
	static void entry();
	static bool timer(int16 t_nr, int16 ani_nr);

	static void use_linke_rohr();
	static int16 chewy_trans();
	static int16 useTransformerTube();
	static int16 use_terminal();
	static void talk_bork();
};

} // namespace Rooms
} // namespace Chewy

#endif
