
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

#ifndef M4_WSCRIPT_WS_CRUNCHER_H
#define M4_WSCRIPT_WS_CRUNCHER_H

#include "m4/m4_types.h"
#include "m4/wscript/ws_machine.h"

namespace M4 {

#define OP_INSTR			0xfe000000
#define OP_FORMAT1			0x01c00000
#define OP_FORMAT2			0x00380000
#define OP_FORMAT3			0x00070000
#define OP_HIGH_DATA		0xffff0000
#define OP_LOW_DATA			0x0000ffff
#define OP_DATA_SIGN		0x8000
#define OP_DATA_VALUE		0x7fff

#define FMT_NOTHING			0
#define FMT_LOCAL_SRC		1
#define FMT_GLOBAL_SRC		2
#define FMT_INT15			3
#define FMT_4_11			4
#define FMT_7_8				5
#define FMT_11_4			6
#define FMT_15_0			7
#define REG_SET_IDX			0x0fff
#define REG_SET_IDX_REG		0x8000
#define LOCAL_FMT			0x7000
#define LOCAL_FMT_PARENT	0x0000
#define LOCAL_FMT_REG		0x1000
#define LOCAL_FMT_DATA		0x2000

#define BRANCH_BR			0
#define BRANCH_BLT			1
#define BRANCH_BLE			2
#define BRANCH_BE			3
#define BRANCH_BNE			4
#define BRANCH_BGE			5
#define BRANCH_BGT			6

struct EOSreq {
	EOSreq *next = nullptr;
	EOSreq *prev = nullptr;
	Anim8 *myAnim8 = nullptr;
};

struct cruncher {
	Anim8 *backLayerAnim8;
	Anim8 *frontLayerAnim8;
	Anim8 *firstAnim8ToCrunch;
	Anim8 *lastAnim8ToCrunch;
};

struct WSCruncher_Globals {
	cruncher *_myCruncher = nullptr;
	bool _cruncherInitialized = false;
	EOSreq *_EOSreqList = nullptr;
	int32 _memtypeEOS = -1;

	int32 _stackSize = 0;
	uint32 *_stackBase = nullptr;
	uint32 *_stackTop = nullptr;
	uint32 *_stackLimit = nullptr;
	Anim8 *_crunchNext = nullptr;

	// OPCODE PROCESSING GLOBALS
	int16 *_myDepthTable = nullptr;

	frac16 _dataArg1 = 0;
	frac16 _dataArg2 = 0;
	frac16 _dataArg3 = 0;
	frac16 *_myArg1 = nullptr;
	frac16 *_myArg2 = nullptr;
	frac16 *_myArg3 = nullptr;

	bool _keepProcessing = false, _terminated = false, _mapTheCel = false;
	bool _bailOut = false;
	int32 _compareCCR = 0;
	int32 _indexReg = 0;
	int32 _pcOffsetOld = 0;
};

int32 *ws_GetDataFormats();
bool ws_InitCruncher();
void ws_KillCruncher();

Anim8 *ws_AddAnim8ToCruncher(machine *m, int32 sequHash);

/**
 * This procedure assumes a machine has a slot with it's own memory
 */
bool ws_ChangeAnim8Program(machine *m, int32 newSequHash);

/**
 * This procedure flags the anim8 slot as empty
 */
void ws_RemoveAnim8FromCruncher(Anim8 *myAnim8);

bool ws_PauseAnim8(Anim8 *myAnim8);
bool ws_ResumeAnim8(Anim8 *myAnim8);

/**
 * Pre-processes a pcode instruction parameters.
 * The instruction number is returned by this function, and the arguments are
 * pointed to by these external globals:
 * Frac16 *myArg1
 * Frac16 *myArg2
 * Frac16 *myArg3
 */
int32 ws_PreProcessPcode(uint32 **PC, Anim8 *myAnim8);

void ws_CrunchAnim8s(int16 *depth_table);
void ws_CrunchEOSreqs();
bool ws_OnEndSeqRequest(Anim8 *myAnim8, int32 pcOffset, int32 pcCount);
void ws_CancelOnEndSeq(Anim8 *myAnim8);

} // End of namespace M4

#endif
