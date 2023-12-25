
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

#ifndef M4_GUI_GUI_VMNG_RECTANGLES_H
#define M4_GUI_GUI_VMNG_RECTANGLES_H

#include "m4/m4_types.h"
#include "m4/gui/gui_univ.h"

namespace M4 {

RectList *vmng_CreateNewRect(int32 x1, int32 y1, int32 x2, int32 y2);
void vmng_AddRectToRectList(RectList **scrnRectList, int32 x1, int32 y1, int32 x2, int32 y2);
RectList *vmng_DuplicateRectList(RectList *myRectList);
bool vmng_RectIntersectsRectList(RectList *myRectList, int32 x1, int32 y1, int32 x2, int32 y2);
bool vmng_RectListValid(RectList *myRectList);
bool vmng_ClipRectList(RectList **myRectList, int32 clipX1, int32 clipY1, int32 clipX2, int32 clipY2);
void vmng_DisposeRectList(RectList **rectList);
void vmng_RemoveRectFromRectList(RectList **scrnRectList, int32 x1, int32 y1, int32 x2, int32 y2);

} // End of namespace M4

#endif
