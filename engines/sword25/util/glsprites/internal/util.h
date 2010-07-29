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

#ifndef GLS_UTIL_H
#define GLS_UTIL_H

#include "../glsprites.h"
#include "glinclude.h"

typedef struct  
{
	GLfloat x1;
	GLfloat y1;
	GLfloat x2;
	GLfloat y2;
} GLS_GLfloatRect;

GLS_Bool	GLS_IsPowerOf2(GLS_UInt32 value);
GLS_UInt32	GLS_NextPowerOf2(GLS_UInt32 value);
GLS_UInt32	GLS_Log2(GLS_UInt32 value);
GLS_Bool	GLS_IntersectGLfloatRects(const GLS_GLfloatRect * r1, const GLS_GLfloatRect * r2, GLS_GLfloatRect * result);

#define GLS_MAX(a,b)            (((a) > (b)) ? (a) : (b))
#define GLS_MIN(a,b)            (((a) < (b)) ? (a) : (b))

#endif
