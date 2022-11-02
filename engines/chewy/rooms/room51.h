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

#ifndef CHEWY_ROOMS_ROOM51_H
#define CHEWY_ROOMS_ROOM51_H

namespace Chewy {
namespace Rooms {

class Room51 {
private:
	static bool _flag;
	static bool _enemyFlag[2];
	static int16 _tmpx, _tmpy;
	static int _index;

	static void setup_func();
	static void timer_action(int16 t_nr, int16 obj_nr);

public:
	static void entry();
	static void xit(int16 eib_nr);
	static bool timer(int16 t_nr, int16 ani_nr);

	static int16 use_door(int16 txt_nr);
};

} // namespace Rooms
} // namespace Chewy

#endif
