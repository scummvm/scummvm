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
//	Summary		:	This include file defines links to all data which is
//					defined in the render.c module, but can be accessed by
//					other parts of the driver96 library.
//
//
//=============================================================================

#ifndef RENDER_H
#define RENDER_H

#include "menu.h"

#define RENDERWIDE		640
#define ALIGNRENDERDEEP		480
#define RENDERDEEP		(ALIGNRENDERDEEP - (MENUDEEP * 2))

typedef struct {
	uint16 packets;
	uint16 offset;
} _parallaxLine;

extern int16 scrollx;		// current x offset into background of display
extern int16 scrolly;		// current y offset into background of display
extern int16 parallaxScrollx;	// current x offset to link a sprite to the
				// parallax layer
extern int16 parallaxScrolly;	// current y offset to link a sprite to the
				// parallax layer
extern int16 locationWide;
extern int16 locationDeep;

void SquashImage(byte *dst, uint16 dstPitch, uint16 dstWidth, uint16 dstHeight, byte *src, uint16 srcPitch, uint16 srcWidth, uint16 srcHeight, byte *backbuf);
void StretchImage(byte *dst, uint16 dstPitch, uint16 dstWidth, uint16 dstHeight, byte *src, uint16 srcPitch, uint16 srcWidth, uint16 srcHeight, byte *backbuf);

void UploadRect(ScummVM::Rect *r);

#endif
