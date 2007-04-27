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

#ifndef CRUISE_BACKGROUNDINCRUST_H
#define CRUISE_BACKGROUNDINCRUST_H

namespace Cruise {

struct backgroundIncrustStruct {
	struct backgroundIncrustStruct *next;
	struct backgroundIncrustStruct *prev;

	uint16 objectIdx;
	uint16 field_6;
	uint16 overlayIdx;
	uint16 X;
	uint16 Y;
	uint16 field_E;
	uint16 scale;
	uint16 backgroundIdx;
	uint16 scriptNumber;
	uint16 scriptOverlayIdx;
	uint8 *ptr;
	int32 field_1C;
	int16 size;
	uint16 field_22;
	uint16 field_24;
	char name[14];
	uint16 var34;
};

typedef struct backgroundIncrustStruct backgroundIncrustStruct;

extern backgroundIncrustStruct backgroundIncrustHead;

void resetBackgroundIncrustList(backgroundIncrustStruct * pHead);
backgroundIncrustStruct *addBackgroundIncrust(int16 overlayIdx, int16 param2,
    backgroundIncrustStruct * pHead, int16 scriptNumber, int16 scriptOverlay,
    int16 backgroundIdx, int16 param4);
void loadBackgroundIncrustFromSave(FILE * fileHandle);
void regenerateBackgroundIncrust(backgroundIncrustStruct * pHead);
void freeBackgroundIncrustList(backgroundIncrustStruct * pHead);

} // End of namespace Cruise

#endif
