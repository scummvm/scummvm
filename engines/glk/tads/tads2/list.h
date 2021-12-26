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

/* List definitions
 *
 * A TADS run-time list is essentially a packed counted array.
 * The first thing in a list is a ushort, which specifies the
 * number of elements in the list.  The list elements are then
 * packed into the list immediately following.
 */

#ifndef GLK_TADS_TADS2_LIST
#define GLK_TADS_TADS2_LIST

#include "glk/tads/tads2/lib.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/* advance a list pointer/size pair to the next element of a list */
void lstadv(uchar **lstp, uint *sizp);

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
