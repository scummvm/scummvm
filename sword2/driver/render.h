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

//=============================================================================
//
//	Filename	:	render.h
//	Created		:	26th August 1996
//	By			:	P.R.Porter
//
//	Version	Date		By		Description
//	------- ---------	---		-----------------------------------------------
//	1.0		16-Sep-96	PRP		Interface to the current scroll position.
//
//	1.1		23-Oct-96	PRP		Added definition of _parallaxLine structure.
//
//	1.2		05-Nov-96	PRP		Added definition of myScreenBuffer.
//
//	1.3		06-Nov-96	PRP		Fixed definition of myScreenBuffer.
//
//	1.4		15-Nov-96	PRP		Definition of menubar size removed, and put
//								into menu.h
//
//	1.5		24-Jan-97	PRP		Added references to parallaxScrollx and
//								parallaxScrolly.  These are used by the sprite
//								drawing code to link sprites to parallax
//								layers.
//
//	1.6		19-Mar-97	PRP		Added profiling flag for testing purposes.
//
//	1.7		24-Mar-97	PRP		Turned profiling off
//
//	1.8		08-Apr-97	JEL		Made locationWide and locationDeep accessible
//								to other drivers.
//
//
//
//	Summary		:	This include file defines links to all data which is
//					defined in the render.c module, but can be accessed by
//					other parts of the driver96 library.
//
//
//=============================================================================


#ifndef RENDER_H
#define RENDER_H

#include "menu.h"

#define RENDERWIDE 640
#define ALIGNRENDERDEEP 480
#define RENDERDEEP (ALIGNRENDERDEEP - (MENUDEEP * 2))


#define PROFILING 0


typedef struct
{
	uint16	packets;
	uint16	offset;
} _parallaxLine;



extern int16					scrollx;			// current x offset into background of display
extern int16					scrolly;			// current y offset into background of display
extern int16					parallaxScrollx;	// current x offset to link a sprite to the parallax layer
extern int16					parallaxScrolly;	// current y offset to link a sprite to the parallax layer
extern int16					locationWide;
extern int16					locationDeep;

// extern uint8 myScreenBuffer[RENDERWIDE * RENDERDEEP];

void SquashImage(byte *dst, uint16 dstPitch, uint16 dstWidth, uint16 dstHeight, byte *src, uint16 srcPitch, uint16 srcWidth, uint16 srcHeight);
void StretchImage(byte *dst, uint16 dstPitch, uint16 dstWidth, uint16 dstHeight, byte *src, uint16 srcPitch, uint16 srcWidth, uint16 srcHeight);

void UploadRect(ScummVM::Rect *r);

#endif

