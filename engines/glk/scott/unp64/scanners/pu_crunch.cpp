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

void scnPuCrunch(UnpStr *unp) {
	byte *mem;
	int q, p;
	if (unp->_idFlag)
		return;
	mem = unp->_mem;
	if (unp->_depAdr == 0) {
		if ((mem[0x80d] == 0x78) &&
			(*(unsigned int *)(mem + 0x813) == 0x34A20185) &&
			(*(unsigned int *)(mem + 0x817) == 0x9D0842BD) &&
			(*(unsigned int *)(mem + 0x81b) == 0xD0CA01FF) &&
			(*(unsigned int *)(mem + 0x83d) == 0x4CEDD088)) {
			for (p = 0x912; p < 0x938; p++) {
				if ((*(unsigned int *)(mem + p) == 0x2D85FAA5) &&
					(*(unsigned int *)(mem + p + 4) == 0x2E85FBA5)) {
					unp->_endAdr = 0xfa;
					unp->_strMem = READ_LE_INT16(&mem[0x879]); // mem[0x879] | mem[0x87a] << 8;
					unp->_depAdr = READ_LE_INT16(&mem[0x841]); // mem[0x841] | mem[0x842] << 8;
					unp->_retAdr = READ_LE_INT16(&mem[p + 0xa]); // mem[p + 0xa] | mem[p + 0xb] << 8;
					unp->_forced = 0x80d;
					break;
				}
			}
		} else if ((mem[0x80d] == 0x78) &&
				   (*(unsigned int *)(mem + 0x81a) == 0x10CA4B95) &&
				   (*(unsigned int *)(mem + 0x81e) == 0xBD3BA2F8) &&
				   (*(unsigned int *)(mem + 0x847) == 0x4CEDD088)) {
			for (p = 0x912; p < 0x938; p++) {
				if ((*(unsigned int *)(mem + p) == 0x2D85FAA5) &&
					(*(unsigned int *)(mem + p + 4) == 0x2E85FBA5)) {
					unp->_endAdr = 0xfa;
					unp->_strMem = READ_LE_INT16(&mem[p + 0x88a]); // mem[0x88a] | mem[0x88b] << 8;
					unp->_depAdr = READ_LE_INT16(&mem[p + 0x84b]); // mem[0x84b] | mem[0x84c] << 8;
					unp->_retAdr = READ_LE_INT16(&mem[p + 0xa]); // mem[p + 0xa] | mem[p + 0xb] << 8;
					unp->_forced = 0x80d;
					break;
				}
			}
		} else if ((mem[0x80d] == 0x78) &&
				   (*(unsigned int *)(mem + 0x811) == 0x85AAA901) &&
				   (*(unsigned int *)(mem + 0x81d) == 0xF69D083C) &&
				   (*(unsigned int *)(mem + 0x861) == 0xC501C320) &&
				   (*(unsigned int *)(mem + 0x839) == 0x01164CED)) {
			unp->_endAdr = 0xfa;
			unp->_strMem = READ_LE_INT16(&mem[0x840]); // mem[0x840] | mem[0x841] << 8;
			unp->_depAdr = 0x116;
			unp->_retAdr = READ_LE_INT16(&mem[0x8df]); // mem[0x8df] | mem[0x8e0] << 8;
			unp->_forced = 0x80d;
		} else if ((mem[0x80d] == 0x78) &&
				   (*(unsigned int *)(mem + 0x811) == 0x85AAA901) &&
				   (*(unsigned int *)(mem + 0x81d) == 0xF69D083C) &&
				   (*(unsigned int *)(mem + 0x861) == 0xC501C820) &&
				   (*(unsigned int *)(mem + 0x839) == 0x01164CED)) {
			unp->_endAdr = 0xfa;
			unp->_strMem = READ_LE_INT16(&mem[0x840]); // mem[0x840] | mem[0x841] << 8;
			unp->_depAdr = 0x116;
			if (mem[0x8de] == 0xa9) {
				unp->_retAdr = READ_LE_INT16(&mem[0x8e1]); // mem[0x8e1] | mem[0x8e2] << 8;
				if ((unp->_retAdr == 0xa871) && (mem[0x8e0] == 0x20) &&
					(mem[0x8e3] == 0x4c)) {
					mem[0x8e0] = 0x2c;
					unp->_retAdr = READ_LE_INT16(&mem[0x8e4]); // mem[0x8e4] | mem[0x8e5] << 8;
				}
			} else {
				unp->_retAdr = READ_LE_INT16(&mem[0x8df]); // mem[0x8df] | mem[0x8e0] << 8;
			}
			unp->_forced = 0x80d;
		} else {
			/* unknown old/hacked pucrunch ? */
			for (p = 0x80d; p < 0x820; p++) {
				if (mem[p] == 0x78) {
					q = p;
					for (; p < 0x824; p++) {
						if (((*(unsigned int *)(mem + p) & 0xf0ffffff) == 0xF0BD53A2) &&
							(*(unsigned int *)(mem + p + 4) == 0x01FF9D08) &&
							(*(unsigned int *)(mem + p + 8) == 0xA2F7D0CA)) {
							unp->_forced = q;
							q = mem[p + 3] & 0xf; /* can be $f0 or $f2, q&0x0f as offset */
							p = READ_LE_INT16(&mem[p + 0xe]); // mem[p + 0xe] | mem[p + 0xf] << 8;
							if (mem[p - 2] == 0x4c && mem[p + 0xa0 + q] == 0x85) {
								unp->_depAdr = READ_LE_INT16(&mem[p - 1]); // mem[p - 1] | mem[p] << 8;
								unp->_strMem = READ_LE_INT16(&mem[p + 4]); // mem[p + 4] | mem[p + 5] << 8;
								unp->_endAdr = 0xfa;
								p += 0xa2;
								q = p + 8;
								for (; p < q; p++) {
									if ((*(unsigned int *)(mem + p) == 0x2D85FAA5) &&
										(mem[p + 9] == 0x4c)) {
										unp->_retAdr = READ_LE_INT16(&mem[p + 0xa]); // mem[p + 0xa] | mem[p + 0xb] << 8;
										break;
									}
								}
							}
						}
					}
				}
			}
		}

		/* various old/hacked pucrunch */
		/* common pattern, variable pos from 0x79 to 0xd1
		90 ?? C8 20 ?? 0? 85 ?? C9 ?0 90 0B A2 0? 20 ?? 0? 85 ?? 20 ?? 0? A8 20 ??
		0? AA BD ?? 0? E0 20 90 0? 8A (A2 03) not always 20 ?? 02 A6
		?? E8 20 F9
		*/
		if (unp->_depAdr == 0) {
			unp->_idFlag = 0;
			for (q = 0x70; q < 0xff; q++) {
				if (((*(unsigned int *)(mem + 0x801 + q) & 0xFFFF00FF) == 0x20C80090) &&
					((*(unsigned int *)(mem + 0x801 + q + 8) & 0xFFFF0FFF) ==
					 0x0B9000C9) &&
					((*(unsigned int *)(mem + 0x801 + q + 12) & 0x00FFF0FF) ==
					 0x002000A2) &&
					((*(unsigned int *)(mem + 0x801 + q + 30) & 0xF0FFFFFf) ==
					 0x009020E0)) {
					unp->_idFlag = 385;
					break;
				}
			}
			if (unp->_idFlag) {
				for (p = 0x801 + q + 34; p < 0x9ff; p++) {
					if (*(unsigned int *)(mem + p) == 0x00F920E8) {
						for (; p < 0x9ff; p++) {
							if (mem[p] == 0x4c) {
								unp->_retAdr = mem[p + 1] | mem[p + 2] << 8;
								if (unp->_retAdr > 0x257)
									break;
							}
						}
						break;
					}
				}
				for (p = 0; p < 0x40; p++) {
					if (unp->_info->_run == -1)
						if (unp->_forced == 0) {
							if (mem[0x801 + p] == 0x78) {
								unp->_forced = 0x801 + p;
								unp->_info->_run = unp->_forced;
							}
						}
					if ((*(unsigned int *)(mem + 0x801 + p) == 0xCA00F69D) &&
						(mem[0x801 + p + 0x1b] == 0x4c)) {
						q = 0x801 + p + 0x1c;
						unp->_depAdr = mem[q] | mem[q + 1] << 8;
						q = 0x801 + p - 2;
						p = mem[q] | mem[q + 1] << 8;
						if ((mem[p + 3] == 0x8d) && (mem[p + 6] == 0xe6)) {
							unp->_strMem = mem[p + 4] | mem[p + 5] << 8;
						}
						break;
					}
				}
				unp->_endAdr = 0xfa; // some hacks DON'T xfer fa/b to 2d/e
				unp->_idFlag = 1;
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
