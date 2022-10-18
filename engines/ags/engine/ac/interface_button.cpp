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

#if defined (OBSOLETE)

#include "ags/shared/ac/interface_button.h"

namespace AGS3 {

void InterfaceButton::set(int xx, int yy, int picc, int overpicc, int actionn) {
	x = xx;
	y = yy;
	pic = picc;
	overpic = overpicc;
	leftclick = actionn;
	pushpic = 0;
	rightclick = 0;
	flags = IBFLG_ENABLED;
	reserved_for_future = 0;
}

} // namespace AGS3

#endif // OBSOLETE
