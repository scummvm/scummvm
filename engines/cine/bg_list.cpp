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

#include "cine/cine.h"

void createVar9Element(s16 objIdx, s16 param);

void addSpriteFilledToBGList(s16 idx) {
	s16 x;
	s16 y;
	s16 width;
	s16 height;

	x = objectTable[idx].x;
	y = objectTable[idx].y;

	width = animDataTable[objectTable[idx].frame].width;
	height = animDataTable[objectTable[idx].frame].var2;

	if (animDataTable[objectTable[idx].frame].ptr1) {
		gfxFillSprite(animDataTable[objectTable[idx].frame].ptr1,
		    width / 2, height, page2Raw, x, y);
	}

	createVar9Element(idx, 1);
}
