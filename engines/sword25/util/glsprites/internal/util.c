/******************************************************************************/
/* This file is part of Broken Sword 2.5                                      */
/* Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer     */
/*                                                                            */
/* Broken Sword 2.5 is free software; you can redistribute it and/or modify   */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* Broken Sword 2.5 is distributed in the hope that it will be useful,        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               */
/* GNU General Public License for more details.                               */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with Broken Sword 2.5; if not, write to the Free Software            */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA   */
/******************************************************************************/

/* --------------------------------------------------------------------------
   INCLUDES
   -------------------------------------------------------------------------- */

#include "util.h"


/* -------------------------------------------------------------------------- */

GLS_Bool GLS_IsPowerOf2(GLS_UInt32 value)
{
	return !(value & (value - 1)) && value;
}


/* -------------------------------------------------------------------------- */

GLS_UInt32 GLS_NextPowerOf2(GLS_UInt32 value)
{
	--value;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	++value;

	return value;
}

/* -------------------------------------------------------------------------- */

GLS_UInt32 GLS_Log2(GLS_UInt32 value)
{
	GLS_UInt32 result = 0;
	while (value >>= 1)	++result;
	return result;
}

/* -------------------------------------------------------------------------- */

GLS_Bool GLS_IntersectGLfloatRects(const GLS_GLfloatRect * r1, const GLS_GLfloatRect * r2, GLS_GLfloatRect * result)
{
	GLS_Bool rectsIntersect = (r1->x1 >= r2->x2 ||
							   r1->x2 <= r2->x1 ||
							   r1->y1 >= r2->y2 ||
							   r1->y2 <= r2->y1) ? GLS_False : GLS_True;

	if(rectsIntersect)
	{
		result->x1 = GLS_MAX(r1->x1, r2->x1);
		result->y1 = GLS_MAX(r1->y1, r2->y1);
		result->x2 = GLS_MIN(r1->x2, r2->x2);
		result->y2 = GLS_MIN(r1->y2, r2->y2);
	}

	return rectsIntersect;
}
