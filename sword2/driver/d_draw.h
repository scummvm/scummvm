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
//	Filename	:	d_draw.h
//	Created		:	22nd August 1996
//	By			:	P.R.Porter
//
//	Version	Date		By		Description
//	------- ---------	---		-----------------------------------------------
//	1.0		16-Sep-96	PRP		Interface to the DirectDraw driver functions
//								and data.
//
//	1.1		19-Sep-96	PRP		Increased scope of directDraw data, such as
//								the back buffer object, so that mouse.c can
//								have access to the screen.
//
//	1.2		25-Sep-96	PRP		Made scrollx and scrolly available to all
//								driver code.
//
//	1.3		26-Sep-96	PRP		Moved scroll stuff to render.h for clarity.
//
//	1.4		07-Nov-96	PRP		Made bFullScreen available to all driver code.
//
//	1.5		18-Nov-96	PRP		Added reference to lpDD2 object.
//
//	1.6		24-Jan-97	PRP		Added hardware capability bits and defines.
//
//	1.7		06-Mar-97	PRP		Changed capability bits, and changed the 
//								direct draw error reporting call.  Added 
//								function to grab screen shot.
//
//	1.8		16-Jun-97	PSJ		Made globall externable to c++.
//
//	1.9		27-Jun-97	PRP		Moving the definition of GrabScreenShot to 
//								driver96.h for external access.
//
//
//	Summary		:	This include file defines links to all data which is
//					defined in the d_draw.c module, but can be accessed by
//					other parts of the driver96 library.
//
//
//=============================================================================


#ifndef D_DRAW_H
#define D_DRAW_H

#ifdef __cplusplus
extern "C" {
#endif

//#include "ddraw.h"
#include "driver96.h"

//  Bits defining hardware and emulation capabilities.
#define RDCAPS_BLTSTRETCH	1
#define RDCAPS_SRCBLTCKEY	2
#define DirectDrawError(f, g) FatalDirectDrawError(f, g, __FILE__, __LINE__)


extern uint8 *lpPalette;			// palette
extern byte *lpBackBuffer;		// back surface
// extern Surface *lpPrimarySurface;	// DirectDraw front buffer.
extern uint8 *lpDD2;				// DirectDraw2 object
extern BOOL bFullScreen;		// Defines whether the app is running in full screen mode or not.
//extern DDCOLORKEY				blackColorKey;		// transparent pixel for color key blitting.
extern uint8 blackColorKey;		// transparent pixel for color key blitting.
extern int32					dxHalCaps;			// Hardware capabilities.
extern int32					dxHelCaps;			// Emulation capabilities.

extern void FatalDirectDrawError(char *str, int32 dderr, char *file, int32 line);

#ifdef __cplusplus
}
#endif

#endif
