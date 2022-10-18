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

void scnExpert(UnpStr *unp) {
	byte *mem;
	int q, p;
	if (unp->_idFlag)
		return;
	mem = unp->_mem;
	if (unp->_depAdr == 0) {
		for (q = 0x81b; q < 0x81d; q++) {
			if ((*(unsigned int *)(mem + q + 0x00) == 0x852FA978) &&
				(*(unsigned int *)(mem + q + 0x04) == 0x8534A900) &&
				(*(unsigned int *)(mem + q + 0x14) == 0x03860286)) {
				for (p = 0x900; p < 0xfff0; p++) {
					if ((*(unsigned int *)(mem + p + 1) == 0x00084C9A) &&
						(*(unsigned int *)(mem + p - 4) == 0xA2058604)) {
						if (unp->_info->_run == -1) {
							unp->_forced = q;
							unp->_info->_run = q;
						}
						q = 0x100 + mem[p] + 1;
						if (q != 0x100) {
							unp->_depAdr = q;
						}
					}
				}
				break;
			}
		}
		if (unp->_depAdr) {
			unp->_rtiFrc = 1;
			if (*(unsigned int *)(mem + 0x835) == 0x6E8D48A9) {
				p = 0;
				if (*(unsigned int *)(mem + 0x92c) == 0x4902B100) {
					if (!unp->_idOnly) {
						p = 0x876;
						mem[p] = 0x00; /* 1st anti hack */
						p = mem[0x930];
					}
				} else if (*(unsigned int *)(mem + 0x92f) == 0x4902B100) {
					if (!unp->_idOnly) {
						p = 0x873;
						mem[p] = 0xa9; /* 1st anti hack */
						mem[p + 1] = 0x02;
						p = mem[0x933];
					}
				}
				if (p && !unp->_idOnly) {
					p |= (p << 24) | (p << 16) | (p << 8);
					for (q = 0x980; q < 0xfff0; q++) {
						if (((mem[q] ^ (p & 0xff)) == 0xac) &&
							((mem[q + 3] ^ (p & 0xff)) == 0xc0) &&
							(((*(unsigned int *)(mem + q + 7)) ^ p) == 0xC001F2AC)) {
							mem[q + 0x06] = (p & 0xff); /* 2nd anti hack */
							mem[q + 0x0d] = (p & 0xff);
							break;
						}
					}
				}
			}
		}
	}
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x81b) == 0x2FA9D878) &&
			(*(unsigned int *)(mem + 0x82d) == 0x0873BDB0)) {
			for (p = 0x900; p < 0xfff0; p++) {
				if ((*(unsigned int *)(mem + p) == 0xA2F3D0CA) &&
					(mem[p + 0x05] == 0x4c)) {
					q = READ_LE_UINT16(&mem[p + 0x06]); // mem[p + 0x06] | mem[p + 0x07] << 8;
					if (q != 0x100) {
						unp->_depAdr = q;
						break;
					}
				}
			}
			if (unp->_depAdr) {
				unp->_rtiFrc = 1;
				unp->_forced = 0x81b;
			}
		}
	}
	/* 2.9 Expert User Club version, found in
	   BloodMoney/HTL & SWIV/Inceria
	*/
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x81b) == 0x8C00A078) &&
			(*(unsigned int *)(mem + 0x831) == 0x05860485) &&
			(*(unsigned int *)(mem + 0x998) == 0x00084C9A)) {
			p = mem[0x919];
			q = p << 24 | p << 16 | p << 8 | p;
			for (p = 0x900; p < 0xfff0; p++) {
				if (((*(unsigned int *)(mem + p) ^ q) == 0xA2F3D0CA) &&
					((mem[p + 0x05] ^ (q & 0xff)) == 0x4c)) {
					q = (mem[p + 0x06] ^ (q & 0xff)) | (mem[p + 0x07] ^ (q & 0xff)) << 8;
					if (q != 0x100) {
						unp->_depAdr = q;
						break;
					}
				}
			}
			if (unp->_depAdr) {
				unp->_rtiFrc = 1;
				unp->_forced = 0x81b;
			}
		}
	}
	/* sys2070 A.S.S. */
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x817) == 0x00852FA9) &&
			(*(unsigned int *)(mem + 0x823) == 0x05860485) &&
			(*(unsigned int *)(mem + 0x9a0) == 0x00084C9A)) {
			p = mem[0x923];
			q = p << 24 | p << 16 | p << 8 | p;
			for (p = 0x900; p < 0xfff0; p++) {
				if (((*(unsigned int *)(mem + p) ^ q) == 0xA2F3D0CA) &&
					((mem[p + 0x05] ^ (q & 0xff)) == 0x4c)) {
					q = (mem[p + 0x06] ^ (q & 0xff)) | (mem[p + 0x07] ^ (q & 0xff)) << 8;
					if (q != 0x100) {
						unp->_depAdr = q;
						break;
					}
				}
			}
			if (unp->_depAdr) {
				unp->_rtiFrc = 1;
				unp->_forced = 0x81b;
			}
		}
	}
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x81b) == 0x7FA978D8) ||
			(*(unsigned int *)(mem + 0x81b) == 0x7FA9D878) ||
			(*(unsigned int *)(mem + 0x816) == 0x7FA978D8)) {
			for (p = 0x900; p < 0xfff0; p++) {
				if ((*(unsigned int *)(mem + p) == 0xA2F3D0CA) &&
					(mem[p + 0x05] == 0x4c)) {
					q = READ_LE_UINT16(&mem[p + 0x06]); // mem[p + 0x06] | mem[p + 0x07] << 8;
					if (q != 0x100) {
						unp->_depAdr = q;
						break;
					}
				}
			}
			if (unp->_depAdr) {
				unp->_rtiFrc = 1;
				if (*(unsigned int *)(mem + 0x816) == 0x7FA978D8) {
					q = 0x816;
					if (!unp->_idOnly) {
						for (p = 0x900; p < 0xfff0; p++) {
							if ((*(unsigned int *)(mem + p) == 0xE0A9F0A2) &&
								(*(unsigned int *)(mem + p + 4) == 0xE807135D) &&
								(mem[p + 0x8] == 0xd0)) {
								mem[p + 0x1] = 0x00;
								mem[p + 0x3] = 0x98;
								memset(mem + p + 4, 0xea, 6);
								break;
							}
						}
					}
				} else {
					q = 0x81b;
					if (!unp->_idOnly) {
						for (p = 0x900; p < 0xfff0; p++) {
							if ((*(unsigned int *)(mem + p) == 0xCA08015D) &&
								(*(unsigned int *)(mem + p + 4) == 0xF8D003E0) &&
								(mem[p + 0xa] == 0xd0)) {
								p += 0xa;
								mem[p] = 0x24;
								break;
							}
						}
					}
				}
				if (unp->_info->_run == -1) {
					unp->_forced = q;
					unp->_info->_run = q;
				}
			}
		}
	}
	if (unp->_depAdr == 0) {
		q = 0x81b;
		if ((*(unsigned int *)(mem + q + 0x00) == 0x852FA978) &&
			(*(unsigned int *)(mem + q + 0x04) == 0x8534A900) &&
			(*(unsigned int *)(mem + q + 0x14) == 0x03860286) &&
			(*(unsigned int *)(mem + q + 0x4f) == 0xA200594C) &&
			(*(unsigned int *)(mem + q + 0xad) == 0x2000124C)) {
			unp->_forced = q;
			unp->_info->_run = q;
			unp->_depAdr = 0x12;
			unp->_rtiFrc = 1;
		}
	}
	/* expert 2.11 (sys2074) & unknown sys2061 */
	if (unp->_depAdr == 0) {
		for (q = 0x80d; q < 0x820; q++) {
			if ((*(unsigned int *)(mem + q + 0x00) == 0x852FA978) &&
				(*(unsigned int *)(mem + q + 0x04) == 0x8534A900) &&
				(*(unsigned int *)(mem + q + 0x13) == 0x03840284) &&
				(*(unsigned int *)(mem + q + 0x4f) == 0x084C003A) &&
				(*(unsigned int *)(mem + q + 0xad) == 0x00AA2048)) {
				unp->_forced = q;
				unp->_info->_run = q;
				unp->_depAdr = 0x100 + mem[q + 0x17a] + 1;
				break;
			}
		}
		if (unp->_depAdr != 0) {
			unp->_rtiFrc = 1;
		}
	}

	if (unp->_depAdr != 0) {
		unp->_idFlag = 1;
		return;
	}
}

} // End of namespace Scott
} // End of namespace Glk
