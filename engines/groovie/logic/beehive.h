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

#ifndef GROOVIE_LOGIC_BEEHIVE_H
#define GROOVIE_LOGIC_BEEHIVE_H

#include "common/system.h"

namespace Groovie {

class BeehiveGame {
public:
	BeehiveGame();
	~BeehiveGame();

	void run(byte *scriptVariables);

private:
	void sub02(int8 *a1, int8 *a2);
	void sub18(int8 a1, int8 *a2, int8 *a3);
	int8 getHexDifference();
	int8 getTotal(int8 *hexagons);

	int8 _beehiveState[61];

};

} // End of Groovie namespace

#endif // GROOVIE_LOGIC_BEEHIVE_H
