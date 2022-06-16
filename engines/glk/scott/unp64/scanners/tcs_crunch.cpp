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

namespace Glk {
namespace Scott {

void scnTCScrunch(UnpStr *unp) {
	byte *mem;
	int q, p;
	if (unp->_idFlag)
		return;
	mem = unp->_mem;
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x819) == 0x018536A9) && mem[0x81d] == 0x4c) {
			p = READ_LE_INT16(&mem[0x81e]); // mem[0x81e] | mem[0x81f] << 8;
			if (mem[p] == 0xa2 && mem[p + 2] == 0xbd &&
				(*(unsigned int *)(mem + p + 0x05) == 0xE801109D) &&
				((*(unsigned int *)(mem + p + 0x38) == 0x01524CFB) ||
				((*(unsigned int *)(mem + p + 0x38) == 0x8DE1A9FB) &&
				(*(unsigned int *)(mem + p + 0x3c) == 0x524C0328)))) {
				unp->_depAdr = 0x334;
				unp->_forced = 0x819;
				unp->_endAdr = 0x2d;
			}
		}
		else if ((*(unsigned int *)(mem + 0x819) == 0x018534A9) && mem[0x81d] == 0x4c) {
			p = READ_LE_INT16(&mem[0x81e]); // mem[0x81e] | mem[0x81f] << 8;
			if (mem[p] == 0xa2 && mem[p + 2] == 0xbd &&
				(*(unsigned int *)(mem + p + 0x05) == 0xE801109D) &&
				(*(unsigned int *)(mem + p + 0x38) == 0x01304CFB)) {
				unp->_depAdr = 0x334;
				unp->_forced = 0x818;
				if (mem[unp->_forced] != 0x78)
					unp->_forced++;
				unp->_endAdr = 0x2d;
				unp->_retAdr = READ_LE_INT16(&mem[p + 0xd9]); // mem[p + 0xd9] | mem[p + 0xda] << 8;
				p += 0xc8;
				q = p + 6;
				for (; p < q; p += 3) {
					if ((mem[p] == 0x20) &&
						(*(unsigned short int *)(mem + p + 1) >= 0xa000) &&
						(*(unsigned short int *)(mem + p + 1) <= 0xbfff)) {
						mem[p] = 0x2c;
					}
				}
			}
		}
		if (unp->_depAdr) {
			unp->_idFlag = 1;
			return;
		}
	}
}

} // End of namespace Scott
} // End of namespace Glk
