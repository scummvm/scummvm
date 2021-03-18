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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/ll/llinc.h"

#include "trecision/trecision.h"

namespace Trecision {

/*-----------------07/02/97 17.31-------------------
					BattutaPrint
--------------------------------------------------*/
void BattutaPrint(int x, int y, int c, const char *txt) {
	g_vm->_oldSdText.set(g_vm->_sdText);

	g_vm->_sdText.x = x;
	g_vm->_sdText.y = y;
	g_vm->_sdText.tcol = c;
	g_vm->_sdText.sign = txt;
}

} // End of namespace Trecision
