/* Copyright (C) 1994-2003 Revolution Software Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef _ROUTER_H
#define _ROUTER_H

#include "memory.h"
#include "object.h"

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

typedef	struct _walkData {
	uint16 frame;
	int16 x;
	int16 y;
	uint8 step;
	uint8 dir;
} GCC_PACK _walkData;

typedef struct {
	int16 x1;
  	int16 y1;
  	int16 x2;
	int16 y2;
	int16 xmin;
	int16 ymin;
	int16 xmax;
	int16 ymax;
	int16 dx;	// x2 - x1
	int16 dy;	// y2 - y1
	int32 co;	// co = (y1 *dx)- (x1*dy) from an equation for a line y*dx = x*dy + co
} GCC_PACK _barData;

typedef struct {
	int16 x;
	int16 y;
	int16 level;
	int16 prev;
	int16 dist;
} GCC_PACK _nodeData;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

int32 RouteFinder(Object_mega *ob_mega, Object_walkdata *ob_walkdata, int32 x, int32 y, int32 dir);

void EarlySlowOut(Object_mega *ob_mega, Object_walkdata *ob_walkdata);

void AllocateRouteMem(void);
_walkData* LockRouteMem(void);
void FloatRouteMem(void);
void FreeRouteMem(void);
void FreeAllRouteMem(void);
void PlotWalkGrid(void);
void AddWalkGrid(int32 gridResource);
void RemoveWalkGrid(int32 gridResource);
void ClearWalkGridList(void);

#endif
