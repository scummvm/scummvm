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

#ifndef CHEWY_ROOMS_ROOM17_H
#define CHEWY_ROOMS_ROOM17_H

namespace Chewy {
namespace Rooms {

class Room17 {
private:
	static void door_kommando(int16 mode);

public:
	static void entry();
	static void xit();
	static bool timer(int16 t_nr, int16 ani_nr);
	static void gedAction(int index);

	static int16 use_seil();
	static void plot_seil();
	static void kletter_down();
	static void kletter_up();
	static void calc_seil();
	static void close_door();
	static int16 energy_lever();
	static int16 get_oel();

};

} // namespace Rooms
} // namespace Chewy

#endif
