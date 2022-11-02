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

void scnExomizer(UnpStr *unp) {
	byte *mem;
	int q, p;
	if (unp->_idFlag)
		return;
	mem = unp->_mem;
	/* exomizer 3.x */
	if (unp->_depAdr == 0) {
		for (p = unp->_info->_end; p > unp->_info->_start; p--) {
			if ((*(unsigned int *)(mem + p) == 0x100A8069) &&
				(*(unsigned int *)(mem + p + 4) == 0xD0FD060F) &&
				mem[p - 6] == 0x4c && mem[p - 4] == 0x01) {
				p -= 5;
				q = 2;
				if (mem[p - q] == 0x8a)
					q++;

				/* low byte of EndAdr, it's a lda $ff00,y */

				if ((mem[p - q - 1] == mem[p - q - 3]) &&
					(mem[p - q - 2] == mem[p - q])) { /* a0 xx a0 xx -> exomizer 3.0/3.01 */
					unp->_exoFnd = 0x30;
				} else { /* d0 c1 a0 xx -> exomizer 3.0.2, force +1 in start/end */
					unp->_exoFnd = 0x32;
				}
				unp->_exoFnd |= (mem[p - q] << 8);
				break;
			}
		}
		if (unp->_exoFnd) {
			unp->_depAdr = 0x100 | mem[p];
			for (; p < unp->_info->_end; p++) {
				if (*(unsigned int *)(mem + p) == 0x7d010020)
					break;
			}
			for (; p < unp->_info->_end; p++) {
				if (mem[p] == 0x4c) {
					unp->_retAdr = 0;
					if ((unp->_retAdr = READ_LE_UINT16(&mem[p + 1])) >= 0x200) {
						break;
					} else { /* it's a jmp $01xx, goto next */
						p++;
						p++;
					}
				}
			}
			if (unp->_info->_run == -1) {
				p = unp->_info->_start;
				q = p + 0x10;
				for (; p < q; p++) {
					if ((mem[p] == 0xba) && (mem[p + 1] == 0xbd)) {
						unp->_forced = p;
						break;
					}
				}
				for (q = p - 1; q >= unp->_info->_start; q--) {
					if (mem[q] == 0xe6)
						unp->_forced = q;
					if (mem[q] == 0xa0)
						unp->_forced = q;
					if (mem[q] == 0x78)
						unp->_forced = q;
				}
			}
		}
	}
	/* exomizer 1.x/2.x */
	if (unp->_depAdr == 0) {
		for (p = unp->_info->_end; p > unp->_info->_start; p--) {
			if ((((*(unsigned int *)(mem + p) == 0x4CF7D088) &&
				  (*(unsigned int *)(mem + p - 0x0d) == 0xD034C0C8)) ||
				 ((*(unsigned int *)(mem + p) == 0x4CA7A438) &&
				  (*(unsigned int *)(mem + p - 0x0c) == 0x799FA5AE)) ||
				 ((*(unsigned int *)(mem + p) == 0x4CECD08A) &&
				  (*(unsigned int *)(mem + p - 0x13) == 0xCECA0EB0)) ||
				 ((*(unsigned int *)(mem + p) == 0x4C00A0D3) &&
				  (*(unsigned int *)(mem + p - 0x04) == 0xD034C0C8)) ||
				 ((*(unsigned int *)(mem + p) == 0x4C00A0D2) &&
				  (*(unsigned int *)(mem + p - 0x04) == 0xD034C0C8))) &&
				mem[p + 5] == 1) {
				p += 4;
				unp->_exoFnd = 1;
				break;
			} else if ((((*(unsigned int *)(mem + p) == 0x8C00A0d2) &&
						 (*(unsigned int *)(mem + p - 0x04) == 0xD034C0C8)) ||
						((*(unsigned int *)(mem + p) == 0x8C00A0d3) &&
						 (*(unsigned int *)(mem + p - 0x04) == 0xD034C0C8)) ||
						((*(unsigned int *)(mem + p) == 0x8C00A0cf) &&
						 (*(unsigned int *)(mem + p - 0x04) == 0xD034C0C8))) &&
					   mem[p + 6] == 0x4c && mem[p + 8] == 1) {
				p += 7;
				unp->_exoFnd = 1;
				break;
			}
		}
		if (unp->_exoFnd) {
			unp->_depAdr = 0x100 | mem[p];
			if (unp->_depAdr >= 0x134 && unp->_depAdr <= 0x14a /*0x13e*/) {
				for (p = unp->_info->_end - 4; p > unp->_info->_start;
					 p--) { /* 02 04 04 30 20 10 80 00 */
					if (*(unsigned int *)(mem + p) == 0x30040402)
						break;
				}
			} else {
				// exception for exo v1.x, otherwise add 8 to the counter and
				// scan backward from here
				if (unp->_depAdr != 0x143)
					p += 0x08;
				else
					p -= 0xb8;
			}
			for (; p > unp->_info->_start; p--) {
				// incredibly there can be a program starting at $4c00 :P
				if ((mem[p] == 0x4c) && (mem[p - 1] != 0x4c) && (mem[p - 2] != 0x4c)) {
					unp->_retAdr = 0;
					if ((unp->_retAdr = READ_LE_UINT16(&mem[p + 1])) >= 0x200) {
						break;
					}
				}
			}
			if (unp->_info->_run == -1) {
				p = unp->_info->_start;
				q = p + 0x10;
				for (; p < q; p++) {
					if ((mem[p] == 0xba) && (mem[p + 1] == 0xbd)) {
						unp->_forced = p;
						break;
					}
				}
				for (q = p - 1; q >= unp->_info->_start; q--) {
					if (mem[q] == 0xe6)
						unp->_forced = q;
					if (mem[q] == 0xa0)
						unp->_forced = q;
					if (mem[q] == 0x78)
						unp->_forced = q;
				}
			}
		}
	}
	if (unp->_depAdr != 0) {
		unp->_idFlag = 1;
		return;
	}
}

} // End of namespace Scott
} // End of namespace Glk
