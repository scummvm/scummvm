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

void scnCCS(UnpStr *unp) {
	byte *mem;
	int p;
	if (unp->_idFlag)
		return;
	mem = unp->_mem;
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x817) == 0xB901E678) &&
			(*(unsigned int *)(mem + 0x81b) == 0xFD990831) &&
			(*(unsigned int *)(mem + 0x8ff) == 0xFEE60290) &&
			(*(unsigned int *)(mem + 0x90f) == 0x02903985)) {
			if (unp->_info->_run == -1)
				unp->_forced = 0x817;
			unp->_depAdr = 0x0ff;
			unp->_fEndAf = 0x2d;
			unp->_endAdC = 0xffff;
			unp->_retAdr = READ_LE_INT16(&mem[0x8ed]);
			if (unp->_retAdr == 0xa659) {
				mem[0x8ec] = 0x2c;
				unp->_retAdr = READ_LE_INT16(&mem[0x8f0]);
			}
			unp->_idFlag = 1;
			return;
		}
	}
	/* derived from supercomp/eqseq */
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x80b) == 0x8C7800A0) &&
			(*(unsigned int *)(mem + 0x812) == 0x0099082F) &&
			(*(unsigned int *)(mem + 0x846) == 0x0DADF2D0) &&
			(*(unsigned int *)(mem + 0x8c0) == 0xF001124C)) {
			if (unp->_info->_run == -1)
				unp->_forced = 0x80b;
			unp->_depAdr = 0x100;
			unp->_endAdr = 0xae;
			unp->_retAdr = READ_LE_INT16(&mem[0x8f1]);
			if (unp->_retAdr == 0xa659) {
				mem[0x8f0] = 0x2c;
				unp->_retAdr = READ_LE_INT16(&mem[0x8f4]);
			}
			unp->_idFlag = 1;
			return;
		}
	}
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x814) == 0xB901E678) &&
			(*(unsigned int *)(mem + 0x818) == 0xFD990829) &&
			(*(unsigned int *)(mem + 0x8a1) == 0xFDA6FDB1) &&
			(*(unsigned int *)(mem + 0x8a5) == 0xFEC602D0)) {
			if (unp->_info->_run == -1)
				unp->_forced = 0x814;
			unp->_depAdr = 0x0ff;
			unp->_fEndBf = 0x39;
			unp->_idFlag = 1;
			return;
		}
	}
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x818) == 0x2CB901E6) &&
			(*(unsigned int *)(mem + 0x81c) == 0x00FB9908) &&
			(*(unsigned int *)(mem + 0x850) == 0xFBB1C84A) &&
			(*(unsigned int *)(mem + 0x854) == 0xB1C81185)) {
			if (unp->_info->_run == -1)
				unp->_forced = 0x812;
			unp->_depAdr = 0x0ff;
			unp->_endAdr = 0xae;
			unp->_idFlag = 1;
			return;
		}
	}
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x818) == 0x2CB901E6) &&
			(*(unsigned int *)(mem + 0x81c) == 0x00FB9908) &&
			(*(unsigned int *)(mem + 0x851) == 0xFBB1C812) &&
			(*(unsigned int *)(mem + 0x855) == 0xB1C81185)) {
			if (unp->_info->_run == -1)
				unp->_forced = 0x812;
			unp->_depAdr = 0x0ff;
			unp->_endAdr = 0xae;
			unp->_idFlag = 1;
			return;
		}
	}
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x82c) == 0x018538A9) &&
			(*(unsigned int *)(mem + 0x831) == 0xFD990842) &&
			(*(unsigned int *)(mem + 0x83e) == 0x00FF4CF1) &&
			(*(unsigned int *)(mem + 0x8a5) == 0x50C651C6)) {
			if (unp->_info->_run == -1)
				unp->_forced = 0x822;
			unp->_depAdr = 0x0ff;
			unp->_fEndBf = 0x39;
			unp->_retAdr = READ_LE_INT16(&mem[0x8ea]);
			unp->_idFlag = 1;
			return;
		}
	}
	if (unp->_depAdr == 0) {
		if ((*(unsigned short int *)(mem + 0x81a) == 0x00A0) &&
			((*(unsigned int *)(mem + 0x820) == 0xFB990837) ||
			 (*(unsigned int *)(mem + 0x824) == 0xFB990837)) &&
			(*(unsigned int *)(mem + 0x83b) == 0xFD91FBB1) &&
			(*(unsigned int *)(mem + 0x8bc) == 0xEE00FC99)) {
			if (unp->_info->_run == -1)
				unp->_forced = 0x81a;
			unp->_depAdr = 0x0ff;
			unp->_fEndAf = 0x39;
			unp->_endAdC = 0xffff;
			unp->_retAdr = READ_LE_INT16(&mem[0x8b3]);
			unp->_idFlag = 1;
			return;
		}
	}
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x812) == 0xE67800A0) &&
			(*(unsigned int *)(mem + 0x816) == 0x0823B901) &&
			(*(unsigned int *)(mem + 0x81a) == 0xC800FD99) &&
			(*(unsigned int *)(mem + 0x81e) == 0xFF4CF7D0) &&
			(*(unsigned int *)(mem + 0x885) == 0xFDA6FDB1)) {
			if (unp->_info->_run == -1)
				unp->_forced = 0x812;
			unp->_depAdr = 0x0ff;
			// $2d is unreliable, Executer uses line number at $0803/4,
			// which is read at $0039/3a by basic, as end address,
			// then can set arbitrarily $2d/$ae pointers after unpack.
			// unp->_fEndAf=0x2d;
			unp->_endAdr = READ_LE_INT16(&mem[0x803]);
			unp->_endAdr++;
			if (*(unsigned int *)(mem + 0x87f) == 0x4CA65920)
				mem[0x87f] = 0x2c;
			unp->_retAdr = READ_LE_INT16(&mem[0x883]);
			unp->_idFlag = 1;
			return;
		}
	}
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x812) == 0xE67800A0) &&
			(*(unsigned int *)(mem + 0x816) == 0x084CB901) &&
			(*(unsigned int *)(mem + 0x81a) == 0xA900FB99) &&
			(*(unsigned int *)(mem + 0x848) == 0x00FF4CE2)) {
			if (unp->_info->_run == -1)
				unp->_forced = 0x812;
			unp->_depAdr = 0x0ff;
			unp->_fEndAf = 0x2d;
			unp->_endAdC = 0xffff;
			unp->_idFlag = 1;
			return;
		}
	}
	/* Triad Hack */
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x838) == 0xB9080099) &&
			(*(unsigned int *)(mem + 0x83f) == 0xD0880816) &&
			(*(unsigned int *)(mem + 0x8ff) == 0xFEE60290) &&
			(*(unsigned int *)(mem + 0x90f) == 0x02903985)) {
			if (unp->_info->_run == -1) {
				for (p = 0x80b; p < 0x820; p++) {
					if ((mem[p] & 0xa0) == 0xa0) {
						unp->_forced = p;
						break;
					}
				}
			}
			unp->_depAdr = 0x0ff;
			unp->_fEndAf = 0x2d;
			unp->_endAdC = 0xffff;
			unp->_retAdr = READ_LE_INT16(&mem[0x8ed]);
			if (unp->_retAdr == 0xa659) {
				mem[0x8ec] = 0x2c;
				unp->_retAdr = READ_LE_INT16(&mem[0x8f0]);
			}
			unp->_idFlag = 1;
			return;
		}
	}
}

} // End of namespace Scott
} // End of namespace Glk
