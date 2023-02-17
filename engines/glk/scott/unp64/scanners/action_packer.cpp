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

#include "common/endian.h"
#include "glk/scott/types.h"
#include "glk/scott/unp64/unp64.h"
#include "glk/scott/unp64/exo_util.h"

namespace Glk {
namespace Scott {

void scnActionPacker(UnpStr *unp) {
	byte *mem;

	if (unp->_idFlag)
		return;
	mem = unp->_mem;
	if (unp->_depAdr == 0) {
		if (u32eq(mem + 0x811, 0x018538A9) &&
			u32eq(mem + 0x81d, 0xCEF7D0E8) &&
			u32eq(mem + 0x82d, 0x0F9D0837) &&
			u32eq(mem + 0x84b, 0x03D00120)) {
			unp->_depAdr = 0x110;
			unp->_forced = 0x811;
			unp->_strMem = READ_LE_UINT16(&mem[0x848]);
			unp->_fEndAf = 0x120;
			unp->_retAdr = READ_LE_UINT16(&mem[0x863]);
			unp->_idFlag = 1;
			return;
		}
	}
}

} // End of namespace Scott
} // End of namespace Glk
