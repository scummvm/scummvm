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
//	Filename	:	console.c
//	Created		:	19th September 1996
//	By			:	P.R.Porter
//
//	Summary		:	This module holds the code which controls and displays
//					the console/debugging window.
//
//	Version	Date		By		Description
//	------- ---------	---		-----------------------------------------------
//	1.0		20-Sep-96	PRP		Inital console code.  Currently draws the 
//								background of the console window.  Tex
//
//
//	Functions
//	---------
//
//	--------------------------------------------------------------------------
//
//	int32 OpenConsole(void)
//
//	Displays the console window and directs keyboard input to it.
//
//	--------------------------------------------------------------------------
//
//	int32 CloseConsole(void)
//
//	Removes the console from the display.
//
//=============================================================================


//#include "ddraw.h"

#include "stdafx.h"
#include "driver96.h"

#include "d_draw.h"



uint8			consoleStatus = 0;			// 1 - console display
//static uint16	consoley = 0;
//static uint32	consoleSize;
static uint8	*consoleSprite = NULL;



//	--------------------------------------------------------------------------
//	Called before the screens are flipped, so that the console can be drawn
//	over the screen if necessary.
//	--------------------------------------------------------------------------
void DisplayConsole(void)

{
	warning("stub DisplayConsole");
/*
	uint8			*src, *dst;
	uint8			i;
//	DDSURFACEDESC	ddDescription;
//	HRESULT			hr;


	if (consoleStatus)
	{

		ddDescription.dwSize = sizeof(ddDescription);
	
		hr = IDirectDrawSurface_Lock(lpBackBuffer, NULL, &ddDescription, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		if (hr != DD_OK)
		{
			hr = IDirectDrawSurface_Lock(lpBackBuffer, NULL, &ddDescription, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		}

		if (hr == DD_OK)
		{

			dst = (uint8 *) ddDescription.lpSurface + (screenDeep - consoley) * ddDescription.lPitch;
			src = (uint8 *) consoleSprite;

			for (i=0; i<consoley; i++)
			{
				memcpy(dst, src, screenWide);
				src += screenWide;
				dst += ddDescription.lPitch;
			}
			IDirectDrawSurface_Unlock(lpBackBuffer, ddDescription.lpSurface);
		}
	}
	*/
}




int32 OpenConsole(void)

{
	warning("stub OpenConsole");
/*
	if (consoleStatus)
		return(RDERR_ALREADYOPEN);

	if (consoleSprite == NULL)
	{
		consoley = screenDeep >> 2;
		consoleSize = screenWide * consoley;
		consoleSprite = (uint8 *) malloc(consoleSize);
	}

	if (consoleSprite == NULL)
		return(RDERR_OUTOFMEMORY);

	memset(consoleSprite, 0, consoleSize);
*/
	consoleStatus = 1;
	
	return(RD_OK);

}




int32 CloseConsole(void)

{

	if (!consoleStatus)
		return(RDERR_ALREADYCLOSED);

	free(consoleSprite);
	consoleSprite = NULL;

	consoleStatus = 0;

	return(RD_OK);

}

