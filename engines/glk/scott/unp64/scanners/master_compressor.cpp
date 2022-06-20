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

void scnMasterCompressor(UnpStr *unp) {
	byte *mem;
	int p;
	if (unp->_idFlag)
		return;
	mem = unp->_mem;
	if (unp->_depAdr == 0) {
		for (p = 0x80d; p < 0x880; p++) {
			if (((*(unsigned int *)(mem + p + 0x005) & 0x00ffffff) == 0x00BDD2A2) &&
				 (*(unsigned int *)(mem + p + 0x00a) == 0xE000F99D) &&
				 (*(unsigned int *)(mem + p + 0x017) == 0xCAEDD0CA) &&
				 (*(unsigned int *)(mem + p + 0x031) == 0x84C82E86) &&
				 ((*(unsigned int *)(mem + p + 0x035) & 0x0000ffff) == 0x00004C2D) &&
				 (*(unsigned int *)(mem + p + 0x134) == 0xDBD0FFE6)) {
				if (/*mem[p]==0x78&&*/ mem[p + 1] == 0xa9 &&
					(*(unsigned int *)(mem + p + 0x003) == 0xD2A20185)) {
					unp->_depAdr = READ_LE_UINT16(&mem[p + 0x37]); // mem[p + 0x37] | mem[p + 0x38] << 8;
					unp->_forced = p + 1;
					if (mem[p + 0x12b] == 0x020) // jsr $0400, unuseful fx
						mem[p + 0x12b] = 0x2c;
				} else if (*(unsigned int *)(mem + p) == 0xD024E0E8) {
					/* HTL version */
					unp->_depAdr = READ_LE_UINT16(&mem[p + 0x37]); // mem[p + 0x37] | mem[p + 0x38] << 8;
					unp->_forced = 0x840;
				}
				if (unp->_depAdr) {
					unp->_retAdr = READ_LE_UINT16(&mem[p + 0x13e]); // mem[p + 0x13e] | mem[p + 0x13f] << 8;
					unp->_endAdr = 0x2d;
					unp->_fStrBf = unp->_endAdr;
					unp->_idFlag = 1;
					return;
				}
			}
		}
	}
	if (unp->_depAdr == 0) {
		for (p = 0x80d; p < 0x880; p++) {
			if (((*(unsigned int *)(mem + p + 0x005) & 0x00ffffff) == 0x00BDD2A2) &&
				 (*(unsigned int *)(mem + p + 0x00a) == 0xE000F99D) &&
				 (*(unsigned int *)(mem + p + 0x017) == 0xCAEDD0CA) &&
				 (*(unsigned int *)(mem + p + 0x031) == 0x84C82E86) &&
				 ((*(unsigned int *)(mem + p + 0x035) & 0x0000ffff) == 0x00004C2D) &&
				 (*(unsigned int *)(mem + p + 0x12d) == 0xe2D0FFE6)) {
				if (mem[p + 1] == 0xa9 &&
					(*(unsigned int *)(mem + p + 0x003) == 0xD2A20185)) {
					unp->_depAdr = READ_LE_UINT16(&mem[p + 0x37]); // mem[p + 0x37] | mem[p + 0x38] << 8;
					unp->_forced = p + 1;
				}
				if (unp->_depAdr) {
					if (mem[p + 0x136] == 0x4c)
						unp->_retAdr = READ_LE_UINT16(&mem[p + 0x137]); // mem[p + 0x137] | mem[p + 0x138] << 8;
					else if (mem[p + 0x13d] == 0x4c)
						unp->_retAdr = READ_LE_UINT16(&mem[p + 0x13e]); // mem[p + 0x13e] | mem[p + 0x13f] << 8;
					unp->_endAdr = 0x2d;
					unp->_fStrBf = unp->_endAdr;
					unp->_idFlag = 1;
					return;
				}
			}
		}
	}
	if (unp->_depAdr == 0) {
		p = 0x812;
		if ((*(unsigned int *)(mem + p + 0x000) == 0xE67800A0) &&
			(*(unsigned int *)(mem + p + 0x004) == 0x0841B901) &&
			(*(unsigned int *)(mem + p + 0x008) == 0xB900FA99) &&
			(*(unsigned int *)(mem + p + 0x00c) == 0x34990910)) {
			unp->_depAdr = 0x100;
			unp->_forced = p;
			unp->_retAdr = READ_LE_UINT16(&mem[0x943]); // mem[0x943] | mem[0x944] << 8;
			unp->_endAdr = 0x2d;
			unp->_fStrBf = unp->_endAdr;
			unp->_idFlag = 1;
			return;
		}
	}
	/* Fred/Channel4 hack */
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x811) == 0xA9A98078) &&
			(*(unsigned int *)(mem + 0x815) == 0x85EE8034) &&
			(*(unsigned int *)(mem + 0x819) == 0x802DA201) &&
			(*(unsigned int *)(mem + 0x882) == 0x01004C2D)) {
			unp->_depAdr = 0x100;
			unp->_forced = 0x811;
			unp->_retAdr = READ_LE_UINT16(&mem[0x98b]); // mem[0x98b] | mem[0x98c] << 8;
			if (unp->_retAdr < 0x800)
				unp->_rtAFrc = 1;
			unp->_endAdr = 0x2d;
			unp->_idFlag = 1;
			return;
		}
	}
}

} // End of namespace Scott
} // End of namespace Glk
