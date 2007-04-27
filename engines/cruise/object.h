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

#ifndef CRUISE_OBJECT_H
#define CRUISE_OBJECT_H

namespace Cruise {

struct gfxEntryStruct {
	uint8 *imagePtr;
	int imageSize;
	int fontIndex;
	int height;
	int width;		// for font: max right border; for sprite: just width
};

typedef struct gfxEntryStruct gfxEntryStruct;

#define	OBJ_SPRITE 4

struct objectParamsQuery {
	int16 X;
	int16 Y;
	int16 baseFileIdx;
	int16 fileIdx;
	int16 scale;
	int16 var5;
	int16 var6;
	int16 var7;
};

typedef struct objectParamsQuery objectParamsQuery;

objDataStruct *getObjectDataFromOverlay(int ovlIdx, int objIdx);
int16 getSingleObjectParam(int16 overlayIdx, int16 param2, int16 param3,
    int16 * returnParam);
int16 getMultipleObjectParam(int16 overlayIdx, int16 objectIdx,
    objectParamsQuery * returnParam);

} // End of namespace Cruise

#endif
