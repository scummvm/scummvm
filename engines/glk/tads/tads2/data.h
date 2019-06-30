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

#ifndef GLK_TADS_TADS2_DATA
#define GLK_TADS_TADS2_DATA

#include "common/scummsys.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

enum DataType {
	DAT_NUMBER  =  1,
	DAT_OBJECT  =  2,
	DAT_SSTRING =  3,
	DAT_BASEPTR =  4,
	DAT_NIL     =  5,		///< nil, as in FALSE or empty list
	DAT_CODE    =  6,
	DAT_LIST    =  7,
	DAT_TRUE    =  8,		///< inverse of nil
	DAT_DSTRING =  9,
	DAT_FNADDR  = 10,		///< a function address
	DAT_TPL     = 11,		///< template list pointer
	DAT_PROPNUM = 13,		///< a property number
	DAT_DEMAND  = 14,		///< special flag: use callback to set on use
	DAT_SYN     = 15,		///< synonym to indicated property value
	DAT_REDIR   = 16,		///< redirection to different object
	DAT_TPL2    = 17		///< new-style template
};
typedef int dattyp;

/* determine the size of a piece of data */
uint datsiz(dattyp typ, const void *valptr);

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
