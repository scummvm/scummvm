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

#define WIN32_LEAN_AND_MEAN

//#include <windows.h>
//#include <windowsx.h>
//#include <mmsystem.h>
#include <stdio.h>

//#include "ddraw.h"

#include "stdafx.h"
#include "driver96.h"
#include "rdwin.h"
#include "_mouse.h"
#include "d_draw.h"
#include "palette.h"

//#include "ddutil.h"
//CDisplay	*g_pDisplay        = NULL;





#define SCREENYOFFSET	40

#define MILLISECSPERCYCLE 83



/*
LPDIRECTDRAW7        m_pDD;


static LPDIRECTDRAW		lpDraw;				// DirectDraw object
LPDIRECTDRAW2			lpDD2;				// DirectDraw2 object
LPDIRECTDRAWSURFACE		lpPrimarySurface;	// DirectDraw primary surface
LPDIRECTDRAWSURFACE		lpBackBuffer;		// DirectDraw back surface
LPDIRECTDRAWPALETTE		lpPalette = NULL;	// DirectDraw palette

static PALETTEENTRY AppPalette[256];		// Application wide logical palette
*/


// Game screen metrics
int16			screenDeep;
int16			screenWide;


// Set to 1 if vertical blank status cannot be checked.
static BOOL		noVbl = 0;
BOOL			bFullScreen = 0;



// Scroll variables.  scrollx and scrolly hold the current scroll position, 
//	and scrollxTarget and scrollyTarget are the target position for the end
//	of the game cycle.

int16			scrollx;
int16			scrolly;
static	int16	scrollxTarget;
static	int16	scrollyTarget;
static	int16	scrollxOld;
static	int16	scrollyOld;
static	int16	failCount = 0;
//static	DDCAPS	driverCaps;

int32			renderCaps = 0;
int32			dxHalCaps = 0;
int32			dxHelCaps = 0;
//DDCOLORKEY		blackColorKey;


//static int32 platformIsNT = 0;


//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------


void FatalDirectDrawError(char *str, int32 code, char *filename, int32 line)
{

	char string[256];

	RestoreDisplay();

	sprintf(string, "FATAL: %s - code 0x%.8x - file %s - line %d", str, code, filename, line);
	//MessageBox(hwnd, string, "DDraw error", MB_OK);
	warning("%s", string);

}




//----------------------------------------------------------------------------------------------------------------



int32 PlotDots(int16 x, int16 y, int16 count)

{

	warning("stub PlotDots( %d, %d, %d )", x, y, count);
/*
	int16			i;
	uint8			*dst;

	DDSURFACEDESC	ddDescription;
	HRESULT			hr;

	ddDescription.dwSize = sizeof(ddDescription);
	
	hr = IDirectDrawSurface2_Lock(lpBackBuffer, NULL, &ddDescription, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	if (hr != DD_OK)
	{
		RestoreSurfaces();
		hr = IDirectDrawSurface2_Lock(lpBackBuffer, NULL, &ddDescription, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	}

	if (hr == DD_OK)
	{

		dst = (uint8 *) ddDescription.lpSurface + y * ddDescription.lPitch + x;

		for (i=0; i<=count; i++)
		{
			*dst = 184;
			dst += 2;
		}
		dst = (uint8 *) ddDescription.lpSurface + (y+1) * ddDescription.lPitch + x;
		for (i=0; i<=count/10; i++)
		{
			*dst = 184;
			dst += 20;
		}
		IDirectDrawSurface2_Unlock(lpBackBuffer, ddDescription.lpSurface);
	}
*/
	return(RD_OK);

}
//----------------------------------------------------------------------------------------------------------------



void RestoreSurfaces(void)

{
	warning("stub RestoreSurfaces");
/*
	IDirectDrawSurface2_Restore(lpPrimarySurface);
	IDirectDrawSurface2_Restore(lpBackBuffer);
*/
}

//----------------------------------------------------------------------------------------------------------------

/*
static PALETTEENTRY *CreateAppPalette(PALETTEENTRY *pe)
{

	HDC	screen_dc;

//  Fill the palette with system colours
	screen_dc = GetDC(NULL);
	GetSystemPaletteEntries(screen_dc, 0, 256, pe);
	ReleaseDC(NULL, screen_dc);

	return pe;

}
*/


//----------------------------------------------------------------------------------------------------------------

int32 RestoreDisplay(void)

{
	warning("stub RestoreDisplay");
/*
    if( lpDraw != NULL )
    {
        if( lpPrimarySurface != NULL )
        {
            IDirectDrawSurface2_Release(lpPrimarySurface);
            lpPrimarySurface = NULL;
        }
        if( lpPalette != NULL )
        {
            IDirectDrawPalette_Release(lpPalette);
            lpPalette = NULL;
        }
		if (lpDD2 != NULL)
		{
			IDirectDraw2_Release(lpDD2);
			lpDD2 = NULL;
		}

		IDirectDraw_Release(lpDraw);
		lpDraw = NULL;
    }
*/
	return(RD_OK);

}


//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------

int32 InitialiseDisplay(int16 width, int16 height, int16 colourDepth, int32 windowType)
{
/*
	DDSURFACEDESC       ddsd;
    DDSCAPS             ddscaps;
    HRESULT             hr;
	DDCAPS				helCaps;
	long int timerFrequency;
	int32 capsError = 0;


   OSVERSIONINFO VersionInfo;
   VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   
   if (GetVersionEx(&VersionInfo))
   {

	   switch (VersionInfo.dwPlatformId)
	   {
	   case VER_PLATFORM_WIN32s :
	   case VER_PLATFORM_WIN32_WINDOWS :
		  break;
	   case VER_PLATFORM_WIN32_NT	:
		   platformIsNT = 1;
		   break;
	   }
   }


	if (windowType == RD_FULLSCREEN)
		bFullScreen = TRUE;
	else
		bFullScreen = FALSE;

	// Colour depths of 8 bits only are currently supported
	if (colourDepth != 8)
	{
		return(RDERR_COLOURDEPTH);
	}

	screenWide = width;
	screenDeep = height;

	// Create the directDraw object
    hr = DirectDrawCreate(NULL, &lpDraw, NULL);
//	hr=DirectDrawCreateEx( NULL, (VOID**)&m_pDD, IID_IDirectDraw7, NULL );


    if ( hr != DD_OK )
	{
//		Zdebug(" DirectDrawCreate failed!");

        DirectDrawError("DirectDraw unavailable", hr);
		return(hr);
	}


    // Get exclusive mode
	if (bFullScreen)
	{

//		hr = m_pDD->SetCooperativeLevel( hWnd, DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN );
	    hr = IDirectDraw2_SetCooperativeLevel(lpDraw, hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
		if (hr != DD_OK)
		{
			DirectDrawError("Exclusive mode unavailable", hr);
			return(hr);
		}

		// TONY TEMP	


		hr = IDirectDraw2_QueryInterface(lpDraw, &IID_IDirectDraw2, (LPVOID *) &lpDD2);
		if (hr != DD_OK)
		{
			DirectDrawError("DirectDraw2 unavailable", hr);
			return(hr);
		}

		// Set up the display mode which has been requested
//		hr = lpDD2->lpVtbl->SetDisplayMode(lpDD2, width, height, colourDepth, 0, 0);
		hr = IDirectDraw2_SetDisplayMode(lpDD2, width, height, colourDepth, 0, 0);
		if (hr != DD_OK)
		{
			DirectDrawError("Unable to set display mode", hr);
			return(hr);
		}

	    // Set up the primary surface descriptor
		ddsd.dwSize = sizeof(ddsd);
	    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	    ddsd.dwBackBufferCount = 1;

		// Create the primary surface
	    hr = IDirectDraw2_CreateSurface(lpDD2, &ddsd, &lpPrimarySurface, NULL);
		if (hr != DD_OK)
		{
		    DirectDrawError("Cannot create primary surface", hr);
			return(hr);
		}

		// Create the back buffer as a page flipping surface
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
	    hr = IDirectDrawSurface2_GetAttachedSurface(lpPrimarySurface, &ddscaps, &lpBackBuffer);
		if (hr != DD_OK)
		{
		    DirectDrawError("Unable to attach back buffer", hr);
			return(hr);
		}

		EraseBackBuffer();
		FlipScreens();
		EraseBackBuffer();
		FlipScreens();

		// Create a palette object
		hr = IDirectDraw2_CreatePalette(lpDD2, DDPCAPS_8BIT | DDPCAPS_ALLOW256 | DDPCAPS_INITIALIZE,
				CreateAppPalette(AppPalette), &lpPalette, NULL);
	    if (hr != DD_OK )
		{
			DirectDrawError("Cannot create 8-bit palette", hr);
			return(hr);
		}

		// Set our palette object active
		hr = IDirectDrawSurface2_SetPalette(lpPrimarySurface, lpPalette);
	    if (hr != DD_OK )
		{
			DirectDrawError("Unable to set palette", hr);
			return(hr);
		}

	}
	else
	{
        RECT rcWork;
        RECT rc;
        HDC hdc;
        DWORD dwStyle;
		uint32 GameBPP;



		hr = IDirectDraw_SetCooperativeLevel(lpDraw, hwnd, DDSCL_NORMAL);
		if (hr != DD_OK)
		{
			DirectDrawError("Cannot set normal cooperative level", hr);
			return(hr);
		}



		hr = IDirectDraw_QueryInterface(lpDraw, &IID_IDirectDraw2, (LPVOID *) &lpDD2);
		if (hr != DD_OK)
		{
			DirectDrawError("DirectDraw2 unavailable", hr);
			return(hr);
		}

        //
        //  when in windows we should use the current mode
        //
        hdc = GetDC(NULL);
        GameBPP = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);
        ReleaseDC(NULL, hdc);

		if ((GameBPP != 8) && (GameBPP != 16))
		{
			MessageBox(hwnd, "Cannot execute in high colour mode - going to full screen", "Broken Sword II", MB_OK);
			return(RDERR_GOFULLSCREEN);
		}
		else if (GameBPP != 8)
		{
			if (MessageBox(hwnd, "Your display is not in 256 colour mode.  Would you like to go to full screen mode (better performance)", "Broken Sword II", MB_YESNO) == IDYES)
			{
				return(RDERR_GOFULLSCREEN);
			}
		}

        //
        // if we are still a WS_POPUP window we should convert to a
        // normal app window so we look like a windows app.
        //
        dwStyle = GetWindowStyle(hwnd);
        dwStyle &= ~WS_POPUP;
        dwStyle |= WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX;
        SetWindowLong(hwnd, GWL_STYLE, dwStyle);

//        if (bStretch)
//            SetRect(&rc, 0, 0, GameMode.cx*2, GameMode.cy*2);
//        else
//            SetRect(&rc, 0, 0, GameMode.cx, GameMode.cy);
		SetRect(&rc, 0, 0, 640, 480);

        AdjustWindowRectEx(&rc,
            GetWindowStyle(hwnd),
            GetMenu(hwnd) != NULL,
            GetWindowExStyle(hwnd));

        SetWindowPos(hwnd, NULL, 0, 0, rc.right-rc.left, rc.bottom-rc.top,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
            SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);

        //
        //  make sure our window does not hang outside of the work area
        //  this will make people who have the tray on the top or left
        //  happy.
        //
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);
        GetWindowRect(hwnd, &rc);
        if (rc.left < rcWork.left) rc.left = rcWork.left;
        if (rc.top  < rcWork.top)  rc.top  = rcWork.top;
        SetWindowPos(hwnd, NULL, rc.left, rc.top, 0, 0,
            SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);




	    // Set up the primary surface descriptor
		ddsd.dwSize = sizeof(ddsd);
	    ddsd.dwFlags = DDSD_CAPS;
	    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

		// Create the primary surface
		hr = IDirectDraw2_CreateSurface(lpDD2, &ddsd, &lpPrimarySurface, NULL);
	    if (hr != DD_OK)
		{
			DirectDrawError("Cannot create primary surface", hr);
			return(hr);
		}

		// Create the back buffer as a page flipping surface
	    memset( &ddsd, 0, sizeof( ddsd ) );
		ddsd.dwSize = sizeof( ddsd );
		ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT |DDSD_WIDTH;

		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	    ddsd.dwHeight = 480;
	    ddsd.dwWidth = 640;

	    hr = IDirectDraw2_CreateSurface(lpDD2, &ddsd, &lpBackBuffer, NULL );
	    if (hr != DD_OK)
		{
	        DirectDrawError("Cannot attach back buffer", hr);
			return(hr);
		}

	//    DDClear();

		if (IDirectDrawSurface2_GetDC(lpPrimarySurface, &hdc) == DD_OK)
	    {
		    char *szMsg = "Broken Sword II is loading.......please wait.";
			SetTextColor(hdc, RGB(255,255,255));
	        SetBkMode(hdc, TRANSPARENT);
		    TextOut(hdc, rcWindow.left, rcWindow.top, szMsg, lstrlen(szMsg));
			IDirectDrawSurface2_ReleaseDC(lpPrimarySurface, hdc);
	    }

		// Create a palette object - only if we have a palette!
		if (GameBPP == 8)
		{
			hr = IDirectDraw2_CreatePalette(lpDD2, DDPCAPS_8BIT, CreateAppPalette(AppPalette), &lpPalette, NULL);
			if (hr != DD_OK )
			{
				DirectDrawError("Cannot create 8-bit palette", hr);
				return(hr);
			}

			hr = IDirectDrawSurface2_SetPalette(lpPrimarySurface, lpPalette);
			if (hr != DD_OK )
			{
				DirectDrawError("Cannot set palette", hr);
				return(hr);
			}
		}
	}

	// Set my capability bits.
	memset(&driverCaps, 0, sizeof(DDCAPS));
	memset(&helCaps, 0, sizeof(DDCAPS));
	driverCaps.dwSize = sizeof(DDCAPS);
	helCaps.dwSize = sizeof(DDCAPS);
	hr = IDirectDraw2_GetCaps(lpDD2, &driverCaps, &helCaps);
	if (hr != DD_OK)
	{
		driverCaps.dwSize = sizeof(DDCAPS_DX3);
		helCaps.dwSize = sizeof(DDCAPS_DX3);
		hr = IDirectDraw2_GetCaps(lpDD2, &driverCaps, &helCaps);
		if (hr != DD_OK)
		{
			MessageBox(hwnd, "Cannot get hardware capabilities.  Software emulation only.  Re-install DirectX!", "DDraw error", MB_OK);
			capsError = 1;
		}
	}

	blackColorKey.dwColorSpaceLowValue = 0;
	blackColorKey.dwColorSpaceHighValue = 0;

	if (capsError)
	{
		helCaps.dwCaps = DDCAPS_BLT + DDCAPS_BLTSTRETCH + DDCAPS_COLORKEY;
		helCaps.dwCKeyCaps = DDCKEYCAPS_SRCBLT;
		dxHelCaps += RDCAPS_BLTSTRETCH;
		dxHelCaps += RDCAPS_SRCBLTCKEY;
		renderCaps = RDBLTFX_MOUSEBLT | RDBLTFX_ARITHMETICSTRETCH | RDBLTFX_EDGEBLEND |
						 RDBLTFX_SHADOWBLEND | RDBLTFX_FLATALPHA | RDBLTFX_GRADEDALPHA;
	}
	else
	{
		if ((helCaps.dwCaps & DDCAPS_BLT == 0) || 
			(helCaps.dwCaps & DDCAPS_BLTSTRETCH == 0) ||
			(helCaps.dwCaps & DDCAPS_COLORKEY == 0) ||
			(helCaps.dwCKeyCaps & DDCKEYCAPS_SRCBLT == 0))
		{
			RestoreDisplay();
			return(RDERR_NOEMULATION);
		}

//		if (driverCaps.dwCaps & DDCAPS_BLTSTRETCH)
//		{
//			if (driverCaps.dwFXCaps & (DDFXCAPS_BLTSHRINKX + DDFXCAPS_BLTSHRINKY + DDFXCAPS_BLTSTRETCHX + DDFXCAPS_BLTSTRETCHY) == 
//				DDFXCAPS_BLTSHRINKX + DDFXCAPS_BLTSHRINKY + DDFXCAPS_BLTSTRETCHX + DDFXCAPS_BLTSTRETCHY)
//				dxHalCaps += RDCAPS_BLTSTRETCH;
//			else if (helCaps.dwCaps & DDCAPS_BLTSTRETCH)
//				dxHelCaps += RDCAPS_BLTSTRETCH;
//			else 
//				return RDERR_DDRAWNOEMULATION;
//		}
//		else if (helCaps.dwCaps & DDCAPS_BLTSTRETCH)
//			dxHelCaps += RDCAPS_BLTSTRETCH;
//		else
//			return(RDERR_DDRAWNOEMULATION);

		if (helCaps.dwCaps & DDCAPS_BLTSTRETCH)
			dxHelCaps += RDCAPS_BLTSTRETCH;
		else
			return(RDERR_DDRAWNOEMULATION);

		if ((driverCaps.dwCaps & DDCAPS_BLT) && (driverCaps.dwCaps & DDCAPS_COLORKEY) && (driverCaps.dwCKeyCaps & DDCKEYCAPS_SRCBLT))
			dxHalCaps += RDCAPS_SRCBLTCKEY;
		else if ((helCaps.dwCaps & DDCAPS_BLT) && (helCaps.dwCaps & DDCAPS_COLORKEY) && (helCaps.dwCKeyCaps & DDCKEYCAPS_SRCBLT))
			dxHelCaps += RDCAPS_SRCBLTCKEY;
		else
			return(RDERR_DDRAWNOEMULATION);


		
		// Do computer speed testing here to set bits. - this is the path we go through:
		//
		//	if (Can everything be done in hardware?)
		//		renderCaps = RDBLTFX_ALLHARDWARE;
		//	else
		//		if (Everything fast enough in software)
		//			turn everything on in software
		//		else
		//			Turn blending off
		//			if (can everything but blending be done in hardware?)
		//				renderCaps = RDBLTFX_ALLHARDWARE
		//			else
		//				if (everything but blending fast enough in software)
		//					Do everything but blending in software
		//				else
		//					Turn off sprite effects
		//				endif
		//			endif
		//		endif
		//	endif




		if ((driverCaps.dwCaps & DDCAPS_BLT) && (driverCaps.dwCaps & DDCAPS_COLORKEY) && (driverCaps.dwCKeyCaps & DDCKEYCAPS_SRCBLT)
			&& (driverCaps.dwCaps & DDSCAPS_ALPHA) && (driverCaps.dwAlphaBltConstBitDepths))
			renderCaps = RDBLTFX_ALLHARDWARE | RDBLTFX_GRADEDALPHA | RDBLTFX_FLATALPHA;
		else if	((driverCaps.dwCaps & DDCAPS_BLT) && (driverCaps.dwCaps & DDCAPS_COLORKEY) && (driverCaps.dwCKeyCaps & DDCKEYCAPS_SRCBLT))
			renderCaps = RDBLTFX_ALLHARDWARE;
		else
			renderCaps = RDBLTFX_MOUSEBLT | RDBLTFX_ARITHMETICSTRETCH | RDBLTFX_EDGEBLEND |
						 RDBLTFX_SHADOWBLEND | RDBLTFX_FLATALPHA | RDBLTFX_GRADEDALPHA;

		if (QueryPerformanceFrequency(&timerFrequency) == TRUE)
			if (timerFrequency.QuadPart > 700000)
				renderCaps = RDBLTFX_MOUSEBLT | RDBLTFX_ARITHMETICSTRETCH | RDBLTFX_EDGEBLEND |
						 RDBLTFX_SHADOWBLEND | RDBLTFX_FLATALPHA | RDBLTFX_GRADEDALPHA;

	//	if ((driverCaps.dwCaps & DDCAPS_BLT) && (driverCaps.dwCaps & DDCAPS_COLORKEY) && (driverCaps.dwCKeyCaps & DDCKEYCAPS_SRCBLT))
	//		renderCaps = RDBLTFX_MOUSEBLT | RDBLTFX_ARITHMETICSTRETCH | RDBLTFX_EDGEBLEND |
	//					 RDBLTFX_SHADOWBLEND | RDBLTFX_FLATALPHA | RDBLTFX_GRADEDALPHA | RDBLTFX_FGPARALLAX;
	//	else
	//		renderCaps = RDBLTFX_MOUSEBLT | RDBLTFX_ARITHMETICSTRETCH | RDBLTFX_EDGEBLEND |
	//					 RDBLTFX_SHADOWBLEND | RDBLTFX_FLATALPHA | RDBLTFX_GRADEDALPHA;

		//	renderCaps = RDBLTFX_MOUSEBLT | RDBLTFX_SHADOWBLEND | RDBLTFX_FLATALPHA | RDBLTFX_ALLHARDWARE;
	}
*/
	return(RD_OK);

}
//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------



int32 RenderHard(void)
{
	warning("stub RenderHard");
/*
	if (renderCaps & RDBLTFX_ALLHARDWARE)
		return(RDERR_ALREADYON);
	if	((driverCaps.dwCaps & DDCAPS_BLT) && (driverCaps.dwCaps & DDCAPS_COLORKEY) && (driverCaps.dwCKeyCaps & DDCKEYCAPS_SRCBLT))
		renderCaps = RDBLTFX_ALLHARDWARE;
	else
		return(RDERR_NOHARDWARE);

*/
	return(RD_OK);
}

int32 RenderSoft(void)
{
	warning("stub RenderSoft");
/*
	if (!(renderCaps & RDBLTFX_ALLHARDWARE))
		return(RDERR_ALREADYON);
	renderCaps = RDBLTFX_MOUSEBLT | RDBLTFX_ARITHMETICSTRETCH | RDBLTFX_EDGEBLEND |
			 RDBLTFX_SHADOWBLEND | RDBLTFX_FLATALPHA | RDBLTFX_GRADEDALPHA;
*/
	return(RD_OK);
}

int32 SetBltFx(void)
{
	renderCaps |= RDBLTFX_EDGEBLEND + RDBLTFX_ARITHMETICSTRETCH;
	return(RD_OK);
}

int32 ClearBltFx(void)
{
	renderCaps &= (0xffffffff - RDBLTFX_EDGEBLEND - RDBLTFX_ARITHMETICSTRETCH);
	return(RD_OK);
}

int32 ClearShadowFx(void)
{
	renderCaps &= (0xffffffff - RDBLTFX_SHADOWBLEND);
	return(RD_OK);
}

int32 SetShadowFx(void)
{
	renderCaps |= RDBLTFX_SHADOWBLEND;
	return RD_OK;
}

int32 GetRenderType(void)
{
	if (renderCaps & RDBLTFX_ALLHARDWARE)
	{
		return (0);
	}
	else
	{
		if (renderCaps & (RDBLTFX_EDGEBLEND + RDBLTFX_ARITHMETICSTRETCH))
			return (3);
		else
		{
			if (renderCaps & RDBLTFX_SHADOWBLEND)
				return(2);
			else
				return (1);
		}
	}
}

int32 FlipScreens(void)

{
	warning("stub FlipScreens");
/*
    HRESULT		hr;
	BOOL		vbl;
	int32		startTime;

	DrawMouse();

	if (bFullScreen)
	{
		startTime = timeGetTime();

		while(TRUE)
	    {
			if (!noVbl)
			{
				hr = IDirectDraw2_GetVerticalBlankStatus(lpDD2, &vbl);
				if (hr != DD_OK)
				{
					DirectDrawError("Vertical blank status unavailable", hr);
				}
			}

			if (vbl || noVbl)
			{
				hr = IDirectDrawSurface2_Flip(lpPrimarySurface, NULL, 0);
				if (hr == DD_OK)
    			{
					break;
				}

				if (hr == DDERR_SURFACELOST)
				{
					if (gotTheFocus)
					{
    					hr = IDirectDrawSurface2_Restore(lpPrimarySurface);
					
        				if(hr != DD_OK)
	          			{
							if (++failCount == 32)
								return(RDERR_CANNOTFLIP);
						}
					}

				}
				else
					failCount = 0;

				if(hr != DDERR_WASSTILLDRAWING)
		  		{
				    break;
				}
			}
			if (timeGetTime() - startTime > 20)
			{
				noVbl = 1;
			}
		}
    }
	else
	{

		hr = IDirectDrawSurface2_Blt(lpPrimarySurface, &rcWindow, lpBackBuffer, NULL, DDBLT_WAIT, NULL);
		if (hr != DD_OK)
		{
			return(RDERR_UNKNOWN);
		}
	}
*/
	return(RD_OK);

}



int32 WaitForVbl(void)
{
	warning("stub WaitForVbl");
/*
	BOOL		vbl;
	HRESULT		hr;
	uint32		counter = 0;


	while(1)
	{
		hr = IDirectDraw2_GetVerticalBlankStatus(lpDD2, &vbl);
		
		if (hr != DD_OK)
		{
			DirectDrawError("Cannot get vertical blank status", hr);
			return(hr);
		}

		if (vbl || noVbl)
			break;
		
		if (++counter == 250000)
			noVbl = 1;
	}
*/	
	return(RD_OK);

}




int32 EraseBackBuffer( void )
{
	warning("stub EraseBackBuffer");
/*
    DDBLTFX     ddbltfx;
    HRESULT     hr;
	RECT		r = {0, 0, screenWide, screenDeep};


    //	Erase the background
    ddbltfx.dwSize = sizeof(ddbltfx);
    ddbltfx.dwFillColor = 0;

    while( 1 )
    {

        hr = IDirectDrawSurface2_Blt(lpBackBuffer, &r, NULL, NULL, DDBLT_COLORFILL, &ddbltfx );

		if (hr == DDERR_SURFACELOST)
        {

			RestoreSurfaces();

	        hr = IDirectDrawSurface2_Blt(lpBackBuffer, &r, NULL, NULL, DDBLT_COLORFILL, &ddbltfx );

            if (hr != DD_OK)
            {
				if (++failCount == 32)
				{
	                DirectDrawError("Cannot render back buffer", hr);
					return(hr);
				}
				else
				{
					failCount = 0;
					return(RD_OK);
				}
            }

        }

		if (hr == DD_OK)
        {
            break;
        }
    
		if (hr != DDERR_WASSTILLDRAWING)
        {
            DirectDrawError("Cannot render back buffer", hr);
			return(hr);
        }
    }
*/	
	return(RD_OK);

}




int32 SaveScreenShot(uint8 *buffer, uint8 *palette)
{

	static uint16 pcxCount = 0;
  int virtualWidth;
  int pix;
  int keyPix;
  int line;
  int i;
  int runLength;
  char filename[80];
  unsigned char ch;
  unsigned char *pcxData;
  unsigned char pal[256*3];
  FILE *fp;
  _pcxHeader pcxHead;

  sprintf(filename, "snap%.4d.pcx", pcxCount);

  fp = fopen(filename, "wb");
  if (fp == NULL)
  {
    sprintf(filename, "c:\\snap%.4d.pcx", pcxCount);
    fp = fopen(filename, "wb");
    if (fp == NULL)
    {
      return(0);
    }
  }
  pcxCount += 1;

  // Set up and write the header
  pcxHead.manufacturer = 0x0a;
  pcxHead.version = 5;
  pcxHead.encoding = 1;
  pcxHead.bitsPerPixel = 8;
  pcxHead.xmin = 0;
  pcxHead.ymin = 0;
  pcxHead.xmax = 639;
  pcxHead.ymax = 479;
  pcxHead.hres = 72;
  pcxHead.vres = 72;
  pcxHead.reserved = 0;
  pcxHead.colourPlanes = 1;
  pcxHead.bytesPerLine = 640;
  pcxHead.paletteType = 1;
  fwrite(&pcxHead , sizeof(pcxHead), 1, fp);

  // The simplest job is to write out the entire file as a series of single units

  virtualWidth = 640;
//  char *pcxDataBase = buffer;           //GetBitMap() + (GetHeight()-1)* virtualWidth ;
  for (line = 0 ; line < 480; line++)
  {       
          pcxData = (unsigned char *)buffer;   //pcxDataBase;

          // Look to compress this line of 'width' pixels
          pix = 0;
          while (pix < 640)
          {       // Look for some run length coding
                  keyPix = pcxData[pix++];
                  runLength = 1;
                  while ( (pix < 640) && (keyPix == pcxData[pix]) )
                  {       runLength++;
                          pix++;
                  }
                  while (runLength > 1)
                  {       // We have a run length bit. Runs are a maximum of 0x3f
                          int lRun = runLength > 0x3f ? 0x3f : runLength;
                          runLength -= lRun;
                          lRun |= 0xc0;
                          ch = (unsigned char) lRun;
                          fwrite(&ch, 1, 1, fp);
                          ch = (unsigned char) keyPix;
                          fwrite(&ch, 1, 1, fp);
//                          fFile.WriteChar(lRun);
//                          fFile.WriteChar(keyPix);
                  }
                  if (runLength)
                  {       // Single pixel. If its <= 0x3f it goes straight in, otherwise it is a single run length
                          if (keyPix < 0xc0)
                          {
                                  ch = (unsigned char) keyPix;
                                  fwrite(&ch, 1, 1, fp);
//                                  fFile.WriteChar(keyPix);
                          }
                          else
                          {
                             ch = 0xc1;
                             fwrite(&ch, 1, 1, fp);
                             ch = (unsigned char) keyPix;
                             fwrite(&ch, 1, 1, fp);
//                                  fFile.WriteChar(0xc1);
//                                  fFile.WriteChar(keyPix);
                          }
                  }
          }
//          pcxDataBase -= virtualWidth;
          buffer += virtualWidth;
  }

  // Convert and write out the palette
//  StringClass sPal(768);
//  unsigned char *pal = palette;
//  for (int count = 0 ; count < 256 ; count++)
//  {       *(pal++) = bmi.bmiColors[count].rgbRed;
//          *(pal++) = bmi.bmiColors[count].rgbGreen;
//          *(pal++) = bmi.bmiColors[count].rgbBlue;
//  }
  ch = 0x0c;
  fwrite(&ch, 1, 1, fp);

  for (i=0; i<256*3; i++)
  {
    pal[i] = *((unsigned char *) palette + i);
  }

//  fFile.WriteChar(0x0c);
  fwrite(pal, 256*3, 1, fp);
//  if (fFile.Write(sPal , 768)!=768)
//          return(-1);
  fclose(fp);

  return(1);

}






int32 GrabScreenShot(void)

{
	warning("stub GrabScreenShot");
/*
	uint8			*screenGrabBuffer;
	uint8			*palette;
	DDSURFACEDESC	ddsd;
	HRESULT			hr;
	int32			i;


	screenGrabBuffer = (uint8 *) malloc(screenWide * screenDeep);
	if (screenGrabBuffer == NULL)
		return(RDERR_OUTOFMEMORY);

	ddsd.dwSize = sizeof(DDSURFACEDESC);
	hr = IDirectDrawSurface2_Lock(lpPrimarySurface, NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	if (hr != DD_OK)
	{
		free(screenGrabBuffer);
		return(RDERR_LOCKFAILED);
	}
	
	for (i=0; i<screenDeep; i++)
	{
		memcpy(screenGrabBuffer + i * screenWide, (uint8 *) ddsd.lpSurface + ddsd.lPitch * i, screenWide);
	}
	IDirectDrawSurface2_Unlock(lpPrimarySurface, ddsd.lpSurface);


	palette = (uint8 *) malloc(256 * 3);
	if (palette == NULL)
	{
		free(screenGrabBuffer);
		return(RDERR_OUTOFMEMORY);
	}

	for (i=0; i<256; i++)
	{
		palette[i*3] = palCopy[i][0];
		palette[i*3+1] = palCopy[i][1];
		palette[i*3+2] = palCopy[i][2];
	}

	hr = SaveScreenShot(screenGrabBuffer,palette);


	free(palette);
	free(screenGrabBuffer);
*/
	return(RD_OK);
}



int32 NextSmackerFrame(void)
{
	warning("stub NextSmackerFrame");
	return(RD_OK);
}


uint32 textSurface = 0;

void OpenTextObject(_movieTextObject *obj)
{
	CreateSurface(obj->textSprite, &textSurface);
}

void CloseTextObject(_movieTextObject *obj)
{
	DeleteSurface(textSurface);
	textSurface = 0;
}

void DrawTextObject(_movieTextObject *obj)
{
	warning("stub DrawTextObject");
/*
	HRESULT				hr;
	RECT				rd, rs;
	LPDIRECTDRAWSURFACE	dds;
	_spriteInfo *s = obj->textSprite;
	char myString[256];


	dds = (LPDIRECTDRAWSURFACE) textSurface;

	// Set startx and starty for the screen buffer		ADDED THIS!
	if (s->type & RDSPR_DISPLAYALIGN)
		rd.top = s->y;
	else
		rd.top = s->y - scrolly;
		
	if (s->type & RDSPR_DISPLAYALIGN)
		rd.left = s->x;
	else
		rd.left = s->x - scrollx;

	rs.left = 0;
	rs.right = s->w;
	rs.top = 0;
	rs.bottom = s->h;
	if (s->scale & 0xff)
	{
		rd.right = rd.left + s->scaledWidth;
		rd.bottom = rd.top + s->scaledHeight;
		// Do clipping
		if (rd.top < 40)
		{
			rs.top = (40 - rd.top) * 256 / s->scale;
			rd.top = 40;
		}
		if (rd.bottom > 440)
		{
			rs.bottom -= ((rd.bottom - 440) * 256 / s->scale);
			rd.bottom = 440;
		}
		if (rd.left < 0)
		{
			rs.left = (0 - rd.left) * 256 / s->scale;
			rd.left = 0;
		}
		if (rd.right > 640)
		{
			rs.right -= ((rd.right - 640) * 256 / s->scale);
			rd.right = 640;
		}
	}
	else
	{
		rd.right = rd.left + s->w;
		rd.bottom = rd.top + s->h;

		// Do clipping
		if (rd.top < 40)
		{
			rs.top = 40 - rd.top;
			rd.top = 40;
		}
		if (rd.bottom > 440)
		{
			rs.bottom -= (rd.bottom - 440);
			rd.bottom = 440;
		}
		if (rd.left < 0)
		{
			rs.left = 0 - rd.left;
			rd.left = 0;
		}
		if (rd.right > 640)
		{
			rs.right -= (rd.right - 640);
			rd.right = 640;
		}
	}

	if (s->type & RDSPR_TRANS)
	{
		hr = IDirectDrawSurface2_Blt(lpPrimarySurface, &rd, dds, &rs, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
		if (hr)
		{
			if (hr == DDERR_SURFACELOST)
				hr = RDERR_SURFACELOST;
			else if (dxHalCaps & RDCAPS_BLTSTRETCH)
				dxHalCaps -= RDCAPS_BLTSTRETCH;
			else
			{
				sprintf(myString, "Cannot print smacker text x%d y%d w%d h%d s%d t%d\n", s->x, s->y, s->w, s->h, s->scale, s->type);
				DirectDrawError(myString, hr);
			}
		}
	}
	else
	{
		hr = IDirectDrawSurface2_Blt(lpPrimarySurface, &rd, dds, &rs, DDBLT_WAIT, NULL);
		if (hr)
		{
			if (hr == DDERR_SURFACELOST)
				hr = RDERR_SURFACELOST;
			else
			{
				sprintf(myString, "Cannot print smacker text x%d y%d w%d h%d s%d t%d\n", s->x, s->y, s->w, s->h, s->scale, s->type);
				DirectDrawError(myString, hr);
			}
		}
	}
*/
}


extern uint8 musicMuted;

int32 PlaySmacker(char *filename, _movieTextObject *text[], uint8 *musicOut)
{
	warning("stub PlaySmacker %s", filename);
	return(RD_OK);

}



void GetDrawStatus(_drvDrawStatus *s)
{
//	s->hwnd				= hwnd;
//	s->lpDraw			= lpDraw;
//	s->lpDD2			= lpDD2;
//	s->lpPrimarySurface = lpPrimarySurface;
//	s->lpBackBuffer		= lpBackBuffer;
//	s->lpPalette		= lpPalette;
	s->screenDeep		= screenDeep;
	s->screenWide		= screenWide;
	s->scrollx			= scrollx;
	s->scrolly			= scrolly;
	s->scrollxTarget	= scrollxTarget;
	s->scrollyTarget	= scrollyTarget;
	s->scrollxOld		= scrollxOld;
	s->scrollyOld		= scrollyOld;
	s->failCount		= failCount;
	s->renderCaps		= renderCaps;
	s->dxHalCaps		= dxHalCaps;
	s->dxHelCaps		= dxHelCaps;
	s->noVbl			= noVbl;
	s->bFullScreen		= bFullScreen;
	
//	memcpy(&s->driverCaps,	 &driverCaps,	sizeof(DDCAPS));
//	memset(&blackColorKey, 0, sizeof(DDCOLORKEY));
}



void SetDrawStatus(_drvDrawStatus *s)
{
//	hwnd			= s->hwnd;
//	lpDraw			= s->lpDraw;
//	lpDD2			= s->lpDD2;
//	lpPrimarySurface= s->lpPrimarySurface;
//	lpBackBuffer	= s->lpBackBuffer;
//	lpPalette		= s->lpPalette;
	screenDeep		= s->screenDeep;
	screenWide		= s->screenWide;
	scrollx			= s->scrollx;
	scrolly			= s->scrolly;
	scrollxTarget	= s->scrollxTarget;
	scrollyTarget	= s->scrollyTarget;
	scrollxOld		= s->scrollxOld;
	scrollyOld		= s->scrollyOld;
	failCount		= s->failCount;
//	renderCaps		= s->renderCaps;
	dxHalCaps		= s->dxHalCaps;
	dxHelCaps		= s->dxHelCaps;
	noVbl			= s->noVbl;
	bFullScreen		= s->bFullScreen;
	
//	memcpy(&driverCaps,	 &s->driverCaps,	sizeof(DDCAPS));
//	memset(&blackColorKey, 0, sizeof(DDCOLORKEY));
}
