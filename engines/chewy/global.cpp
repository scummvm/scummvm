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

#include "chewy/global.h"
#include "chewy/chewy.h"
#include "chewy/types.h"
#include "chewy/defines.h"
#include "chewy/file.h"
#include "chewy/ngshext.h"
#include "chewy/text.h"
#include "chewy/room.h"
#include "chewy/object.h"
#include "chewy/timer.h"
#include "chewy/detail.h"
#include "chewy/effect.h"
#include "chewy/atds.h"
#include "chewy/movclass.h"
#include "chewy/gedclass.h"
#include "chewy/bitclass.h"

namespace Chewy {

#define MAX_RAND_NO_USE 6

const uint8 RAND_NO_USE[MAX_RAND_NO_USE] = {0, 1, 3, 4, 5, 6};


Globals *g_globals = nullptr;

Globals::Globals() {
	g_globals = this;

	static const int16 R45_PINFO[R45_MAX_PERSON][4] = {
		{ 0, 4, 1490, 1500 },
		{ 0, 1, 180, 190 },
		{ 0, 1, 40, 60 },
		{ 0, 1, 40, 150 },
		{ 0, 1, 40, 230 },
		{ 0, 1, 40, 340 },
		{ 0, 1, 49, 50 },
		{ 0, 1, 5, 90 },
		{ 0, 1, 7, 190 }
	};
	Common::copy(&R45_PINFO[0][0], &R45_PINFO[0][0] + R45_MAX_PERSON * 4, &_r45_pinfo[0][0]);
}

Globals::~Globals() {
	g_globals = nullptr;
}

} // namespace Chewy
