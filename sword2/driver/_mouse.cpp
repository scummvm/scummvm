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
//	Filename	:	mouse.c
//	Created		:	17th September 1996
//	By			:	P.R.Porter
//
//	Summary		:	This module holds the interface to the mouse..
//
//	Version	Date		By		Description
//	------- ---------	---		-----------------------------------------------
//	1.0		18-Sep-96	PRP		Initial mouse functions.  Simple logging of
//								previous 16 mouse events implemented.  Mouse
//								drawing is currently hard coded, but animations
//								will be definable at a later date.
//
//	1.1		03-Oct-96	PRP		Changed the mouse position so that mouse y of
//								zero is the top left corner of the screen, and
//								not the top left corner of the top menubar.
//								Also, removed the x and y position from the
//								mouse log.  And changed the MouseEvent function
//								so that it returns a pointer to _mouseEvent
//
//	1.2		04-Oct-96	PRP		Put direct path in for ddraw.h
//
//	1.3		31-Oct-96	PRP		Added code to draw the proper type of mouse
//								sprite, which comprises of the internal
//								function DrawMouse and globally available
//								AnimateMouse and SetMouseAnim.
//
//	1.4		15-Nov-96	PRP		Definition of menubar size is now obtained
//								from menu.h
//
//	1.5		18-Nov-96	PRP		Changed the direct draw interface to
//								IDirectDraw2.
//
//	1.6		25-Nov-96	PRP		Added functionality to set the luggage
//								sprite.
//
//	1.7		06-Jan-97	PRP		Changed the width and height of sprites
//								to be signed.
//
//	1.8		14-Jan-97	JEL		Reset mouse frame when new animation starts.
//
//	1.9		27-Jan-97	PRP		Changed the mouse drawing routines to utilize
//								directDraw surfaces and transparency blitting.
//
//	1.10	10-Feb-97	PRP		Changed the directDraw error reporting so that
//								it works properly.  Also, created the mouse
//								sprite depending upon whether the hardware can
//								blt or not.
//
//	1.11	19-Mar-97	PRP		Fixed a bug which was causing the mouse sprite
//								to be freed up each frame and therefore 
//								decompressed and re-loaded each frame.
//
//	1.12	20-Mar-97	PRP		Added a function to reset the render code when
//								the control panel is entered.
//
//	1.13	09-Apr-97	PRP		Made the mouse animation wrap back to the
//								seventh frame.
//
//	1.14	10-Apr-97	PRP		Added parameter to define whether mouse flashes
//								or not.
//
//	1.15	23-Jul-97	JEL		Added CheckForMouseEvents() to return no. of events outstanding
//
//
//	Functions
//	---------
//
//	--------------------------------------------------------------------------
//
//  _mouseEvent *MouseEvent(void)
//
//	The address of a _mouseEvent pointer is passed in.  If there is a mouse 
//	event in the queue, a the value of the mouse event pointer is set to the
//	address of the event, otherwise, the mouse event pointer is set to NULL.
//
//	--------------------------------------------------------------------------
//
//	int32 SetMouseAnim(uint8 *ma, int32 size)
//
//	A pointer to a valid mouse animation is passed in, along with the size of
//	the header plus sprite data.  Remember to check that the function has 
//	successfully completed, as memory allocation is required.
//	Pass NULL in to clear the mouse sprite.
//
//	--------------------------------------------------------------------------
//
//	int32 SetLuggageAnim(uint8 *ma, int32 size)
//
//	A pointer to a valid luggage animation is passed in, along with the size of
//	the header plus sprite data.  Remember to check that the function has 
//	successfully completed, as memory allocation is required.
//	Pass NULL in to clear the luggage sprite.  Luggage sprites are of the same
//	format as mouse sprites.
//
//	--------------------------------------------------------------------------
//
//	int32 AnimateMouse(void)
//
//	This function animates the current mouse pointer.  If no pointer is 
//	currently defined, an error code is returned.
//
//=============================================================================


#define WIN32_LEAN_AND_MEAN

//#include <windows.h>
//#include <windowsx.h>

//#include "ddraw.h"

#include "stdafx.h"
#include "driver96.h"

#include "d_draw.h"
#include "render.h"
#include "menu.h"
#include "../sword2.h"



#define MAX_MOUSE_EVENTS 16
#define MOUSEFLASHFRAME 6


typedef struct
{
	uint8	runTimeComp;	// type of runtime compression used for the frame data
	uint8	noAnimFrames;	// number of frames in the anim
	int8	xHotSpot;		
	int8	yHotSpot;
	uint8	mousew;
	uint8	mouseh;
} _mouseAnim;


int16				mousex;
int16				mousey;

static	uint8		mouseBacklog = 0;
static	uint8		mouseLogPos = 0;
static	uint8		mouseFrame;
//static	uint8		luggageFrame = 0;
static	uint8		*mouseSprite = NULL;
static	_mouseAnim	*mouseAnim = NULL;
//static	_mouseAnim	*luggageAnim = NULL;
static	_mouseEvent	mouseLog[MAX_MOUSE_EVENTS];
static  int32		*mouseOffsets;
//static	int32		*luggageOffset;
//static	LPDIRECTDRAWSURFACE	*mouseSurfaces;
//static	LPDIRECTDRAWSURFACE luggageSurface = NULL;







void ResetRenderEngine(void)

{
	
	memset(myScreenBuffer, 0, RENDERWIDE * RENDERDEEP);
	parallaxScrollx = 0;
	parallaxScrolly = 0;
	scrollx = 0;
	scrolly = 0;

}




//	--------------------------------------------------------------------------
//	Logs the mouse button event passed in buttons.  The button events are 
//	defined as RD_LEFTBUTTONDOWN, RD_LEFTBUTTONUP, RD_RIGHTBUTTONDOWN and
//	RD_RIGHTBUTTONUP.
//	--------------------------------------------------------------------------
void LogMouseEvent(uint16 buttons)

{

	_mouseEvent *me;


	if (mouseBacklog == MAX_MOUSE_EVENTS-1)			// We need to leave the one which is
	{												// the current event alone!
		return;
	}
	
	me = &mouseLog[(mouseBacklog + mouseLogPos) % MAX_MOUSE_EVENTS];
	me->buttons = buttons;
	mouseBacklog += 1;

}






int32 DecompressMouse(uint8 *decomp, uint8 *comp, int32 size)

{

	int32 i = 0;

	while (i < size)
	{
		if (*comp > 183)
		{
			*decomp++ = *comp++;
			i += 1;
		}
		else
		{
			memset(decomp, 0, *comp);
			decomp += *comp;
			i += *comp++;
		}
	}
	return(RD_OK);
}



//	--------------------------------------------------------------------------
//	Draws the mouse sprite to the back buffer.
//	--------------------------------------------------------------------------
int32 DrawMouse(void)

{
	warning("stub DrawMouse");
/*
	uint8			*src, *dst;
	int16			i;
	int16			xoff=0, yoff=0;
	uint8			*decompSprite;
	DDSURFACEDESC	ddsd;
	HRESULT			hr;
	RECT			rs, rd;



	if (luggageAnim)
	{

		if (luggageSurface == NULL)
		{
			// Create the luggage surface.
			memset(&ddsd, 0, sizeof(DDSURFACEDESC));
			ddsd.dwSize = sizeof(DDSURFACEDESC);
			ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			if (dxHalCaps & RDCAPS_SRCBLTCKEY)
				ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
			else
				ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

			ddsd.dwWidth = luggageAnim->mousew;
			ddsd.dwHeight = luggageAnim->mouseh;
			hr = IDirectDraw2_CreateSurface(lpDD2, &ddsd, &luggageSurface, NULL);
			if (hr != DD_OK)
			{
				if (hr == DDERR_OUTOFVIDEOMEMORY)
				{
					ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
					hr = IDirectDraw2_CreateSurface(lpDD2, &ddsd, &luggageSurface, NULL);
				}
				if (hr != DD_OK)
				{
					DirectDrawError("Cannot create mouse surface", hr);
					return(hr);
				}
			}
			//	Set the surface blt source colour key
			hr = IDirectDrawSurface2_SetColorKey(luggageSurface, DDCKEY_SRCBLT, &blackColorKey);

			//  Copy the data into the surfaces.
			decompSprite = (uint8 *) malloc(luggageAnim->mousew * luggageAnim->mouseh);
			if (decompSprite == NULL)
				return(RDERR_OUTOFMEMORY);
//			DecompressMouse(decompSprite, (uint8 *) luggageAnim + *mouseOffsets, luggageAnim->mousew * luggageAnim->mouseh);
			DecompressMouse(decompSprite, (uint8 *) luggageAnim + *luggageOffset, luggageAnim->mousew * luggageAnim->mouseh);
			memset(&ddsd, 0, sizeof(DDSURFACEDESC));
			ddsd.dwSize = sizeof(DDSURFACEDESC);
			hr = IDirectDrawSurface2_Lock(luggageSurface, NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
			if (hr != DD_OK)
			{
				IDirectDrawSurface2_Restore(luggageSurface);
				hr = IDirectDrawSurface2_Lock(luggageSurface, NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
				if (hr != DD_OK)
				{
					DirectDrawError("Unable to lock luggage surface", hr);
					return(hr);
				}
			}
			dst = ddsd.lpSurface;
			src = decompSprite;
			for (i=0; i<luggageAnim->mouseh; i++)
			{
				memcpy(dst, src, luggageAnim->mousew);
				dst += ddsd.lPitch;
				src += luggageAnim->mousew;
			}
			IDirectDrawSurface2_Unlock(luggageSurface, ddsd.lpSurface);
			free(decompSprite);
		}


		rd.top = mousey + MENUDEEP - luggageAnim->yHotSpot;
		rd.bottom = rd.top + luggageAnim->mouseh;
		rd.left = mousex - luggageAnim->xHotSpot;
		rd.right = rd.left + luggageAnim->mousew;

		rs.left = 0;
		rs.right = luggageAnim->mousew;
		rs.top = 0;
		rs.bottom = luggageAnim->mouseh;

		if (rd.left < 0)
		{
			rs.left = 0 - rd.left;
			rd.left = 0;
		}
		if (rd.top < 0)
		{
			rs.top = 0 - rd.top;
			rd.top = 0;
		}
		if (rd.right > RENDERWIDE)
		{
			rs.right -= (rd.right - RENDERWIDE);
			rd.right = RENDERWIDE;
		}
		if (rd.bottom > ALIGNRENDERDEEP)
		{
			rs.bottom -= (rd.bottom - ALIGNRENDERDEEP);
			rd.bottom = ALIGNRENDERDEEP;
		}

		hr = IDirectDrawSurface2_Blt(lpBackBuffer, &rd, luggageSurface, &rs, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
		if (hr = DDERR_SURFACELOST)
		{
			IDirectDrawSurface2_Release(luggageSurface);
			luggageSurface = NULL;
		}

	}

	if (mouseAnim == NULL)
	{
		return(RD_OK);
	}


	//  Decompress the mouse sprite onto the directDraw surface, if it is not
	//	there already.
	if (*(mouseSurfaces + mouseFrame) == NULL)
	{

		// Create the mouse surface.
		memset(&ddsd, 0, sizeof(DDSURFACEDESC));
		ddsd.dwSize = sizeof(DDSURFACEDESC);
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		if (dxHalCaps & RDCAPS_SRCBLTCKEY)
			ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		else
			ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

		ddsd.dwWidth = mouseAnim->mousew;
		ddsd.dwHeight = mouseAnim->mouseh;
		hr = IDirectDraw2_CreateSurface(lpDD2, &ddsd, mouseSurfaces + mouseFrame, NULL);
		if (hr != DD_OK)
		{
			if (hr == DDERR_OUTOFVIDEOMEMORY)
			{
				ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
				hr = IDirectDraw2_CreateSurface(lpDD2, &ddsd, mouseSurfaces + mouseFrame, NULL);
			}
			if (hr != DD_OK)
			{
				DirectDrawError("Cannot create mouse surface", hr);
				return(hr);
			}
		}
		//	Set the surface blt source colour key
		hr = IDirectDrawSurface2_SetColorKey(*(mouseSurfaces + mouseFrame), DDCKEY_SRCBLT, &blackColorKey);

		//  Copy the data into the surfaces.
		decompSprite = (uint8 *) malloc(mouseAnim->mousew * mouseAnim->mouseh);
		if (decompSprite == NULL)
			return(RDERR_OUTOFMEMORY);
		DecompressMouse(decompSprite, mouseSprite, mouseAnim->mousew * mouseAnim->mouseh);
		memset(&ddsd, 0, sizeof(DDSURFACEDESC));
		ddsd.dwSize = sizeof(DDSURFACEDESC);
		hr = IDirectDrawSurface2_Lock(*(mouseSurfaces + mouseFrame), NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
		if (hr != DD_OK)
		{
			IDirectDrawSurface2_Restore(*(mouseSurfaces + mouseFrame));
			hr = IDirectDrawSurface2_Lock(*(mouseSurfaces + mouseFrame), NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
			if (hr != DD_OK)
			{
				DirectDrawError("Cannot lock mouse surface", hr);
				return(hr);
			}
		}
		dst = ddsd.lpSurface;
		src = decompSprite;
		for (i=0; i<mouseAnim->mouseh; i++)
		{
			memcpy(dst, src, mouseAnim->mousew);
			dst += ddsd.lPitch;
			src += mouseAnim->mousew;
		}
		IDirectDrawSurface2_Unlock(*(mouseSurfaces + mouseFrame), ddsd.lpSurface);
		free(decompSprite);
	}


	rd.top = mousey + MENUDEEP - mouseAnim->yHotSpot;
	rd.bottom = rd.top + mouseAnim->mouseh;
	rd.left = mousex - mouseAnim->xHotSpot;
	rd.right = rd.left + mouseAnim->mousew;

	rs.left = 0;
	rs.right = mouseAnim->mousew;
	rs.top = 0;
	rs.bottom = mouseAnim->mouseh;

	if (rd.left < 0)
	{
		rs.left = 0 - rd.left;
		rd.left = 0;
	}
	if (rd.top < 0)
	{
		rs.top = 0 - rd.top;
		rd.top = 0;
	}
	if (rd.right > RENDERWIDE)
	{
		rs.right -= (rd.right - RENDERWIDE);
		rd.right = RENDERWIDE;
	}
	if (rd.bottom > ALIGNRENDERDEEP)
	{
		rs.bottom -= (rd.bottom - ALIGNRENDERDEEP);
		rd.bottom = ALIGNRENDERDEEP;
	}

	hr = IDirectDrawSurface2_Blt(lpBackBuffer, &rd, *(mouseSurfaces + mouseFrame), &rs, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	if (hr == DDERR_SURFACELOST)
	{
		IDirectDrawSurface2_Release(*(mouseSurfaces + mouseFrame));
		*(mouseSurfaces + mouseFrame) = NULL;
	}
*/
	return(RD_OK);

}



_mouseEvent *MouseEvent(void)

{
	_mouseEvent *me;

	if (mouseBacklog)
	{
		me = &mouseLog[mouseLogPos];
		if (++mouseLogPos == MAX_MOUSE_EVENTS)
		{
			mouseLogPos = 0;
		}
		mouseBacklog -= 1;
		return(me);
	}

	return(NULL);

}


uint8 CheckForMouseEvents(void)		// (James23july97)
{
	return (mouseBacklog);	// return the number of mouse events waiting	
}


int32 AnimateMouse(void)

{
	if (mouseAnim)
	{
		if (++mouseFrame == mouseAnim->noAnimFrames)
		{
			mouseFrame = MOUSEFLASHFRAME;
		}
		mouseSprite = (uint8 *) mouseAnim + *(mouseOffsets+mouseFrame);
	}
	else
	{
		return(RDERR_UNKNOWN);
	}
	
	return(RD_OK);

}



int32 SetMouseAnim(uint8 *ma, int32 size, int32 mouseFlash)
{
	if (ma) {
		if (mouseAnim)
			free(mouseAnim);

		if (mouseFlash == RDMOUSE_FLASH)
			mouseFrame = 0;
		else
			mouseFrame = MOUSEFLASHFRAME;

		mouseAnim = (_mouseAnim *)malloc(size);
		memcpy((uint8 *)mouseAnim, ma, size);

		mouseOffsets = (int32 *)((uint8 *)mouseAnim + sizeof(_mouseAnim));
		AnimateMouse();

		uint8 *decompSprite = (uint8 *)malloc(mouseAnim->mousew * mouseAnim->mouseh);
		DecompressMouse(decompSprite, mouseSprite, mouseAnim->mousew * mouseAnim->mouseh);

		int i;
		for (i = 0; i < mouseAnim->mousew * mouseAnim->mouseh; i++)
			if (decompSprite[i] == 0)
				decompSprite[i] = 0xff;
		
		g_sword2->_system->set_mouse_cursor(decompSprite,
				mouseAnim->mousew, mouseAnim->mouseh,
				mouseAnim->xHotSpot, mouseAnim->yHotSpot - MENUDEEP);
		g_sword2->_system->show_mouse(true);
	} else
		g_sword2->_system->show_mouse(false);

/*
	int32 i;

	if (mouseAnim)
	{
		for (i=0; i<mouseAnim->noAnimFrames; i++)
		{
			if (*(mouseSurfaces + i))
			{
				IDirectDrawSurface2_Release(*(mouseSurfaces + i));
				*(mouseSurfaces + i) = NULL;
			}
		}

		free(mouseAnim);
		mouseAnim = NULL;
		free(mouseSurfaces);
		mouseSurfaces = NULL;
	}

	if (ma)
	{
		if (mouseFlash == RDMOUSE_FLASH)
			mouseFrame = 0;
		else
			mouseFrame = MOUSEFLASHFRAME;
		mouseAnim = malloc(size);
		if (mouseAnim == NULL)
		{
			return(RDERR_OUTOFMEMORY);
		}
		else
		{
			memcpy((uint8 *) mouseAnim, ma, size);
			mouseOffsets = (int32 *) ((uint8 *) mouseAnim + sizeof(_mouseAnim));
			AnimateMouse();
			mouseSurfaces = (LPDIRECTDRAWSURFACE *) malloc(mouseAnim->noAnimFrames * sizeof(LPDIRECTDRAWSURFACE));
			if (mouseSurfaces == NULL)
				return(RDERR_OUTOFMEMORY);
			memset(mouseSurfaces, 0, sizeof(LPDIRECTDRAWSURFACE) * mouseAnim->noAnimFrames);
		}
	}
*/
	return(RD_OK);

}


int32 SetLuggageAnim(uint8 *ma, int32 size)

{
	warning("stub SetLugggeAnim");
/*
	
	if (luggageAnim)
	{
		free(luggageAnim);
		luggageAnim = NULL;
	}

	if (ma)
	{
		luggageAnim = malloc(size);
		if (luggageAnim == NULL)
		{
			return(RDERR_OUTOFMEMORY);
		}
		else
		{
			memcpy((uint8 *) luggageAnim, ma, size);
			luggageOffset = (int32 *) ((uint8 *) luggageAnim + sizeof(_mouseAnim));
			AnimateMouse();
		}
	}
*/
	return(RD_OK);

}


