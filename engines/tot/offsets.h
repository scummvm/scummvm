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
#include "common/scummsys.h"


namespace Tot {

	const int FONT_LITT_OFFSET_ES = 153584;
	const int FONT_EURO_OFFSET_ES = 159681;
	const int FONT_LITT_OFFSET_EN = 153010;
	const int FONT_EURO_OFFSET_EN = 159107;

    static const long menuOffsets_ES[8][2] = {
        {0,     16004},
        {16004, 24535},
        {40539, 24535},
        {65074, 26745},
        {91819, 16004},
        {107823, 24535},
        {132358, 14969},
        {147327, 7148},
    };

    static const long menuOffsets_EN[8][2] = {
        {0,     16004},
        {16004, 24535},
        {40539, 24535},
        {65074, 26745},
        {91819, 16004},
        {107823, 24535},
        {132358, 14417},
        {146775, 7148},
    };

} // End of namespace Tot
