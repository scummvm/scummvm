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

#ifndef CHEWY_ROOMS_ROOM97_H
#define CHEWY_ROOMS_ROOM97_H

namespace Chewy {
namespace Rooms {

class Room97 {
private:
	static int _word18DB2E;
	static bool _slimeThrown;
	static bool _bool18DB32;
	static int _word18DB34;
	static int _word18DB36;
	static int _word18DB38;
	static bool _bool18DB3A;

private:
	static void setup_func();
	static void proc2();
	static void proc3();
	static void proc4();
	static void proc12();
	static void proc13();
	static void sensorAnim();

public:
	static void entry();
	static void xit();
	static void gedAction(int index);

	static int proc5();
	static int proc6();
	static int proc7();
	static int proc8();
	static int proc9();
	static int proc10();
	static int proc11();
	static int throwSlime();
};

} // namespace Rooms
} // namespace Chewy

#endif
