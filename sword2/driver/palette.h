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
//	Filename	:	palette.h
//	Created		:	8th November 1996
//	By			:	P.R.Porter
//
//	Version	Date		By		Description
//	------- ---------	---		-----------------------------------------------
//	1.0		08-Nov-96	PRP		Internal driver interface to the palette
//								functions and data.
//
//	1.1		11-Nov-96	PRP		Added internal driver reference for
//								RestorePalette which should be called by the
//								windows message handler whenever the window
//								has been minimised/maximised.
//
//	1.2		22-Nov-96	PRP		Made the palette available to the rest of the
//								driver96 library.
//
//	1.3		29-Nov-96	PRP		Made paletteMatch table available to other
//								parts of the driver library.  Also, made the
//								FadeServer available for the windows module
//								to have access.
//
//	1.4		11-Apr-97	CJR		Moved palCopy to driver96.h for use in the
//								game engine.
//
//
//	Summary		:	This include file defines links to all data which is
//					defined in the palette.c module, but can be accessed by
//					other parts of the driver96 library.
//
//
//=============================================================================


#ifndef PALETTE_H
#define PALETTE_H

extern uint8 paletteMatch[64*64*64];

extern int32 RestorePalette(void);
extern void FadeServer(void *);


#endif

