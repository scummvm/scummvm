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

#include "glk/alan3/compatibility.h"

namespace Glk {
namespace Alan3 {

/*----------------------------------------------------------------------*/
static bool is3_0Alpha(const byte version[]) {
	return version[3] == 3 && version[2] == 0 && version[0] == 'a';
}

/*----------------------------------------------------------------------*/
static bool is3_0Beta(const byte version[]) {
	return version[3] == 3 && version[2] == 0 && version[0] == 'b';
}

/*----------------------------------------------------------------------*/
static int correction(const byte version[]) {
	return version[1];
}

/*======================================================================*/
bool isPreAlpha5(const byte version[4]) {
	return is3_0Alpha(version) && correction(version) < 5;
}

/*======================================================================*/
bool isPreBeta2(const byte version[4]) {
	return is3_0Alpha(version) || (is3_0Beta(version) && correction(version) == 1);
}

/*======================================================================*/
bool isPreBeta3(const byte version[4]) {
	return is3_0Alpha(version) || (is3_0Beta(version) && correction(version) <= 2);
}

/*======================================================================*/
bool isPreBeta4(const byte version[4]) {
	return is3_0Alpha(version) || (is3_0Beta(version) && correction(version) <= 3);
}

/*======================================================================*/
bool isPreBeta5(const byte version[4]) {
	return is3_0Alpha(version) || (is3_0Beta(version) && correction(version) <= 4);
}

} // End of namespace Alan3
} // End of namespace Glk
