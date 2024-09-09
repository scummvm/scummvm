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

void scnMegabyte(UnpStr *unp) {
	byte *mem;
	int p;
	if (unp->_idFlag)
		return;
	mem = unp->_mem;
	if (unp->_depAdr == 0) {
		p = 0;
		if (mem[0x816] == 0x4c)
			p = READ_LE_UINT16(&mem[0x817]); // mem[0x817] | mem[0x818] << 8;
		else if (unp->_info->_run == 0x810 && mem[0x814] == 0x4c &&
				 u32eqmasked(mem + 0x810, 0xffff00ff, 0x018500A9))
			p = READ_LE_UINT16(&mem[0x815]); // mem[0x815] | mem[0x816] << 8;
		if (p) {
			if (mem[p + 0] == 0x78 && mem[p + 1] == 0xa2 &&
				mem[p + 3] == 0xa0 &&
				u32eq(mem + p + 0x05, 0x15841486) &&
				u32eq(mem + p + 0x1d, 0x03804CF7)) {
				unp->_depAdr = 0x380;
				unp->_endAdr = READ_LE_UINT16(&mem[p + 0x55]); // mem[p + 0x55] | mem[p + 0x56] << 8;
				unp->_endAdr++;
				unp->_strMem = 0x801;
				unp->_retAdr = 0x801; /* usually it just runs */
				unp->_idFlag = 1;
				return;
			}
		}
	}
	if (unp->_depAdr == 0) {
		p = 0;
		if (mem[0x81a] == 0x4c &&
			u32eqmasked(mem + 0x816, 0xffff00ff, 0x018500A9))
			p = READ_LE_UINT16(&mem[0x81b]); // mem[0x81b] | mem[0x81c] << 8;
		if (p) {
			if (mem[p + 0] == 0x78 && mem[p + 1] == 0xa2 &&
				mem[p + 3] == 0xa0 &&
				u32eq(mem + p + 0x05, 0x15841486) &&
				u32eq(mem + p + 0x1d, 0x03844CF7)) {
				unp->_depAdr = 0x384;
				unp->_forced = 0x816;
				unp->_endAdr = READ_LE_UINT16(&mem[p + 0x59]); // mem[p + 0x59] | mem[p + 0x5a] << 8;
				unp->_endAdr++;
				unp->_strMem = 0x801;
				unp->_retAdr = 0x801; /* usually it just runs */
				unp->_idFlag = 1;
				return;
			}
		}
	}
}

} // End of namespace Scott
} // End of namespace Glk
