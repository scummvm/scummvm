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

void scnCaution(UnpStr *unp) {
	byte *mem;

	if (unp->_idFlag)
		return;
	mem = unp->_mem;
	/* quickpacker 1.0 sysless */
	if (unp->_depAdr == 0) {
		if (u32eq(mem + 0x801, 0xE67800A2) &&
			u32eq(mem + 0x805, 0x07EDBD01) &&
			u32eq(mem + 0x80d, 0x00284CF8) &&
			u32eq(mem + 0x844, 0xAC00334C)) {
			unp->_forced = 0x801;
			unp->_depAdr = 0x28;
			unp->_retAdr = READ_LE_UINT16(&mem[0x86b]);
			unp->_endAdr = READ_LE_UINT16(&mem[0x85a]);
			unp->_fStrAf = mem[0x863];
			unp->_strAdC = EA_ADDFF | 0xffff;
			unp->_idFlag = 1;
			return;
		}
	}
	/* quickpacker 2.x + sys */
	if (unp->_depAdr == 0) {
		if (u32eqmasked(mem + 0x80b, 0xf0ffffff, 0x60A200A0) &&
			u32eq(mem + 0x80f, 0x0801BD78) &&
			u32eq(mem + 0x813, 0xD0CA0095) &&
			u32eq(mem + 0x81e, 0xD0C80291) &&
			u32eq(mem + 0x817, 0x001A4CF8)) {
			unp->_forced = 0x80b;
			unp->_depAdr = 0x01a;
			if (mem[0x80e] == 0x69) {
				unp->_retAdr = READ_LE_UINT16(&mem[0x842]);
				unp->_endAdr = READ_LE_UINT16(&mem[0x850]);
				unp->_endAdr += 0x100;
				unp->_fStrAf = 0x4f;
				unp->_strAdC = 0xffff | EA_USE_Y;
				unp->_idFlag = 1;
				return;
			} else if (mem[0x80e] == 0x6c) {
				unp->_retAdr = READ_LE_UINT16(&mem[0x844]);
				unp->_endAdr = READ_LE_UINT16(&mem[0x84e]);
				unp->_endAdr++;
				unp->_fStrAf = 0x4d;
				unp->_idFlag = 1;
				return;
			}
		}
	}
	/* strangely enough, sysless v2.0 depacker is at $0002 */
	if (unp->_depAdr == 0) {
		if (u32eq(mem + 0x83d, 0xAA004A20) &&
			u32eq(mem + 0x801, 0xA27800A0) &&
			u32eq(mem + 0x805, 0x080FBD55) &&
			u32eq(mem + 0x809, 0xD0CA0095) &&
			u32eq(mem + 0x80d, 0x00024CF8)) {
			unp->_forced = 0x801;
			unp->_depAdr = 0x2;
			unp->_retAdr = READ_LE_UINT16(&mem[0x83b]);
			unp->_endAdr = READ_LE_UINT16(&mem[0x845]);
			unp->_endAdr++;
			unp->_fStrAf = mem[0x849];
			// unp->_StrAdC=0xffff;
			unp->_idFlag = 1;
			return;
		}
	}
	/* same goes for v2.5 sysless, seems almost another packer */
	if (unp->_depAdr == 0) {
		if (u32eq(mem + 0x83b, 0xAA005520) &&
			u32eq(mem + 0x801, 0x60A200A0) &&
			u32eq(mem + 0x805, 0x0801BD78) &&
			u32eq(mem + 0x809, 0xD0CA0095) &&
			u32eq(mem + 0x80d, 0x00104CF8)) {
			unp->_forced = 0x801;
			unp->_depAdr = 0x10;
			unp->_retAdr = READ_LE_UINT16(&mem[0x839]);
			unp->_endAdr = READ_LE_UINT16(&mem[0x847]);
			unp->_endAdr += 0x100;
			unp->_fStrAf = 0x46;
			unp->_strAdC = 0xffff | EA_USE_Y;
			unp->_idFlag = 1;
			return;
		}
	}
	/* hardpacker */
	if (unp->_depAdr == 0) {
		if (u32eq(mem + 0x80d, 0x8534A978) &&
			u32eq(mem + 0x811, 0xB9B3A001) &&
			u32eq(mem + 0x815, 0x4C99081F) &&
			u32eq(mem + 0x819, 0xF7D08803) &&
			u32eq(mem + 0x81d, 0xB9034D4C)) {
			unp->_forced = 0x80d;
			unp->_depAdr = 0x34d;
			unp->_retAdr = READ_LE_UINT16(&mem[0x87f]);
			unp->_endAdr = READ_LE_UINT16(&mem[0x88d]);
			unp->_fStrAf = 0x3ba;
			unp->_strAdC = EA_ADDFF | 0xffff;
			unp->_idFlag = 1;
			return;
		}
	}
}

} // End of namespace Scott
} // End of namespace Glk
