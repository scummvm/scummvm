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

// Adapted from the ScottFree version of UNP64 for shared use in ScummVM.

/*
UNP64 - generic Commodore 64 prg unpacker
(C) 2008-2018 iAN CooG/HVSC Crew^C64Intros
original source and idea: testrun.c, taken from exo20b7

Follows original disclaimer
*/

/*
 * Copyright (c) 2002 - 2023 Magnus Lind.
 *
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 */

#include "common/compression/unp64.h"
#include "common/compression/unp64/6502_emu.h"
#include "common/compression/unp64/exo_util.h"
#include "common/compression/unp64/unp64.h"

#include "common/str.h"
#include "common/util.h"

namespace Common {
namespace Unp64 {

static void reinitUnp(UnpStr &unp) {
	unp._idFlag = 0;
	unp._forced = 0;
	unp._strMem = 0x800;
	unp._retAdr = 0x800;
	unp._depAdr = 0;
	unp._endAdr = 0x10000;
	unp._rtAFrc = 0;
	unp._wrMemF = 0;
	unp._lfMemF = 0;
	unp._exoFnd = 0;
	unp._ecaFlg = 0;
	unp._fEndBf = 0;
	unp._fEndAf = 0;
	unp._fStrAf = 0;
	unp._fStrBf = 0;
	unp._mon1st = 0;
}

static int isBasicRun1(int pc) {
	if (pc == 0xa7ae || pc == 0xa7ea || pc == 0xa7b1 || pc == 0xa474 || pc == 0xa533 || pc == 0xa871 || pc == 0xa888 || pc == 0xa8bc)
		return 1;
	else
		return 0;
}

static int isBasicRun2(int pc) {
	if (isBasicRun1(pc) || ((pc >= 0xA57C) && (pc <= 0xA659)) || pc == 0xa660 || pc == 0xa68e)
		return 1;
	else
		return 0;
}

int unp64(const byte *compressed, uint32 length, byte *destinationBuffer, uint32 *finalLength, const char *switches) {

	char settings[4][64];
	int numSettings = 0;

	if (switches != NULL) {
		char string[100];
		size_t string_length = strlen(switches);
		if (string_length > 0 && string_length < 100) {
			snprintf(string, sizeof string, "%s", switches);
			char *setting = strtok(string, " ");
			while (setting != NULL && numSettings < 4) {
				snprintf(settings[numSettings], sizeof settings[numSettings], "%s", setting);
				numSettings++;
				setting = strtok(NULL, " ");
			}
		}
	}

	UnpStr unp;
	int iter = 0;
	CpuCtx r[1];
	LoadInfo info[1];
	char name[260] = {0}, forcedname[260] = {0};
	byte mem[65536] = {0}, oldmem[65536] = {0};
	byte vector[0x20] = {0x31, 0xEA, 0x66, 0xFE, 0x47, 0xFE, 0x4A, 0xF3,
						 0x91, 0xF2, 0x0E, 0xF2, 0x50, 0xF2, 0x33, 0xF3,
						 0x57, 0xF1, 0xCA, 0xF1, 0xED, 0xF6, 0x3E, 0xF1,
						 0x2F, 0xF3, 0x66, 0xFE, 0xA5, 0xF4, 0xED, 0xF5};

	byte stack[0x100] = {0x33, 0x38, 0x39, 0x31, 0x31, 0x00, 0x30, 0x30, 0x30, 0x30,
						 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
						 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7D,
						 0xEA, 0x00, 0x00, 0x82, 0x22, 0x0E, 0xBC, 0x81, 0x64, 0xB8,
						 0x0C, 0xBD, 0xBA, 0xB7, 0xBC, 0x03, 0x00, 0x46, 0xE1, 0xE9,
						 0xA7, 0xA7, 0x79, 0xA6, 0x9C, 0xE3};

	int iterMax = ITERMAX;
	int p;

	memset(&unp, 0, sizeof(unp));
	reinitUnp(unp);
	unp._fStack = 1;
	unp._mem = mem;
	unp._r = r;
	unp._name = name;
	unp._info = info;

	p = 0;

	if (numSettings != 0) {
		if (settings[0][0] == '-' && settings[0][1] == 'f') {
			strToInt(settings[p] + 2, (int *)&unp._filler);
			if (unp._filler) {
				memset(mem + (unp._filler >> 16), unp._filler & 0xff, 0x10000 - (unp._filler >> 16));
			}
			p++;
		}
	}

	looprecurse:
	info->_basicTxtStart = 0x801;
	loadData(compressed, length, mem, info);
		/* no start address from load */
	if (info->_run == -1) {
		/* look for sys line */
		info->_run = findSys(mem + info->_basicTxtStart, 0x9e);
	}

	scanners(&unp);
	if (unp._idFlag == 2)
		return 0;

	if ((unp._recurs == 0) && (numSettings > 0)) {
		while (p < numSettings) {
			if (settings[p][0] == '-') {
				switch (settings[p][1]) {
				case '-':
					p = numSettings;
					break;
				case 'e':
					strToInt(settings[p] + 2, &unp._forced);
					unp._forced &= 0xffff;
					if (unp._forced < 0x1)
						unp._forced = 0;
					break;
				case 'a':
					unp._strMem = 2;
					unp._endAdr = 0x10001;
					unp._fEndAf = 0;
					unp._fStrAf = 0;
					unp._strAdC = 0;
					unp._endAdC = 0;
					unp._monEnd = 0;
					unp._monStr = 0;
					break;
				case 'r':
					strToInt(settings[p] + 2, &unp._retAdr);
					unp._retAdr &= 0xffff;
					break;
				case 'R':
					strToInt(settings[p] + 2, &unp._retAdr);
					unp._retAdr &= 0xffff;
					unp._rtAFrc = 1;
					break;
				case 'd':
					strToInt(settings[p] + 2, &unp._depAdr);
					unp._depAdr &= 0xffff;
					break;
				case 't':
					strToInt(settings[p] + 2, &unp._endAdr);
					unp._endAdr &= 0xffff;
					if (unp._endAdr >= 0x100)
						unp._endAdr++;
					break;
				case 'u':
					unp._wrMemF = 1;
					break;
				case 'l':
					unp._lfMemF = info->_end;
					break;
				case 's':
					unp._fStack = 0;
					break;
				case 'x':
					break;
				case 'B':
					//copyRoms[0][1] = 1;
					break;
				case 'K':
					//copyRoms[1][1] = 1;
					break;
				case 'c':
					unp._recurs++;
					break;
				case 'm': // keep undocumented for now
					strToInt(settings[p] + 2, &iterMax);
				}
			}
			p++;
		}
	}

	if (unp._idOnly) {
		if (unp._depAdr == 0)
			return 0;
	}

	if (unp._wrMemF | unp._lfMemF) {
		memcpy(oldmem, mem, sizeof(oldmem));
	}

	if (unp._forced) {
		info->_run = unp._forced;
	}

	if (info->_run == -1) {
		return 0;
	}

	if (unp._strMem > unp._retAdr) {
		unp._strMem = unp._retAdr;
	}

	mem[0] = 0x60;
	r->_cycles = 0;
	mem[1] = 0x37;

	if (((unp._forced >= 0xa000) && (unp._forced < 0xc000)) || (unp._forced >= 0xd000))
		mem[1] = 0x38;

	/* some packers rely on basic pointers already set */
	mem[0x2b] = info->_basicTxtStart & 0xff;
	mem[0x2c] = info->_basicTxtStart >> 8;

	if (info->_basicVarStart == -1) {
		mem[0x2d] = info->_end & 0xff;
		mem[0x2e] = info->_end >> 8;
	} else {
		mem[0x2d] = info->_basicVarStart & 0xff;
		mem[0x2e] = info->_basicVarStart >> 8;
	}

	mem[0x2f] = mem[0x2d];
	mem[0x30] = mem[0x2e];
	mem[0x31] = mem[0x2d];
	mem[0x32] = mem[0x2e];
	mem[0xae] = info->_end & 0xff;
	mem[0xaf] = info->_end >> 8;

	/* CCS unpacker requires $39/$3a (current basic line number) set */
	mem[0x39] = mem[0x803];
	mem[0x3a] = mem[0x804];
	mem[0x52] = 0;
	mem[0x53] = 3;

	if (unp._fStack) {
		memcpy(mem + 0x100, stack,
			   sizeof(stack)); /* stack as found on clean start */
		r->_sp = 0xf6;         /* sys from immediate mode leaves $f6 in stackptr */
	} else {
		r->_sp = 0xff;
	}

	if (info->_start > (long int)(0x314 + sizeof(vector))) {
		/* some packers use values in irq pointers to decrypt themselves */
		memcpy(mem + 0x314, vector, sizeof(vector));
	}

	mem[0x200] = 0x8a;
	r->_mem = mem;
	r->_pc = info->_run;
	r->_flags = 0x20;
	r->_a = 0;
	r->_y = 0;

	if (info->_run > 0x351) /* temporary for XIP */ {
		r->_x = 0;
	}

	iter = 0;
	while ((unp._depAdr ? r->_pc != unp._depAdr : r->_pc >= unp._retAdr)) {
		if ((((mem[1] & 0x7) >= 6) && (r->_pc >= 0xe000)) || ((r->_pc >= 0xa000) && (r->_pc <= 0xbfff) && ((mem[1] & 0x7) > 6))) {
			/* some packer relies on regs set at return from CLRSCR */
			if ((r->_pc == 0xe536) || (r->_pc == 0xe544) || (r->_pc == 0xff5b) || ((r->_pc == 0xffd2) && (r->_a == 0x93))) {
				if (r->_pc != 0xffd2) {
					r->_x = 0x01;
					r->_y = 0x84;

					if (r->_pc == 0xff5b)
						r->_a = 0x97; /* actually depends on $d012 */
					else
						r->_a = 0xd8;

					r->_flags &= ~(128 | 2);
					r->_flags |= (r->_a == 0 ? 2 : 0) | (r->_a & 128);
				}
				memset(mem + 0x400, 0x20, 1000);
			}
			/* intros */
			if ((r->_pc == 0xffe4) || (r->_pc == 0xf13e)) {
				static int flipspe4 = -1;
				static unsigned char fpressedchars[] = {0x20, 0, 0x4e, 0, 3, 0, 0x5f, 0, 0x11, 00, 0x0d, 0, 0x31, 0};
				flipspe4++;

				if (flipspe4 > ARRAYSIZE(fpressedchars))
					flipspe4 = 0;

				r->_a = fpressedchars[flipspe4];
				r->_flags &= ~(128 | 2);
				r->_flags |= (r->_a == 0 ? 2 : 0) | (r->_a & 128);
			}

			if (r->_pc == 0xfd15) {
				r->_a = 0x31;
				r->_x = 0x30;
				r->_y = 0xff;
			}

			if (r->_pc == 0xfda3) {
				mem[0x01] = 0xe7;
				r->_a = 0xd7;
				r->_x = 0xff;
			}

			if (r->_pc == 0xffbd) {
				mem[0xB7] = r->_a;
				mem[0xBB] = r->_x;
				mem[0xBC] = r->_y;
			}

			if ((r->_pc == 0xffd5) || (r->_pc == 0xf4a2)) {
				break;
			}

			if (isBasicRun1(r->_pc)) {
				info->_run = findSys(mem + info->_basicTxtStart, 0x9e);
				if (info->_run > 0) {
					r->_sp = 0xf6;
					r->_pc = info->_run;
				} else {
					mem[0] = 0x60;
					r->_pc = 0; /* force a RTS instead of executing ROM code */
				}
			} else {
				mem[0] = 0x60;
				r->_pc = 0; /* force a RTS instead of executing ROM code */
			}
		}

		if (nextInst(r) == 1)
			return 0;

		iter++;
		if (iter == iterMax) {
				return 0;
		}

		if (unp._exoFnd && (unp._endAdr == 0x10000) && (r->_pc >= 0x100) && (r->_pc <= 0x200) && (unp._strMem != 2)) {
			unp._endAdr = r->_mem[0xfe] + (r->_mem[0xff] << 8);
			if ((unp._exoFnd & 0xff) == 0x30) { /* low byte of _endAdr, it's a lda $ff00,y */
				unp._endAdr = (unp._exoFnd >> 8) + (r->_mem[0xff] << 8);
			} else if ((unp._exoFnd & 0xff) == 0x32) { /* add 1 */
				unp._endAdr = 1 + ((unp._exoFnd >> 8) + (r->_mem[0xff] << 8));
			}

			if (unp._endAdr == 0)
				unp._endAdr = 0x10001;
		}

		if (unp._fEndBf && (unp._endAdr == 0x10000) && (r->_pc == unp._depAdr)) {
			unp._endAdr = r->_mem[unp._fEndBf] | r->_mem[unp._fEndBf + 1] << 8;
			unp._endAdr++;

			if (unp._endAdr == 0)
					unp._endAdr = 0x10001;

			unp._fEndBf = 0;
		}

		if (unp._fStrBf && (unp._strMem != 0x2) && (r->_pc == unp._depAdr)) {
			unp._strMem = r->_mem[unp._fStrBf] | r->_mem[unp._fStrBf + 1] << 8;
			unp._fStrBf = 0;
		}

		if (unp._debugP) {
			for (p = 0; p < 0x20; p += 2) {
				if (*(unsigned short int *)(mem + 0x314 + p) != *(unsigned short int *)(vector + p)) {
					*(unsigned short int *)(vector + p) = *(unsigned short int *)(mem + 0x314 + p);
				}
			}
		}
	}

	iter = 0;
	while (unp._rtAFrc ? r->_pc != unp._retAdr : r->_pc < unp._retAdr) {
		if (unp._monEnd && r->_pc == unp._depAdr) {
			p = r->_mem[unp._monEnd >> 16] | r->_mem[unp._monEnd & 0xffff] << 8;
			if (p > (unp._endAdr & 0xffff)) {
				unp._endAdr = p;
			}
		}
		if (unp._monStr && r->_pc == unp._depAdr) {
			p = r->_mem[unp._monStr >> 16] | r->_mem[unp._monStr & 0xffff] << 8;
			if (p > 0) {
				if (unp._mon1st == 0) {
					unp._strMem = p;
				}
				unp._mon1st = (unsigned int)unp._strMem;
				unp._strMem = (p < unp._strMem ? p : unp._strMem);
			}
		}

		if (r->_pc >= 0xe000) {
			if (((mem[1] & 0x7) >= 6) && ((mem[1] & 0x7) <= 7)) {
				mem[0] = 0x60;
				r->_pc = 0;
			}
		}
		if (nextInst(r) == 1)
			return 0;

		if ((mem[r->_pc] == 0x40) && (unp._rtiFrc == 1)) {
			unp._retAdr = r->_pc;
			unp._rtAFrc = 1;
			if (unp._retAdr < unp._strMem)
				unp._strMem = 2;
			break;
		}

		iter++;
		if (iter == iterMax) {
			return 0;
		}

		if ((r->_pc >= 0xa000) && (r->_pc <= 0xbfff) && ((mem[1] & 0x7) == 7)) {
			if (isBasicRun2(r->_pc)) {
				r->_pc = 0xa7ae;
				break;
			} else {
				mem[0] = 0x60;
				r->_pc = 0;
			}
		}

		if (r->_pc >= 0xe000) {
			if (((mem[1] & 0x7) >= 6) && ((mem[1] & 0x7) <= 7)) {
				if (r->_pc == 0xffbd) {
					mem[0xB7] = r->_a;
					mem[0xBB] = r->_x;
					mem[0xBC] = r->_y;
				}
				/* return into IRQ handler, better stop here */
				if (((r->_pc >= 0xea31) && (r->_pc <= 0xeb76)) || (r->_pc == 0xffd5) || (r->_pc == 0xfce2)) {
					break;
				}

				if (r->_pc == 0xfda3) {
					mem[0x01] = 0xe7;
					r->_a = 0xd7;
					r->_x = 0xff;
				}
				mem[0] = 0x60;
				r->_pc = 0;
			}
		}
	}

	if (unp._fEndAf && unp._monEnd) {
		unp._endAdC = (unsigned int)(mem[unp._fEndAf] | mem[unp._fEndAf + 1] << 8);
		if ((int)unp._endAdC > unp._endAdr)
			unp._endAdr = (int)unp._endAdC;

		unp._endAdC = 0;
		unp._fEndAf = 0;
	}

	if (unp._fEndAf && (unp._endAdr == 0x10000)) {
		unp._endAdr = r->_mem[unp._fEndAf] | r->_mem[unp._fEndAf + 1] << 8;
		if (unp._endAdr == 0)
			unp._endAdr = 0x10000;
		else
			unp._endAdr++;
		unp._fEndAf = 0;
	}

	if (unp._fStrAf /*&&(unp._strMem==0x800)*/) {
		unp._strMem = r->_mem[unp._fStrAf] | r->_mem[unp._fStrAf + 1] << 8;
		unp._strMem++;
		unp._fStrAf = 0;
	}

	if (unp._exoFnd && (unp._strMem != 2)) {
		unp._strMem = r->_mem[0xfe] + (r->_mem[0xff] << 8);

		if ((unp._exoFnd & 0xff) == 0x30) {
			unp._strMem += r->_y;
		} else if ((unp._exoFnd & 0xff) == 0x32) {
			unp._strMem += r->_y + 1;
		}
	}

	if (r->_pc == 0xfce2) {
		if ((*(unsigned int *)(mem + 0x8004) == 0x38cdc2c3) && (mem[0x8008] == 0x30)) {
			r->_pc = r->_mem[0x8000] + (r->_mem[0x8001] << 8);
		}
	} else if (r->_pc == 0xa7ae) {
		info->_basicTxtStart = mem[0x2b] | mem[0x2c] << 8;
		if (info->_basicTxtStart == 0x801) {
			info->_run = findSys(mem + info->_basicTxtStart, 0x9e);
			if (info->_run > 0)
				r->_pc = info->_run;
		}
	}

	if (unp._wrMemF) {
		unp._wrMemF = 0;
		for (p = 0x800; p < 0x10000; p += 4) {
			if (*(unsigned int *)(oldmem + p) == *(unsigned int *)(mem + p)) {
				*(unsigned int *)(mem + p) = 0;
				unp._wrMemF = 1;
			}
		}
		/* clean also the $fd30 table copy in RAM */
		if (memcmp(mem + 0xfd30, vector, sizeof(vector)) == 0) {
			memset(mem + 0xfd30, 0, sizeof(vector));
		}
	}

	if (unp._lfMemF) {
		for (p = 0xffff; p > 0x0800; p--) {
			if (oldmem[--unp._lfMemF] == mem[p])
				mem[p] = 0x0;
			else {
				if (p >= 0xffff)
					unp._lfMemF = 0 | unp._ecaFlg;
				break;
			}
		}
	}

	if (*forcedname) {
		Common::sprintf_s(name, sizeof name, "%s", forcedname);
	} else {
		size_t ln = strlen(name);
		if (ln > 248) {/* dirty hack in case name is REALLY long */
			name[248] = 0;
			ln = 248;
		}

		Common::sprintf_s(name + ln, sizeof(name) - ln, ".%04x%s", r->_pc, ((unp._wrMemF | unp._lfMemF) ? ".clean" : ""));
	}

	/*  endadr is set to a ZP location? then use it as a pointer
	  todo: use __fEndAf instead, it can be used for any location, not only ZP. */
	if (unp._endAdr && (unp._endAdr < 0x100)) {
		p = (mem[unp._endAdr] | mem[unp._endAdr + 1] << 8) & 0xffff;
		unp._endAdr = p;
	}

	if (unp._ecaFlg && (unp._strMem != 2)) /* checkme */ {
		if (unp._endAdr >= ((unp._ecaFlg >> 16) & 0xffff)) {
			/* most of the times transfers $2000 byte from $d000-efff to $e000-ffff but there are exceptions */
			if (unp._lfMemF)
				memset(mem + ((unp._ecaFlg >> 16) & 0xffff), 0, 0x1000);
			unp._endAdr += 0x1000;
		}
	}

	if (unp._endAdr <= 0)
		unp._endAdr = 0x10000;

	if (unp._endAdr > 0x10000)
		unp._endAdr = 0x10000;

	if (unp._endAdr < unp._strMem)
		unp._endAdr = 0x10000;

	if (unp._endAdC & 0xffff) {
		unp._endAdr += (unp._endAdC & 0xffff);
		unp._endAdr &= 0xffff;
	}

	if (unp._endAdC & EA_USE_A) {
		unp._endAdr += r->_a;
		unp._endAdr &= 0xffff;
	}

	if (unp._endAdC & EA_USE_X) {
		unp._endAdr += r->_x;
		unp._endAdr &= 0xffff;
	}

	if (unp._endAdC & EA_USE_Y) {
		unp._endAdr += r->_y;
		unp._endAdr &= 0xffff;
	}

	if (unp._strAdC & 0xffff) {
		unp._strMem += (unp._strAdC & 0xffff);
		unp._strMem &= 0xffff;
		/* only if ea_addff, no reg involved */
		if (((unp._strAdC & 0xffff0000) == EA_ADDFF) && ((unp._strMem & 0xff) == 0)) {
			unp._strMem += 0x100;
			unp._strMem &= 0xffff;
		}
	}

	if (unp._strAdC & EA_USE_A) {
		unp._strMem += r->_a;
		unp._strMem &= 0xffff;
		if (unp._strAdC & EA_ADDFF) {
			if ((unp._strMem & 0xff) == 0xff)
				unp._strMem++;

			if (r->_a == 0) {
				unp._strMem += 0x100;
				unp._strMem &= 0xffff;
			}
		}
	}

	if (unp._strAdC & EA_USE_X) {
		unp._strMem += r->_x;
		unp._strMem &= 0xffff;

		if (unp._strAdC & EA_ADDFF) {
			if ((unp._strMem & 0xff) == 0xff)
				unp._strMem++;

			if (r->_x == 0) {
				unp._strMem += 0x100;
				unp._strMem &= 0xffff;
			}
		}
	}

	if (unp._strAdC & EA_USE_Y) {
		unp._strMem += r->_y;
		unp._strMem &= 0xffff;

		if (unp._strAdC & EA_ADDFF) {
			if ((unp._strMem & 0xff) == 0xff)
				unp._strMem++;

			if (r->_y == 0) {
				unp._strMem += 0x100;
				unp._strMem &= 0xffff;
			}
		}
	}

	if (unp._endAdr <= 0)
		unp._endAdr = 0x10000;

	if (unp._endAdr > 0x10000)
		unp._endAdr = 0x10000;

	if (unp._endAdr < unp._strMem)
		unp._endAdr = 0x10000;

	mem[unp._strMem - 2] = unp._strMem & 0xff;
	mem[unp._strMem - 1] = unp._strMem >> 8;

	memcpy(destinationBuffer, mem + (unp._strMem - 2), (size_t)(unp._endAdr - unp._strMem + 2));
	*finalLength = (size_t)(unp._endAdr - unp._strMem + 2);

	if (unp._recurs) {
		if (++unp._recurs > RECUMAX)
			return 1;
		reinitUnp(unp);
		goto looprecurse;
	}
	return 1;
}

} // End of namespace Unp64
} // End of namespace Common
