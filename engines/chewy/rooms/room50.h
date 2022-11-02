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

#ifndef CHEWY_ROOMS_ROOM50_H
#define CHEWY_ROOMS_ROOM50_H

namespace Chewy {
namespace Rooms {

class Room50 {
private:
	static bool _wasser;

	static void stop_page();
	static void go_page();
	static void aad_page(int16 aad_nr, int16 ani_nr);
	static void setup_func();
	static void calc_wasser();
	static void calc_treppe();

public:
	static void entry(int16 eib_nr);
	static void xit(int16 eib_nr);
	static bool timer(int16 t_nr, int16 ani_nr);
	static void gedAction(int index);

	static int16 use_gutschein();
	static int16 use_gum();
	static void talk_page();
	static void stop_cigar();
};

} // namespace Rooms
} // namespace Chewy

#endif
