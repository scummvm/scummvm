/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "sky/compact.h"

#include "sky/compacts/0compact.h"
#include "sky/compacts/1compact.h"
#include "sky/compacts/29comp.h"
#include "sky/compacts/2compact.h"
#include "sky/compacts/30comp.h"
#include "sky/compacts/3compact.h"
#include "sky/compacts/4compact.h"
#include "sky/compacts/5compact.h"
#include "sky/compacts/66comp.h"
#include "sky/compacts/90comp.h"
#include "sky/compacts/9compact.h"
#include "sky/compacts/linc_gen.h"
#include "sky/compacts/lincmenu.h"
#include "sky/compacts/z_compac.h"

#define COMPACT_SIZE (sizeof(compactOffsets)/sizeof(uint32))
#define EXTCOMPACT_SIZE (sizeof(extCompactOffsets)/sizeof(uint32))
#define UNKNOWNSTUFF_SIZE (sizeof(unknownStuffOffsets)/sizeof(uint32))

#define OFFS(type,item) (((uint32)(&((type*)0)->item)))
#define MK32(type,item) OFFS(type, item),0,0,0
#define MK16(type,item) OFFS(type, item),0

namespace SkyCompact {

static const uint32 compactOffsets[] = {
	MK16(Compact, logic),
	MK16(Compact, status),
	MK16(Compact, sync),
	MK16(Compact, screen),
	MK16(Compact, place),
	MK32(Compact, getToTable),
	MK16(Compact, xcood),
	MK16(Compact, ycood),
	MK16(Compact, frame),
	MK16(Compact, cursorText),
	MK16(Compact, mouseOn),
	MK16(Compact, mouseOff),
	MK16(Compact, mouseClick),
	MK16(Compact, mouseRelX),
	MK16(Compact, mouseRelY),
	MK16(Compact, mouseSizeX),
	MK16(Compact, mouseSizeY),
	MK16(Compact, actionScript),
	MK16(Compact, upFlag),
	MK16(Compact, downFlag),
	MK16(Compact, getToFlag),
	MK16(Compact, flag),
	MK16(Compact, mood),
	MK32(Compact, grafixProg),
	MK16(Compact, offset),
	MK16(Compact, mode),
	MK16(Compact, baseSub),
	MK16(Compact, baseSub_off),
};

static const uint32 extCompactOffsets[] = {
	MK16(ExtCompact, actionSub),
	MK16(ExtCompact, actionSub_off),
	MK16(ExtCompact, getToSub),
	MK16(ExtCompact, getToSub_off),
	MK16(ExtCompact, extraSub),
	MK16(ExtCompact, extraSub_off),
	MK16(ExtCompact, dir),
	MK16(ExtCompact, stopScript),
	MK16(ExtCompact, miniBump),
	MK16(ExtCompact, leaving),
	MK16(ExtCompact, atWatch),
	MK16(ExtCompact, atWas),
	MK16(ExtCompact, alt),
	MK16(ExtCompact, request),
	MK16(ExtCompact, spWidth_xx),
	MK16(ExtCompact, spColour),
	MK16(ExtCompact, spTextId),
	MK16(ExtCompact, spTime),
	MK16(ExtCompact, arAnimIndex),
	MK32(ExtCompact, turnProg),
	MK16(ExtCompact, waitingFor),
	MK16(ExtCompact, arTargetX),
	MK16(ExtCompact, arTargetY),
	MK32(ExtCompact, animScratch),
	MK16(ExtCompact, megaSet),
	MK16(ExtCompact, gridWidth),
	MK16(ExtCompact, colOffset),
	MK16(ExtCompact, colWidth),
	MK16(ExtCompact, lastChr),
	MK32(ExtCompact, animUp),
	MK32(ExtCompact, animDown),
	MK32(ExtCompact, animLeft),
	MK32(ExtCompact, animRight),
	MK32(ExtCompact, standUp),
	MK32(ExtCompact, standDown),
	MK32(ExtCompact, standLeft),
	MK32(ExtCompact, standRight),
	MK32(ExtCompact, standTalk),
};

static const uint32 unknownStuffOffsets[] = {
	MK16(UnknownStuff, unknownStuff_1_1[0]),
	MK16(UnknownStuff, unknownStuff_1_1[1]),
	MK16(UnknownStuff, unknownStuff_1_1[2]),
	MK16(UnknownStuff, unknownStuff_1_1[3]),
	MK32(UnknownStuff, unknownStuff_1_2[0]),
	MK32(UnknownStuff, unknownStuff_1_2[1]),
	MK32(UnknownStuff, unknownStuff_1_2[2]),
	MK32(UnknownStuff, unknownStuff_1_2[3]),
	MK32(UnknownStuff, unknownStuff_1_2[4]),
	MK32(UnknownStuff, unknownStuff_1_2[5]),
	MK32(UnknownStuff, unknownStuff_1_2[6]),
	MK32(UnknownStuff, unknownStuff_1_2[7]),
	MK32(UnknownStuff, unknownStuff_1_2[8]),
	MK32(UnknownStuff, unknownStuff_1_2[9]),
	MK32(UnknownStuff, unknownStuff_1_2[10]),
	MK32(UnknownStuff, unknownStuff_1_2[11]),
	MK32(UnknownStuff, unknownStuff_1_2[12]),
	MK32(UnknownStuff, unknownStuff_1_2[13]),
	MK32(UnknownStuff, unknownStuff_1_2[14]),
	MK32(UnknownStuff, unknownStuff_1_2[15]),
	MK32(UnknownStuff, unknownStuff_1_2[16]),
	MK32(UnknownStuff, unknownStuff_1_2[17]),
	MK32(UnknownStuff, unknownStuff_1_2[18]),
	MK32(UnknownStuff, unknownStuff_1_2[19]),
	MK32(UnknownStuff, unknownStuff_1_2[20]),
	MK32(UnknownStuff, unknownStuff_1_2[21]),
	MK32(UnknownStuff, unknownStuff_1_2[22]),
	MK32(UnknownStuff, unknownStuff_1_2[23]),
	MK32(UnknownStuff, unknownStuff_1_2[24]),
	MK32(UnknownStuff, unknownStuff_1_2[25]),
	MK32(UnknownStuff, unknownStuff_1_2[26]),
	MK32(UnknownStuff, unknownStuff_1_2[27]),
	MK32(UnknownStuff, unknownStuff_1_2[28]),
	MK32(UnknownStuff, unknownStuff_1_2[29]),
	MK32(UnknownStuff, unknownStuff_1_2[30]),
	MK32(UnknownStuff, unknownStuff_1_2[31]),
	MK32(UnknownStuff, unknownStuff_1_2[32]),
	MK32(UnknownStuff, unknownStuff_1_2[33]),
	MK16(UnknownStuff, unknownStuff_2_1[0]),
	MK16(UnknownStuff, unknownStuff_2_1[1]),
	MK16(UnknownStuff, unknownStuff_2_1[2]),
	MK16(UnknownStuff, unknownStuff_2_1[3]),
	MK32(UnknownStuff, unknownStuff_2_2[0]),
	MK32(UnknownStuff, unknownStuff_2_2[1]),
	MK32(UnknownStuff, unknownStuff_2_2[2]),
	MK32(UnknownStuff, unknownStuff_2_2[3]),
	MK32(UnknownStuff, unknownStuff_2_2[4]),
	MK32(UnknownStuff, unknownStuff_2_2[5]),
	MK32(UnknownStuff, unknownStuff_2_2[6]),
	MK32(UnknownStuff, unknownStuff_2_2[7]),
	MK32(UnknownStuff, unknownStuff_2_2[8]),
	MK32(UnknownStuff, unknownStuff_2_2[9]),
	MK32(UnknownStuff, unknownStuff_2_2[10]),
	MK32(UnknownStuff, unknownStuff_2_2[11]),
	MK32(UnknownStuff, unknownStuff_2_2[12]),
	MK32(UnknownStuff, unknownStuff_2_2[13]),
	MK32(UnknownStuff, unknownStuff_2_2[14]),
	MK32(UnknownStuff, unknownStuff_2_2[15]),
	MK32(UnknownStuff, unknownStuff_2_2[16]),
	MK32(UnknownStuff, unknownStuff_2_2[17]),
	MK32(UnknownStuff, unknownStuff_2_2[18]),
	MK32(UnknownStuff, unknownStuff_2_2[19]),
	MK32(UnknownStuff, unknownStuff_2_2[20]),
	MK32(UnknownStuff, unknownStuff_2_2[21]),
	MK32(UnknownStuff, unknownStuff_2_2[22]),
	MK32(UnknownStuff, unknownStuff_2_2[23]),
	MK32(UnknownStuff, unknownStuff_2_2[24]),
	MK32(UnknownStuff, unknownStuff_2_2[25]),
	MK32(UnknownStuff, unknownStuff_2_2[26]),
	MK32(UnknownStuff, unknownStuff_2_2[27]),
	MK32(UnknownStuff, unknownStuff_2_2[28]),
	MK32(UnknownStuff, unknownStuff_2_2[29]),
	MK32(UnknownStuff, unknownStuff_2_2[30]),
	MK32(UnknownStuff, unknownStuff_2_2[31]),
	MK32(UnknownStuff, unknownStuff_2_2[32]),
	MK32(UnknownStuff, unknownStuff_2_2[33]),
	MK16(UnknownStuff, unknownStuff_3_1[0]),
	MK16(UnknownStuff, unknownStuff_3_1[1]),
	MK16(UnknownStuff, unknownStuff_3_1[2]),
	MK16(UnknownStuff, unknownStuff_3_1[3]),
	MK32(UnknownStuff, unknownStuff_3_2[0]),
	MK32(UnknownStuff, unknownStuff_3_2[1]),
	MK32(UnknownStuff, unknownStuff_3_2[2]),
	MK32(UnknownStuff, unknownStuff_3_2[3]),
	MK32(UnknownStuff, unknownStuff_3_2[4]),
	MK32(UnknownStuff, unknownStuff_3_2[5]),
	MK32(UnknownStuff, unknownStuff_3_2[6]),
	MK32(UnknownStuff, unknownStuff_3_2[7]),
	MK32(UnknownStuff, unknownStuff_3_2[8]),
	MK32(UnknownStuff, unknownStuff_3_2[9]),
	MK32(UnknownStuff, unknownStuff_3_2[10]),
	MK32(UnknownStuff, unknownStuff_3_2[11]),
	MK32(UnknownStuff, unknownStuff_3_2[12]),
	MK32(UnknownStuff, unknownStuff_3_2[13]),
	MK32(UnknownStuff, unknownStuff_3_2[14]),
	MK32(UnknownStuff, unknownStuff_3_2[15]),
	MK32(UnknownStuff, unknownStuff_3_2[16]),
	MK32(UnknownStuff, unknownStuff_3_2[17]),
	MK32(UnknownStuff, unknownStuff_3_2[18]),
	MK32(UnknownStuff, unknownStuff_3_2[19]),
	MK32(UnknownStuff, unknownStuff_3_2[20]),
	MK32(UnknownStuff, unknownStuff_3_2[21]),
	MK32(UnknownStuff, unknownStuff_3_2[22]),
	MK32(UnknownStuff, unknownStuff_3_2[23]),
	MK32(UnknownStuff, unknownStuff_3_2[24]),
	MK32(UnknownStuff, unknownStuff_3_2[25]),
	MK32(UnknownStuff, unknownStuff_3_2[26]),
	MK32(UnknownStuff, unknownStuff_3_2[27]),
	MK32(UnknownStuff, unknownStuff_3_2[28]),
	MK32(UnknownStuff, unknownStuff_3_2[29]),
	MK32(UnknownStuff, unknownStuff_3_2[30]),
	MK32(UnknownStuff, unknownStuff_3_2[31]),
	MK32(UnknownStuff, unknownStuff_3_2[32]),
	MK32(UnknownStuff, unknownStuff_3_2[33]),
};

void *getCompactElem(Compact *cpt, uint32 off) {
	if (off < COMPACT_SIZE)
		return((uint8 *)cpt + compactOffsets[off]);

	off -= COMPACT_SIZE;
	if (off < EXTCOMPACT_SIZE)
		return((uint8 *)(cpt->extCompact) + extCompactOffsets[off]);

	off -= EXTCOMPACT_SIZE;
	if (off < 5*5*4)
		return ((void **)(cpt->turnTable))[off/4];

	off -= 5*5*4;
	if (off < UNKNOWNSTUFF_SIZE)
		return((uint8 *)(cpt->unknownStuff) + unknownStuffOffsets[off]);

	error("Offset %X out of bounds of compact", off + COMPACT_SIZE + EXTCOMPACT_SIZE + 5*5*4);
}
};

