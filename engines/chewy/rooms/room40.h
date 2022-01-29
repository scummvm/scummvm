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

#ifndef CHEWY_ROOMS_ROOM40_H
#define CHEWY_ROOMS_ROOM40_H

namespace Chewy {
namespace Rooms {

class Room40 {
private:
	static void move_train(int16 mode);
	static void setup_func();
	static int16 use_schalter(int16 aad_nr);
	static void bmeister_dia(int16 aad_nr);

public:
	static void entry(int16 eib_nr);
	static void xit(int16 eib_nr);
	static bool timer(int16 t_nr, int16 ani_nr);

	static int16 use_mr_pumpkin();
	static void talk_police();
	static void talk_handler();
	static int16 use_haendler();
	static int16 use_bmeister();
	static bool use_police();
	static int16 use_tele();
};

} // namespace Rooms
} // namespace Chewy

#endif
