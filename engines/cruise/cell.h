/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef CRUISE_CELL_H
#define CRUISE_CELL_H

#include "common/stdafx.h"
#include "common/scummsys.h"

namespace Cruise {

struct gfxEntryStruct;

struct cellStruct {
	struct cellStruct *next;
	struct cellStruct *prev;
	int16 idx;
	int16 type;
	int16 overlay;
	int16 field_A;
	int16 field_C;
	int16 spriteIdx;
	int16 field_10;
	int16 backgroundPlane;
	int16 freeze;
	int16 field_16;
	int16 field_18;
	int16 field_1A;
	int16 followObjectOverlayIdx;
	int16 followObjectIdx;
	int16 field_20;
	int16 field_22;
	int16 nextAnimDelay;
	int16 animStep;
	int16 field_28;
	int16 field_2A;
	int16 field_2C;
	int16 currentAnimDelay;
	int16 field_30;
	gfxEntryStruct *gfxPtr;
};

extern cellStruct cellHead;

void resetPtr(cellStruct * ptr);
void loadSavegameDataSub2(FILE * f);
cellStruct *addCell(int16 overlayIdx, int16 param2, cellStruct * pHead,
    int16 scriptType, int16 scriptNumber, int16 scriptOverlay, int16 param3,
    int16 param4);

} // End of namespace Cruise

#endif
