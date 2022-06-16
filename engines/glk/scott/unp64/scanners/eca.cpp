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

void scnECA(UnpStr *unp) {
	byte *mem;
	int q, p;
	if (unp->_idFlag)
		return;
	mem = unp->_mem;
	if (unp->_depAdr == 0) {
		// for(p=0x810;p<0x830;p+=0x4)
		for (p = 0x80d; p < 0x830; p += 0x1) {
			if ((*(unsigned int *)(mem + p + 0x08) == (unsigned int)0x2D9D0032 + p) &&
				(*(unsigned int *)(mem + p + 0x3a) == 0x2a2a2a2a) &&
				(*(unsigned int *)(mem + p + 0x0c) == 0xF710CA00)) {
				if (((*(unsigned int *)(mem + p + 0x00) & 0xf4fff000) == 0x8434A000) &&
					(*(unsigned int *)(mem + p + 0x04) == 0xBD05A201)) {
					unp->_forced = p + 1;
				} else if (((*(unsigned int *)(mem + p + 0x00) & 0xffffff00) == 0x04A27800) &&
						   (*(unsigned int *)(mem + p + 0x04) == 0xBDE80186)) {
					unp->_forced = p + 1;
				} else if (((*(unsigned int *)(mem + p - 0x03) & 0xffffff00) == 0x04A27800) &&
						   (*(unsigned int *)(mem + p + 0x04) == 0xBDE80186)) {
					unp->_forced = p - 2;
				} else if (*(unsigned int *)(mem + p - 0x03) == 0x8D00a978) {
					unp->_forced = p - 2;
				}
			}
			if (!unp->_forced) {
				if ((*(unsigned int *)(mem + p + 0x3a) == 0x2a2a2a2a) &&
					(*(unsigned int *)(mem + p + 0x02) == 0x8534A978) &&
					(mem[p - 3] == 0xa0)) {
					unp->_forced = p - 3;
					if (mem[p + 0x0d6] == 0x20 && mem[p + 0x0d7] == 0xe0 &&
						mem[p + 0x0d8] == 0x03 && mem[p + 0x1da] == 0x5b &&
						mem[p + 0x1e7] == 0x59) {
						/* antiprotection :D */
						mem[p + 0x0d6] = 0x4c;
						mem[p + 0x0d7] = 0xae;
						mem[p + 0x0d8] = 0xa7;
					}
				}
			}
			if (!unp->_forced) { /* FDT */
				if ((*(unsigned int *)(mem + p + 0x3a) == 0x2a2a2a2a) &&
					(*(unsigned int *)(mem + p + 0x03) == 0x8604A278) &&
					(*(unsigned int *)(mem + p + 0x0a) == 0x2D950842)) {
					unp->_forced = p + 3;
				}
			}
			if (!unp->_forced) {
				/* decibel hacks */
				if ((*(unsigned int *)(mem + p + 0x3a) == 0x2a2a2a2a) &&
					(*(unsigned int *)(mem + p + 0x00) == 0x9D085EBD) &&
					(*(unsigned int *)(mem + p - 0x06) == 0x018534A9)) {
					unp->_forced = p - 0x6;
				}
			}
			if (unp->_forced) {
				for (q = 0xd6; q < 0xde; q++) {
					if (mem[p + q] == 0x20) {
						if ((*(unsigned short int *)(mem + p + q + 1) == 0xa659) ||
							(*(unsigned short int *)(mem + p + q + 1) == 0xff81) ||
							(*(unsigned short int *)(mem + p + q + 1) == 0xe3bf) ||
							(*(unsigned short int *)(mem + p + q + 1) == 0xe5a0) ||
							(*(unsigned short int *)(mem + p + q + 1) == 0xe518)) {
							mem[p + q] = 0x2c;
							q += 2;
							continue;
						} else {
							unp->_retAdr = READ_LE_INT16(&mem[p + q + 1]); // mem[p + q + 1] | mem[p + q + 2] << 8;
							break;
						}
					}
					if (mem[p + q] == 0x4c) {
						unp->_retAdr = READ_LE_INT16(&mem[p + q + 1]); // mem[p + q + 1] | mem[p + q + 2] << 8;
						break;
					}
				}
				unp->_depAdr = READ_LE_INT16(&mem[p + 0x30]); // mem[p + 0x30] | mem[p + 0x31] << 8;
				// some use $2d, some $ae
				for (q = 0xed; q < 0x108; q++) {
					if (*(unsigned int *)(mem + p + q) == 0xA518F7D0) {
						unp->_endAdr = mem[p + q + 4];
						// if(unp->_DebugP)
						// printf("EndAdr from $%02x\n",unp->_endAdr);
						break;
					}
				}
				/*
				if anything it's unpacked to $d000-efff, it will be copied
				to $e000-ffff as last action in unpacker before starting.
				0196  20 DA 01  JSR $01DA ; some have this jsr nopped, reloc doesn't
				happen 0199  A9 37     LDA #$37 019b  85 01     STA $01 019d  58 CLI
				019e  20 00 0D  JSR $0D00 ; retaddr can be either here or following
				01a1  4C AE A7  JMP $A7AE
				01da  B9 00 EF  LDA $EF00,Y
				01dd  99 00 FF  STA $FF00,Y
				01e0  C8        INY
				01e1  D0 F7     BNE $01DA
				01e3  CE DC 01  DEC $01DC
				01e6  CE DF 01  DEC $01DF
				01e9  AD DF 01  LDA $01DF
				01ec  C9 DF     CMP #$DF   ;<< not fixed, found as lower as $44 for
				example 01ee  D0 EA     BNE $01DA 01f0  60        RTS Because of this,
				$d000-dfff will be a copy of $e000-efff. So if $2d points to >= $d000,
				SOMETIMES it's better save upto $ffff or: mem[$2d]|(mem[$2e]+$10)<<8
				Still it's not a rule and I don't know exactly when.
				17/06/09: Implemented but still experimental, so better check
				extensively. use -v to know when it does the adjustments. 28/10/09:
				whoops, was clearing ONLY $d000-dfff =)
				*/
				unp->_strMem = READ_LE_INT16(&mem[p + 0x32]); // mem[p + 0x32] | mem[p + 0x33] << 8;
				for (q = 0xcd; q < 0xd0; q++) {
					if ((*(unsigned int *)(mem + p + q) & 0xffff00ff) == 0xa9010020) {
						unp->_ecaFlg = READ_LE_INT16(&mem[p + q + 1]); // mem[p + q + 1] | mem[p + q + 2] << 8;
						for (q = 0x110; q < 0x11f; q++) {
							if ((*(unsigned int *)(mem + p + q) == 0x99EF00B9) &&
								(mem[p + q + 0x12] == 0xc9)) {
								unp->_ecaFlg |= (mem[p + q + 0x13] - 0xf) << 24;
								break;
							}
						}
						break;
					}
				}
				/* radwar hack has a BRK here, fffe/f used as IRQ/BRK vector */
				if (mem[0x8e1] == 0) {
					mem[0x8e1] = 0x6c;
					mem[0x8e2] = 0xfe;
					mem[0x8e3] = 0xff;
				}
				break;
			}
		}
		if (unp->_depAdr) {
			unp->_idFlag = 1;
			return;
		}
	}
	/* old packer, many old 1985 warez used this */
	if (unp->_depAdr == 0) {
		if ((*(unsigned int *)(mem + 0x81b) == 0x018534A9) &&
			(*(unsigned int *)(mem + 0x822) == 0xAFC600A0) &&
			(*(unsigned int *)(mem + 0x826) == 0xB1082DCE) &&
			(*(unsigned int *)(mem + 0x85b) == 0x2A2A2A2A)) {
			unp->_forced = 0x81b;
			unp->_depAdr = 0x100;
			unp->_strMem = READ_LE_INT16(&mem[0x853]); // mem[0x853] | mem[0x854] << 8;
			unp->_endAdr = mem[0x895];
			unp->_retAdr = READ_LE_INT16(&mem[0x885]); // mem[0x885] | mem[0x886] << 8;
			unp->_idFlag = 1;
			return;
		}
	}
}

} // End of namespace Scott
} // End of namespace Glk
