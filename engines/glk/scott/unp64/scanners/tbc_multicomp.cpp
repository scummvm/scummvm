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

void scnTBCMultiComp(UnpStr *unp) {
	byte *mem;
	int p = 0, q = 0, strtmp;
	if (unp->_idFlag)
		return;
	mem = unp->_mem;
	if (unp->_depAdr == 0) {
		if (((*(unsigned int *)(mem + 0x82c) & 0xfffffffd) == 0x9ACA0184) &&
			 (*(unsigned int *)(mem + 0x830) == 0xA001004C) &&
			 (*(unsigned int *)(mem + 0x834) == 0x84FD8400) &&
			 (*(unsigned int *)(mem + 0x8a2) == 0x01494C01)) {
			/*normal 2080*/
			if (mem[0x84a] == 0x81) {
				if (*(unsigned int *)(mem + 0x820) == 0x32BDE9A2) {
					unp->_forced = 0x820;
					unp->_retAdr = READ_LE_UINT16(&mem[0x8b2]); // mem[0x8b2] | mem[0x8b3] << 8;
					if (unp->_retAdr == 0x1e1) {
						if (*(unsigned int *)(mem + 0x916) == 0x4CA87120) {
							p = *(unsigned short int *)(mem + 0x91a);
							if (p == 0xa7ae) {
								unp->_retAdr = p;
								mem[0x8b2] = 0xae;
								mem[0x8b3] = 0xa7;
							} else {
								mem[0x916] = 0x2c;
								unp->_retAdr = p;
							}
						} else if ((mem[0x916] == 0x4C) || (mem[0x916] == 0x20)) {
							unp->_retAdr = READ_LE_UINT16(&mem[0x917]); // mem[0x917] | mem[0x918] << 8;
						} else if (mem[0x919] == 0x4c) {
							unp->_retAdr = READ_LE_UINT16(&mem[0x91a]); // mem[0x91a] | mem[0x91b] << 8;
						}
					}
					if ((unp->_retAdr == 0) && (mem[0x8b1] == 0)) {
						unp->_retAdr = 0xa7ae;
						mem[0x8b1] = 0x4c;
						mem[0x8b2] = 0xae;
						mem[0x8b3] = 0xa7;
					}
					p = 0x8eb;
				}
			}
			/*firelord 2076*/
			else if (mem[0x84a] == 0x7b) {
				if (*(unsigned int *)(mem + 0x81d) == 0x32BDE9A2) {
					unp->_forced = 0x81d;
					unp->_retAdr = READ_LE_UINT16(&mem[0x8ac]); // mem[0x8ac] | mem[0x8ad] << 8;
					p = 0x8eb;
				}
			}
			if (unp->_forced) {
				unp->_depAdr = 0x100;
				unp->_strMem = READ_LE_UINT16(&mem[p + 1]); // mem[p + 1] | mem[p + 2] << 8;
				q = p;
				q += mem[p];
				unp->_endAdr = 0;
				for (; q > p; q -= 4) {
					strtmp = READ_LE_UINT16(&mem[q - 1]); //(mem[q - 1] | mem[q] << 8);
					if (strtmp == 0)
						strtmp = 0x10000;
					if (strtmp > unp->_endAdr)
						unp->_endAdr = strtmp;
				}
				unp->_idFlag = 1;
				return;
			}
		}
	}
	/* TBC Multicompactor ?  very similar but larger code */
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x822) == 0x9D083DBD) &&
			(*(unsigned int *)(mem + 0x826) == 0xD0CA0333) &&
			(*(unsigned int *)(mem + 0x832) == 0xF7D0CA00) &&
			(*(unsigned int *)(mem + 0x836) == 0xCA018678) &&
			(*(unsigned int *)(mem + 0x946) == 0xADC5AFA5)) {
			if (unp->_info->_run == -1) {
				for (p = 0x81e; p < 0x821; p++) {
					if (mem[p] == 0xa2) {
						unp->_forced = p;
						break;
					}
				}
			}
			unp->_depAdr = 0x334;
			unp->_retAdr = READ_LE_UINT16(&mem[0x92a]); // mem[0x92a] | mem[0x92b] << 8;
			p = 0x94d;
			unp->_strMem = READ_LE_UINT16(&mem[p + 1]); // mem[p + 1] | mem[p + 2] << 8;
			q = p;
			q += mem[p];
			unp->_endAdr = 0;
			for (; q > p; q -= 4) {
				strtmp = READ_LE_UINT16(&mem[q - 1]); //(mem[q - 1] | mem[q] << 8);
				if (strtmp == 0)
					strtmp = 0x10000;
				if (strtmp > unp->_endAdr)
					unp->_endAdr = strtmp;
			}
			unp->_idFlag = 1;
			return;
		}
	}
	/*"AUTOMATIC BREAK SYSTEM" found in Manowar Cracks*/
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x835) == 0x9D0845BD) &&
			(*(unsigned int *)(mem + 0x839) == 0xD0CA00ff) &&
			(*(unsigned int *)(mem + 0x83e) == 0xCA018678) &&
			(*(unsigned int *)(mem + 0x8e1) == 0xADC5AFA5)) {
			if (unp->_info->_run == -1) {
				for (p = 0x830; p < 0x834; p++) {
					if (mem[p] == 0xa2) {
						unp->_forced = p;
						break;
					}
				}
			}
			unp->_depAdr = 0x100;
			unp->_retAdr = READ_LE_UINT16(&mem[0x8c5]); // mem[0x8c5] | mem[0x8c6] << 8;
			p = 0x8fe;
			unp->_strMem = READ_LE_UINT16(&mem[p + 1]); // mem[p + 1] | mem[p + 2] << 8;
			q = p;
			q += mem[p];
			unp->_endAdr = 0;
			for (; q > p; q -= 4) {
				strtmp = READ_LE_UINT16(&mem[q - 1]); //(mem[q - 1] | mem[q] << 8);
				if (strtmp == 0)
					strtmp = 0x10000;
				if (strtmp > unp->_endAdr)
					unp->_endAdr = strtmp;
			}
			unp->_idFlag = 1;
			return;
		}
	}
}

} // End of namespace Scott
} // End of namespace Glk
