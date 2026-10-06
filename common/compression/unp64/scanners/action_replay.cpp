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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/endian.h"
#include "common/compression/unp64/exo_util.h"
#include "common/compression/unp64/unp64.h"

namespace Common {
namespace Unp64 {

// Adapted from UNP64 2.42's Action Replay, Super Snapshot and Freeze Machine scanner.
void scnActionReplay(UnpStr *unp) {
	if (unp->_idFlag || unp->_depAdr)
		return;

	const byte *mem = unp->_mem;
	if (READ_LE_UINT32(mem + 0x80f) == 0xdd0d8d7f &&
		READ_LE_UINT32(mem + 0x927) == 0x4ce7d0ca) {
		uint16 destination = READ_LE_UINT16(mem + 0x92b);
		for (uint offset = 0; offset < 0x100; ++offset) {
			const byte *code = mem + 0xa00 + offset;
			if (code[0] == 0x99 && READ_LE_UINT16(code + 1) == destination &&
				(code[0x11] == 0x40 || code[0x10] == 0x40)) {
				unp->_depAdr = 0x100 + offset;
				break;
			}
		}
		if (!unp->_depAdr)
			unp->_depAdr = destination;
	} else if (READ_LE_UINT32(mem + 0x812) == 0xdd0d8ddc) {
		for (uint address = 0x900; address < 0xcfff; ++address) {
			if (READ_LE_UINT32(mem + address) == 0xa9dc0e8d &&
				READ_LE_UINT32(mem + address + 0x0a) == 0xa9dd0e8d &&
				mem[address + 0x18] == 0x4c) {
				unp->_depAdr = READ_LE_UINT16(mem + address + 0x19);
				break;
			}
		}
	} else if (READ_LE_UINT32(mem + 0x80f) == 0xdd0d8d7f &&
		READ_LE_UINT32(mem + 0x8ef) == 0x0330bd01 &&
		READ_LE_UINT32(mem + 0x8fa) == 0x7e4c00a0 &&
		READ_LE_UINT32(mem + 0xbc8) == 0x4c01c6df) {
		unp->_depAdr = READ_LE_UINT16(mem + 0xbcc);
		unp->_strMem = 2;
		unp->_endAdr = 0x10000;
	}

	if (unp->_depAdr) {
		unp->_rtiFrc = 1;
		if (unp->_info->_run == -1)
			unp->_forced = 0x80d;
		unp->_idFlag = 1;
	}
}

} // End of namespace Unp64
} // End of namespace Common
