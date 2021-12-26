/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef CHEWY_ROOMS_ROOM0_H
#define CHEWY_ROOMS_ROOM0_H

namespace Chewy {
namespace Rooms {

#include "chewy/ngsdefs.h"

/**
 * Prison Cell - Starting room
 */
class Room0 {
private:
	static void eyeStart(int16 mode);

	static void auge_wait();

	static void auge_shoot();

	static void auge_schleim_back();

	static void ch_schleim_auge();

	static void fuetter_start(int16 mode);

	static void ch_fuetter();
	static void ch_kissen();

	static void kissen_wurf();

	static void ani_klappe_delay();

	static void calc_auge_click(int16 ani_nr);

	static void calc_kissen_click(int16 ani_nr);
public:
	static void entry();

	static void eyeAnim();
	static void fuett_ani();
	static bool getPillow();
	static bool pullSlime();
};

} // namespace Rooms
} // namespace Chewy

#endif
