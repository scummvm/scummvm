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

void scnByteBoiler(UnpStr *unp) {
	byte *mem;
	int q, p;
	if (unp->_idFlag)
		return;
	mem = unp->_mem;
	if (unp->_depAdr == 0) {
		if (u32eq(mem + 0x813, 0xE800F09D) &&
			u32eq(mem + 0x818, 0x014E4CF7)) {
			p = READ_LE_UINT16(&mem[0x811]);
			if (u32eq(mem + p + 1, 0x02D0FAA5)) {
				unp->_depAdr = 0x14e;
				unp->_forced = 0x80b;
				unp->_retAdr = READ_LE_UINT16(&mem[p + 0x5c]);
				unp->_endAdr = READ_LE_UINT16(&mem[p + 0x0e]);
				unp->_endAdr++;
				unp->_fStrAf = 0xfe;
				unp->_idFlag = 1;
				return;
			}
		}
	}
	/* CPX hack */
	if (unp->_depAdr == 0) {
		if (u32eq(mem + 0x80b, 0xA97800A2) &&
			u32eq(mem + 0x815, 0x4C01E6D0)) {
			q = READ_LE_UINT16(&mem[0x819]);
			if (u32eq(mem + q + 3, 0xE800F09D) &&
				u32eq(mem + q + 8, 0x014E4CF7)) {
				p = READ_LE_UINT16(&mem[q + 1]);
				if (u32eq(mem + p + 1, 0x02D0FAA5)) {
					unp->_depAdr = 0x14e;
					unp->_forced = 0x80b;
					unp->_retAdr = READ_LE_UINT16(&mem[p + 0x5c]);
					unp->_endAdr = READ_LE_UINT16(&mem[p + 0x0e]);
					unp->_endAdr++;
					unp->_fStrAf = 0xfe;
					unp->_idFlag = 1;
					return;
				}
			}
		}
	}
	/* SCS hack */
	if (unp->_depAdr == 0) {
		if (u32eq(mem + 0x813, 0xE800F09D) &&
			u32eq(mem + 0x818, 0x01bf4CF7)) {
			p = READ_LE_UINT16(&mem[0x811]);
			if (u32eq(mem + p + 1, 0x02D0FAA5) &&
				u32eq(mem + p + 0xdd, 0x014e4c01)) {
				unp->_depAdr = 0x14e;
				unp->_forced = 0x80b;
				unp->_retAdr = READ_LE_UINT16(&mem[p + 0x5c]);
				unp->_endAdr = READ_LE_UINT16(&mem[p + 0x0e]);
				unp->_endAdr++;
				unp->_fStrAf = 0xfe;
				unp->_idFlag = 1;
				return;
			}
		}
	}
}

} // End of namespace Scott
} // End of namespace Glk
