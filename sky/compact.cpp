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
#define MEGASET_SIZE (sizeof(megaSetOffsets)/sizeof(uint32))
#define TURNTABLE_SIZE (sizeof(turnTableOffsets)/sizeof(uint32))

#define OFFS(type,item) (((uint32)(&((type*)0)->item)))
#define MK32(type,item) OFFS(type, item),0,0,0
#define MK16(type,item) OFFS(type, item),0
#define MK32_A5(type, item) MK32(type, item[0]), MK32(type, item[1]), \
	MK32(type, item[2]), MK32(type, item[3]), MK32(type, item[4])

namespace SkyCompact {

uint16 *getSub(Compact *cpt, uint16 mode) {
	switch (mode) {
	case 0:
		return &(cpt->baseSub);
	case 2:
		return &(cpt->baseSub_off);
	case 4:
		return &(cpt->extCompact->actionSub);
	case 6:
		return &(cpt->extCompact->actionSub_off);
	case 8:
		return &(cpt->extCompact->getToSub);
	case 10:
		return &(cpt->extCompact->getToSub_off);
	case 12:
		return &(cpt->extCompact->extraSub);
	case 14:
		return &(cpt->extCompact->extraSub_off);
	default:
		error("Invalid Mode (%d)", mode);
	}
}

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
};

static const uint32 megaSetOffsets[] = {
	MK16(MegaSet, gridWidth),
	MK16(MegaSet, colOffset),
	MK16(MegaSet, colWidth),
	MK16(MegaSet, lastChr),
	MK32(MegaSet, animUp),
	MK32(MegaSet, animDown),
	MK32(MegaSet, animLeft),
	MK32(MegaSet, animRight),
	MK32(MegaSet, standUp),
	MK32(MegaSet, standDown),
	MK32(MegaSet, standLeft),
	MK32(MegaSet, standRight),
	MK32(MegaSet, standTalk),
};

static const uint32 turnTableOffsets[] = {
	MK32_A5(TurnTable, turnTableUp),
	MK32_A5(TurnTable, turnTableDown),
	MK32_A5(TurnTable, turnTableLeft),
	MK32_A5(TurnTable, turnTableRight),
	MK32_A5(TurnTable, turnTableTalk),
};

void *getCompactElem(Compact *cpt, uint32 off) {
	if (off < COMPACT_SIZE)
		return((uint8 *)cpt + compactOffsets[off]);

	off -= COMPACT_SIZE;
	if (off < EXTCOMPACT_SIZE)
		return((uint8 *)(cpt->extCompact) + extCompactOffsets[off]);

	off -= EXTCOMPACT_SIZE;
	if (off < MEGASET_SIZE)
		return((uint8 *)(cpt->extCompact->megaSet0) + megaSetOffsets[off]);

	off -= MEGASET_SIZE;
	if (off < TURNTABLE_SIZE)
		return ((uint8 *)(cpt->extCompact->megaSet0->turnTable) + turnTableOffsets[off]);

	off -= TURNTABLE_SIZE;
	if (off < MEGASET_SIZE)
		return((uint8 *)(cpt->extCompact->megaSet1) + megaSetOffsets[off]);

	off -= MEGASET_SIZE;
	if (off < TURNTABLE_SIZE)
		return ((uint8 *)(cpt->extCompact->megaSet1->turnTable) + turnTableOffsets[off]);

	off -= TURNTABLE_SIZE;
	if (off < MEGASET_SIZE)
		return((uint8 *)(cpt->extCompact->megaSet2) + megaSetOffsets[off]);

	off -= MEGASET_SIZE;
	if (off < TURNTABLE_SIZE)
		return ((uint8 *)(cpt->extCompact->megaSet2->turnTable) + turnTableOffsets[off]);

	off -= TURNTABLE_SIZE;
	if (off < MEGASET_SIZE)
		return((uint8 *)(cpt->extCompact->megaSet3) + megaSetOffsets[off]);

	off -= MEGASET_SIZE;
	if (off < TURNTABLE_SIZE)
		return ((uint8 *)(cpt->extCompact->megaSet3->turnTable) + turnTableOffsets[off]);
	off -= TURNTABLE_SIZE;

	error("Offset %X out of bounds of compact", off + COMPACT_SIZE + EXTCOMPACT_SIZE + 4 * MEGASET_SIZE + 4 * TURNTABLE_SIZE);
}
};

