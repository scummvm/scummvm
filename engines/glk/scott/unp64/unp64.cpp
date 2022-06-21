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

// This is a cut-down version of UNP64 with only the bare minimum
// needed to decompress a number of Scott Adams Commodore 64 games
// for the ScottFree interpreter.

/*
UNP64 - generic Commodore 64 prg unpacker
(C) 2008-2018 iAN CooG/HVSC Crew^C64Intros
original source and idea: testrun.c, taken from exo20b7

Follows original disclaimer
*/

/*
 * Copyright (c) 2002 - 2008 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */

#include "glk/scott/globals.h"
#include "glk/scott/types.h"
#include "glk/scott/unp64/6502_emu.h"
#include "glk/scott/unp64/exo_util.h"
#include "glk/scott/unp64/unp64.h"

namespace Glk {
namespace Scott {

void reinitUnp(void) {
	_G(_unp)._idFlag = 0;
	_G(_unp)._forced = 0;
	_G(_unp)._strMem = 0x800;
	_G(_unp)._retAdr = 0x800;
	_G(_unp)._depAdr = 0;
	_G(_unp)._endAdr = 0x10000;
	_G(_unp)._rtAFrc = 0;
	_G(_unp)._wrMemF = 0;
	_G(_unp)._lfMemF = 0;
	_G(_unp)._exoFnd = 0;
	_G(_unp)._ecaFlg = 0;
	_G(_unp)._fEndBf = 0;
	_G(_unp)._fEndAf = 0;
	_G(_unp)._fStrAf = 0;
	_G(_unp)._fStrBf = 0;
	_G(_unp)._mon1st = 0;
}

int isBasicRun1(int pc) {
	if (pc == 0xa7ae || pc == 0xa7ea || pc == 0xa7b1 || pc == 0xa474 || pc == 0xa533 || pc == 0xa871 || pc == 0xa888 || pc == 0xa8bc)
		return 1;
	else
		return 0;
}

int isBasicRun2(int pc) {
	if (isBasicRun1(pc) || ((pc >= 0xA57C) && (pc <= 0xA659)) || pc == 0xa660 || pc == 0xa68e)
		return 1;
	else
		return 0;
}

int unp64(uint8_t *compressed, size_t length, uint8_t *destinationBuffer, size_t *finalLength, char *settings[], int numSettings) {
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
	int copyRoms[2][2] = {{0xa000, 0}, {0xe000, 0}};
	int p;

	memset(&_G(_unp), 0, sizeof(_G(_unp)));
	reinitUnp();
	_G(_unp)._fStack = 1;
	_G(_unp)._mem = mem;
	_G(_unp)._r = r;
	_G(_unp)._name = name;
	_G(_unp)._info = info;

	p = 0;

	if (numSettings != 0) {
		if (settings[0][0] == '-' && _G(_parsePar) && settings[0][1] == 'f') {
			strToInt(settings[p] + 2, (int *)&_G(_unp)._filler);
			if (_G(_unp)._filler) {
				memset(mem + (_G(_unp)._filler >> 16), _G(_unp)._filler & 0xff, 0x10000 - (_G(_unp)._filler >> 16));
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

	scanners(&_G(_unp));
	if (_G(_unp)._idFlag == 2)
		return 0;

	if ((_G(_unp)._recurs == 0) && (numSettings > 0)) {
		while (p < numSettings) {
			if (settings && settings[p][0] == '-') {
				switch (settings[p][1]) {
				case '-':
					p = numSettings;
					break;
				case 'e':
					strToInt(settings[p] + 2, &_G(_unp)._forced);
					_G(_unp)._forced &= 0xffff;
					if (_G(_unp)._forced < 0x1)
						_G(_unp)._forced = 0;
					break;
				case 'a':
					_G(_unp)._strMem = 2;
					_G(_unp)._endAdr = 0x10001;
					_G(_unp)._fEndAf = 0;
					_G(_unp)._fStrAf = 0;
					_G(_unp)._strAdC = 0;
					_G(_unp)._endAdC = 0;
					_G(_unp)._monEnd = 0;
					_G(_unp)._monStr = 0;
					break;
				case 'r':
					strToInt(settings[p] + 2, &_G(_unp)._retAdr);
					_G(_unp)._retAdr &= 0xffff;
					break;
				case 'R':
					strToInt(settings[p] + 2, &_G(_unp)._retAdr);
					_G(_unp)._retAdr &= 0xffff;
					_G(_unp)._rtAFrc = 1;
					break;
				case 'd':
					strToInt(settings[p] + 2, &_G(_unp)._depAdr);
					_G(_unp)._depAdr &= 0xffff;
					break;
				case 't':
					strToInt(settings[p] + 2, &_G(_unp)._endAdr);
					_G(_unp)._endAdr &= 0xffff;
					if (_G(_unp)._endAdr >= 0x100)
						_G(_unp)._endAdr++;
					break;
				case 'u':
					_G(_unp)._wrMemF = 1;
					break;
				case 'l':
					_G(_unp)._lfMemF = info->_end;
					break;
				case 's':
					_G(_unp)._fStack = 0;
					break;
				case 'x':
					break;
				case 'B':
					copyRoms[0][1] = 1;
					break;
				case 'K':
					copyRoms[1][1] = 1;
					break;
				case 'c':
					_G(_unp)._recurs++;
					break;
				case 'm': // keep undocumented for now
					strToInt(settings[p] + 2, &iterMax);
				}
			}
			p++;
		}
	}

	if (_G(_unp)._idOnly) {
		if (_G(_unp)._depAdr == 0)
			return 0;
	}

	if (_G(_unp)._wrMemF | _G(_unp)._lfMemF) {
		memcpy(oldmem, mem, sizeof(oldmem));
	}

	if (_G(_unp)._forced) {
		info->_run = _G(_unp)._forced;
	}

	if (info->_run == -1) {
		return 0;
	}

	if (_G(_unp)._strMem > _G(_unp)._retAdr) {
		_G(_unp)._strMem = _G(_unp)._retAdr;
	}

	mem[0] = 0x60;
	r->_cycles = 0;
	mem[1] = 0x37;

	if (((_G(_unp)._forced >= 0xa000) && (_G(_unp)._forced < 0xc000)) || (_G(_unp)._forced >= 0xd000))
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

	if (_G(_unp)._fStack) {
		memcpy(mem + 0x100, stack,
			   sizeof(stack)); /* stack as found on clean start */
		r->_sp = 0xf6;         /* sys from immediate mode leaves $f6 in stackptr */
	} else {
		r->_sp = 0xff;
	}

	if (info->_start > (0x314 + sizeof(vector))) {
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

	_G(_iter) = 0;
	while ((_G(_unp)._depAdr ? r->_pc != _G(_unp)._depAdr : r->_pc >= _G(_unp)._retAdr)) {
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

				if (flipspe4 > (sizeof(fpressedchars) / sizeof(*fpressedchars)))
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

		_G(_iter)++;
 		if (_G(_iter) == iterMax) {
				return 0;
		}

		if (_G(_unp)._exoFnd && (_G(_unp)._endAdr == 0x10000) && (r->_pc >= 0x100) && (r->_pc <= 0x200) && (_G(_unp)._strMem != 2)) {
			_G(_unp)._endAdr = r->_mem[0xfe] + (r->_mem[0xff] << 8);
			if ((_G(_unp)._exoFnd & 0xff) == 0x30) { /* low byte of _endAdr, it's a lda $ff00,y */
				_G(_unp)._endAdr = (_G(_unp)._exoFnd >> 8) + (r->_mem[0xff] << 8);
			} else if ((_G(_unp)._exoFnd & 0xff) == 0x32) { /* add 1 */
				_G(_unp)._endAdr = 1 + ((_G(_unp)._exoFnd >> 8) + (r->_mem[0xff] << 8));
			}

			if (_G(_unp)._endAdr == 0)
				_G(_unp)._endAdr = 0x10001;
		}

		if (_G(_unp)._fEndBf && (_G(_unp)._endAdr == 0x10000) && (r->_pc == _G(_unp)._depAdr)) {
			_G(_unp)._endAdr = r->_mem[_G(_unp)._fEndBf] | r->_mem[_G(_unp)._fEndBf + 1] << 8;
			_G(_unp)._endAdr++;

			if (_G(_unp)._endAdr == 0)
					_G(_unp)._endAdr = 0x10001;

			_G(_unp)._fEndBf = 0;
		}

		if (_G(_unp)._fStrBf && (_G(_unp)._strMem != 0x2) && (r->_pc == _G(_unp)._depAdr)) {
			_G(_unp)._strMem = r->_mem[_G(_unp)._fStrBf] | r->_mem[_G(_unp)._fStrBf + 1] << 8;
			_G(_unp)._fStrBf = 0;
		}

		if (_G(_unp)._debugP) {
			for (p = 0; p < 0x20; p += 2) {
				if (*(unsigned short int *)(mem + 0x314 + p) != *(unsigned short int *)(vector + p)) {
					*(unsigned short int *)(vector + p) = *(unsigned short int *)(mem + 0x314 + p);
				}
			}
		}
	}

	_G(_iter) = 0;
	while (_G(_unp)._rtAFrc ? r->_pc != _G(_unp)._retAdr : r->_pc < _G(_unp)._retAdr) {
		if (_G(_unp)._monEnd && r->_pc == _G(_unp)._depAdr) {
			p = r->_mem[_G(_unp)._monEnd >> 16] | r->_mem[_G(_unp)._monEnd & 0xffff] << 8;
			if (p > (_G(_unp)._endAdr & 0xffff)) {
				_G(_unp)._endAdr = p;
			}
		}
		if (_G(_unp)._monStr && r->_pc == _G(_unp)._depAdr) {
			p = r->_mem[_G(_unp)._monStr >> 16] | r->_mem[_G(_unp)._monStr & 0xffff] << 8;
			if (p > 0) {
				if (_G(_unp)._mon1st == 0) {
					_G(_unp)._strMem = p;
				}
				_G(_unp)._mon1st = _G(_unp)._strMem;
				_G(_unp)._strMem = (p < _G(_unp)._strMem ? p : _G(_unp)._strMem);
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

		if ((mem[r->_pc] == 0x40) && (_G(_unp)._rtiFrc == 1)) {
			if (nextInst(r) == 1)
				return 0;
			_G(_unp)._retAdr = r->_pc;
			_G(_unp)._rtAFrc = 1;
			if (_G(_unp)._retAdr < _G(_unp)._strMem)
				_G(_unp)._strMem = 2;
			break;
		}

		_G(_iter)++;
		if (_G(_iter) == iterMax) {
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

	if (_G(_unp)._fEndAf && _G(_unp)._monEnd) {
		_G(_unp)._endAdC = mem[_G(_unp)._fEndAf] | mem[_G(_unp)._fEndAf + 1] << 8;
		if ((int)_G(_unp)._endAdC > _G(_unp)._endAdr)
			_G(_unp)._endAdr = _G(_unp)._endAdC;

		_G(_unp)._endAdC = 0;
		_G(_unp)._fEndAf = 0;
	}

	if (_G(_unp)._fEndAf && (_G(_unp)._endAdr == 0x10000)) {
		_G(_unp)._endAdr = r->_mem[_G(_unp)._fEndAf] | r->_mem[_G(_unp)._fEndAf + 1] << 8;
		if (_G(_unp)._endAdr == 0)
			_G(_unp)._endAdr = 0x10000;
		else
			_G(_unp)._endAdr++;
		_G(_unp)._fEndAf = 0;
	}

	if (_G(_unp)._fStrAf /*&&(_G(_unp)._strMem==0x800)*/) {
		_G(_unp)._strMem = r->_mem[_G(_unp)._fStrAf] | r->_mem[_G(_unp)._fStrAf + 1] << 8;
		_G(_unp)._strMem++;
		_G(_unp)._fStrAf = 0;
	}

	if (_G(_unp)._exoFnd && (_G(_unp)._strMem != 2)) {
		_G(_unp)._strMem = r->_mem[0xfe] + (r->_mem[0xff] << 8);

		if ((_G(_unp)._exoFnd & 0xff) == 0x30) {
			_G(_unp)._strMem += r->_y;
		} else if ((_G(_unp)._exoFnd & 0xff) == 0x32) {
			_G(_unp)._strMem += r->_y + 1;
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

	if (_G(_unp)._wrMemF) {
		_G(_unp)._wrMemF = 0;
		for (p = 0x800; p < 0x10000; p += 4) {
			if (*(unsigned int *)(oldmem + p) == *(unsigned int *)(mem + p)) {
				*(unsigned int *)(mem + p) = 0;
				_G(_unp)._wrMemF = 1;
			}
		}
		/* clean also the $fd30 table copy in RAM */
		if (memcmp(mem + 0xfd30, vector, sizeof(vector)) == 0) {
			memset(mem + 0xfd30, 0, sizeof(vector));
		}
	}

	if (_G(_unp)._lfMemF) {
		for (p = 0xffff; p > 0x0800; p--) {
			if (oldmem[--_G(_unp)._lfMemF] == mem[p])
				mem[p] = 0x0;
			else {
				if (p >= 0xffff)
					_G(_unp)._lfMemF = 0 | _G(_unp)._ecaFlg;
				break;
			}
		}
	}

	if (*forcedname) {
		strcpy(name, forcedname);
	} else {
		if (strlen(name) > 248) /* dirty hack in case name is REALLY long */
			name[248] = 0;
		sprintf(name + strlen(name), ".%04x%s", r->_pc, ((_G(_unp)._wrMemF | _G(_unp)._lfMemF) ? ".clean" : ""));
	}

	/*  endadr is set to a ZP location? then use it as a pointer
	  todo: use __fEndAf instead, it can be used for any location, not only ZP. */
	if (_G(_unp)._endAdr && (_G(_unp)._endAdr < 0x100)) {
		p = (mem[_G(_unp)._endAdr] | mem[_G(_unp)._endAdr + 1] << 8) & 0xffff;
		_G(_unp)._endAdr = p;
	}

	if (_G(_unp)._ecaFlg && (_G(_unp)._strMem != 2)) /* checkme */ {
		if (_G(_unp)._endAdr >= ((_G(_unp)._ecaFlg >> 16) & 0xffff)) {
			/* most of the times transfers $2000 byte from $d000-efff to $e000-ffff but there are exceptions */
			if (_G(_unp)._lfMemF)
				memset(mem + ((_G(_unp)._ecaFlg >> 16) & 0xffff), 0, 0x1000);
			_G(_unp)._endAdr += 0x1000;
		}
	}

	if (_G(_unp)._endAdr <= 0)
		_G(_unp)._endAdr = 0x10000;

	if (_G(_unp)._endAdr > 0x10000)
		_G(_unp)._endAdr = 0x10000;

	if (_G(_unp)._endAdr < _G(_unp)._strMem)
		_G(_unp)._endAdr = 0x10000;

	if (_G(_unp)._endAdC & 0xffff) {
		_G(_unp)._endAdr += (_G(_unp)._endAdC & 0xffff);
		_G(_unp)._endAdr &= 0xffff;
	}

	if (_G(_unp)._endAdC & EA_USE_A) {
		_G(_unp)._endAdr += r->_a;
		_G(_unp)._endAdr &= 0xffff;
	}

	if (_G(_unp)._endAdC & EA_USE_X) {
		_G(_unp)._endAdr += r->_x;
		_G(_unp)._endAdr &= 0xffff;
	}

	if (_G(_unp)._endAdC & EA_USE_Y) {
		_G(_unp)._endAdr += r->_y;
		_G(_unp)._endAdr &= 0xffff;
	}

	if (_G(_unp)._strAdC & 0xffff) {
		_G(_unp)._strMem += (_G(_unp)._strAdC & 0xffff);
		_G(_unp)._strMem &= 0xffff;
		/* only if ea_addff, no reg involved */
		if (((_G(_unp)._strAdC & 0xffff0000) == EA_ADDFF) && ((_G(_unp)._strMem & 0xff) == 0)) {
			_G(_unp)._strMem += 0x100;
			_G(_unp)._strMem &= 0xffff;
		}
	}

	if (_G(_unp)._strAdC & EA_USE_A) {
		_G(_unp)._strMem += r->_a;
		_G(_unp)._strMem &= 0xffff;
		if (_G(_unp)._strAdC & EA_ADDFF) {
			if ((_G(_unp)._strMem & 0xff) == 0xff)
				_G(_unp)._strMem++;

			if (r->_a == 0) {
				_G(_unp)._strMem += 0x100;
				_G(_unp)._strMem &= 0xffff;
			}
		}
	}

	if (_G(_unp)._strAdC & EA_USE_X) {
		_G(_unp)._strMem += r->_x;
		_G(_unp)._strMem &= 0xffff;

		if (_G(_unp)._strAdC & EA_ADDFF) {
			if ((_G(_unp)._strMem & 0xff) == 0xff)
				_G(_unp)._strMem++;

			if (r->_x == 0) {
				_G(_unp)._strMem += 0x100;
				_G(_unp)._strMem &= 0xffff;
			}
		}
	}

	if (_G(_unp)._strAdC & EA_USE_Y) {
		_G(_unp)._strMem += r->_y;
		_G(_unp)._strMem &= 0xffff;

		if (_G(_unp)._strAdC & EA_ADDFF) {
			if ((_G(_unp)._strMem & 0xff) == 0xff)
				_G(_unp)._strMem++;

			if (r->_y == 0) {
				_G(_unp)._strMem += 0x100;
				_G(_unp)._strMem &= 0xffff;
			}
		}
	}

	if (_G(_unp)._endAdr <= 0)
		_G(_unp)._endAdr = 0x10000;

	if (_G(_unp)._endAdr > 0x10000)
		_G(_unp)._endAdr = 0x10000;

	if (_G(_unp)._endAdr < _G(_unp)._strMem)
		_G(_unp)._endAdr = 0x10000;

	mem[_G(_unp)._strMem - 2] = _G(_unp)._strMem & 0xff;
	mem[_G(_unp)._strMem - 1] = _G(_unp)._strMem >> 8;

	memcpy(destinationBuffer, mem + (_G(_unp)._strMem - 2), _G(_unp)._endAdr - _G(_unp)._strMem + 2);
	*finalLength = _G(_unp)._endAdr - _G(_unp)._strMem + 2;

	if (_G(_unp)._recurs) {
		if (++_G(_unp)._recurs > RECUMAX)
			return 1;
		reinitUnp();
		goto looprecurse;
	}
	return 1;
}

} // End of namespace Scott
} // End of namespace Glk
