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

//#include "src\driver96.h"
#include "memory.h"
#include "object.h"


typedef	struct _walkData
{
	uint16	frame;
	int16	x;
	int16	y;
	uint8	step;
	uint8	dir;
} _walkData;




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
uint8 CheckForCollision(void);

//--------------------------------------------------------------------------------------


#endif
