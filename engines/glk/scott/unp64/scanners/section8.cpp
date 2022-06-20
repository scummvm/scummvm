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

void scnSection8(UnpStr *unp) {
	byte *mem;
	int p;
	if (unp->_idFlag)
		return;
	mem =unp->_mem;
	if (unp->_depAdr == 0) {
		for (p = 0x810; p <= 0x828; p++) {
			if ((*(unsigned int *)(mem + p) == (0x00BD00A2 + (((p & 0xff) + 0x11) << 24))) &&
				(*(unsigned int *)(mem + p + 0x04) == 0x01009D08) &&
				(*(unsigned int *)(mem + p + 0x10) == 0x34A97801) &&
				(*(unsigned int *)(mem + p + 0x6a) == 0xB1017820) &&
				(*(unsigned int *)(mem + p + 0x78) == 0x017F20AE)) {
				unp->_depAdr = 0x100;
				break;
			}
		}
		if (unp->_depAdr) {
			if (unp->_info->_run == -1)
				unp->_forced = p;
			unp->_strMem = READ_LE_UINT16(&mem[p + 0x47]); // mem[p + 0x47] | mem[p + 0x4b] << 8;
			unp->_retAdr = READ_LE_UINT16(&mem[p + 0x87]); // mem[p + 0x87] | mem[p + 0x88] << 8;
			if (unp->_retAdr == 0xf7) {
				unp->_retAdr = 0xa7ae;
				mem[p + 0x87] = 0xae;
				mem[p + 0x88] = 0xa7;
			}
			unp->_endAdr = 0xae;
			unp->_idFlag = 1;
			return;
		}
	}
	/* Crackman variant? */
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x827) == 0x38BD00A2) &&
			(*(unsigned int *)(mem + 0x82b) == 0x01009D08) &&
			(*(unsigned int *)(mem + 0x837) == 0x34A97801) &&
			(*(unsigned int *)(mem + 0x891) == 0xB1018420) &&
			(*(unsigned int *)(mem + 0x89f) == 0x018b20AE)) {
			unp->_depAdr = 0x100;
			if (unp->_info->_run == -1)
				unp->_forced = 0x827;
			unp->_strMem = READ_LE_UINT16(&mem[0x86e]); // mem[0x86e] | mem[0x872] << 8;
			if (*(unsigned short int *)(mem + 0x8b7) == 0xff5b) {
				mem[0x8b6] = 0x2c;
				unp->_retAdr = READ_LE_UINT16(&mem[0x8ba]); // mem[0x8ba] | mem[0x8bb] << 8;
			} else {
				unp->_retAdr = READ_LE_UINT16(&mem[0x8b7]); // mem[0x8b7] | mem[0x8b8] << 8;
			}
			unp->_endAdr = 0xae;
			unp->_idFlag = 1;
			return;
		}
	}
	/* PET||SLAN variant? */
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x812) == 0x20BD00A2) &&
			(*(unsigned int *)(mem + 0x816) == 0x033c9D08) &&
			(*(unsigned int *)(mem + 0x863) == 0xB103B420) &&
			(*(unsigned int *)(mem + 0x86c) == 0x03BB20AE)) {
			unp->_depAdr = 0x33c;
			if (unp->_info->_run == -1)
				unp->_forced = 0x812;
			unp->_strMem = READ_LE_UINT16(&mem[0x856]); // mem[0x856] | mem[0x85a] << 8;
			unp->_retAdr = READ_LE_UINT16(&mem[0x896]); // mem[0x896] | mem[0x897] << 8;
			unp->_endAdr = 0xae;
			unp->_idFlag = 1;
			return;
		}
	}
}

} // End of namespace Scott
} // End of namespace Glk
