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

#ifndef CHEWY_ROOMS_ROOM68_H
#define CHEWY_ROOMS_ROOM68_H

namespace Chewy {
namespace Rooms {

class Room68 {
private:
	static void setup_func();
	static void talkToIndigo(int16 aad_nr);

public:
	static void entry();
	static void xit();
	static bool timer(int16 t_nr, int16 ani_nr);

	static void lookAtCactus();
	static int16 useIndigo();
	static void talkToBartender();
	static void useDressOnNichelle(int16 aad_nr);
	static void talkWithParrot();
	static int16 useDiva();
	static int16 useBartender();
	static void calcDiva();
	static int16 useParrot();
	static void talkToIndigo();
};

} // namespace Rooms
} // namespace Chewy

#endif
