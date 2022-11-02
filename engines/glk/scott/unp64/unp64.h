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

#ifndef GLK_SCOTT_UNP64_H
#define GLK_SCOTT_UNP64_H

namespace Glk {
namespace Scott {

struct LoadInfo;
struct CpuCtx;

struct UnpStr {
	int _idFlag;          /* flag, 1=packer identified; 2=not a packer, stop scanning */
	int _forced;          /* forced entry point */
	int _strMem;          /* start of unpacked memory */
	int _retAdr;          /* return address after unpacking */
	int _depAdr;          /* unpacker entry point */
	int _endAdr;          /* end of unpacked memory */
	int _rtAFrc;          /* flag, return address must be exactly RetAdr, else anything >= RetAdr */
	int _wrMemF;          /* flag, clean unwritten memory */
	int _lfMemF;          /* flag, clean end memory leftovers */
	int _exoFnd;          /* flag, Exomizer detected */ 
	int _fStack;          /* flag, fill stack with 0 and SP=$ff, else as in C64 */
	int _ecaFlg;          /* ECA found, holds relocated areas high bytes */
	int _fEndBf;          /* End memory address pointer before unpacking, set when DepAdr is reached */
	int _fEndAf;          /* End memory address pointer after  unpacking, set when RetAdr is reached */
	int _fStrBf;          /* Start memory address pointer before unpacking, set when DepAdr is reached */
	int _fStrAf;          /* Start memory address pointer after  unpacking, set when RetAdr is reached */
	int _idOnly;          /* flag, just identify packer and exit */
	int _debugP;          /* flag, verbosely emit various infos */
	int _rtiFrc;          /* flag, RTI instruction forces return from unpacker */
	int _recurs;          /* recursion counter */
	unsigned int _monEnd;	/* End memory address pointers monitored during execution, updated every time DepAdr is reached */
	unsigned int _monStr;	/* Start memory address pointers monitored during execution, updated every time DepAdr is reached */
	unsigned int _mon1st;	/* flag for forcingly assign monitored str/end ptr the 1st time */
	unsigned int _endAdC;	/* add fixed values and/or registers AXY to End memory address */
	unsigned int _strAdC;	/* add fixed values and/or registers AXY to Start memory address */
	unsigned int _filler;	/* Memory filler byte*/
	unsigned char *_mem;    /* pointer to the memory array */
	char *_name;            /* name of the prg file */
	LoadInfo *_info; /* pointer to the loaded prg info struct */
	CpuCtx *_r;      /* pointer to the registers struct */
};

typedef void (*Scnptr)(UnpStr *);

#define EA_USE_A 0x01000000
#define EA_USE_X 0x00100000
#define EA_USE_Y 0x00010000
#define EA_ADDFF 0x10000000
#define ITERMAX 0x02000000
#define RECUMAX 16

void scanners(UnpStr *);

} // End of namespace Scott
} // End of namespace Glk

#endif 
