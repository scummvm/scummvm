
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
	int32 _compareCCR = 0;
	int32 _indexReg = 0;
	int32 _pcOffsetOld = 0;
};

extern int32 *ws_GetDataFormats();

extern bool ws_InitCruncher();
extern void ws_KillCruncher();

} // End of namespace M4

#endif
