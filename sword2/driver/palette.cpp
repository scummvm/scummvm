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
//	Filename	:	palette.c
//	Created		:	22nd August 1996
//	By			:	P.R.Porter
//
//	Summary		:	This module holds the palette functions and the interface
//					to the directDraw palette.
//
//	Version	Date		By		Description
//	------- ---------	---		-----------------------------------------------
//	1.0		16-Sep-96	PRP		Currently holds the palette setting code.  Will
//								be modified so that the calling functions have
//								to use the correct palette format, and the
//								fading code will be added.
//
//	1.1		17-Sep-96	PRP		Removed utypes.h from include list.
//
//	1.2		03-Oct-96	PRP		Changed the palette format to RGBA 0-255.
//
//	1.3		04-Oct-96	PRP		No changes
//
//	1.4		04-Oct-96	PRP		Put direct path in for ddraw.h
//
//	1.5		08-Nov-96	PRP		Created a drivers copy of the palette so that
//								it can be restored at any time without the
//								game engine having to worry about things.
//								Currently, there is different code for setting
//								the palette in full screen and windowed mode
//								until I can get windowed mode working.
//
//	1.6		09-Nov-96	PRP		More work to get the palette to work propery
//								in windowed mode.
//
//	1.7		22-Nov-96	PRP		Made the whole of the palette available to 
//								the rest of the driver96 code.
//
//	1.8		25-Nov-96	PRP		Added a function to create a palette match
//								table from the current palette.
//
//	1.9		29-Nov-96	PRP		Made the QuickMatch function __inline for
//								speed - but didn't work.  Added functions
//								to fade the palette up and down.
//
//	1.10	05-Dec-96	PRP		Added a flag to SetPalette so that the palette
//								can be set immediately or later by the fade.
//
//	1.11	06-Dec-96	JEL		fadeStatus now initialised to 0
//
//	1.12	09-Dec-96	PRP		Function UpdatePaletteMatchTable() changed so
//								that the data can be part of another file.
//
//	1.13	10-Feb-97	PRP		Changed the direct draw error reporting calls.
//
//	1.14	11-Feb-97	PRP		Suspended the fade server if the direct draw
//								palette object does not exist.
//
//	1.15	17-Mar-97	PRP		Added RDFADE_BLACK as a return value so the
//								engine knows the state of the palette when
//								there is no fade.
//
//	1.16	17-Mar-97	PRP		Fixed driver bug which caused FadeUp to fail
//								becuase of the introduction of RDFADE_BLACK.
//
//	1.17	26-Jun-97	PRP		Added a function to darken the palette for pausing.
//
//	1.18	26-Jun-97	PRP		Forgot to return a value.
//
//	1.19	26-Jun-97	PRP		Forgot to set the palette - DOH!
//
//	1.20	26-Jun-97	PRP		Fixed undefined symbols bugs.
//
//	1.21	09-Jul-97	JEL		Fixed palette dimming - to unsigned rather than signed RGB values!
//
//	1.22	26-Jun-97	JEL		SetPalette now always sets colour 0 to black so doesn't need setting to black from game engine
//
//	1.23	10-Jul-97	JEL		Nothing changed but new version had to be booked in anyway.
//
//	1.24	10-Jul-97	JEL		SetPalette doesn't do that any more (see above)!
//
//	Functions
//	---------
//
//	--------------------------------------------------------------------------
//
//	void SetPalette(int32 startEntry, int32 noEntries, uint8 *colourTable)
//
//	Sets the palette from position startEntry for noEntries, to the data 
//	pointed to by colourTable.
//
//	--------------------------------------------------------------------------
//
//	void UpdatePaletteMatchTable(uint8 *data)
//
//	Uses the current palCopy to create a table of palette indeces which will
//	be searched later for a quick palette match, if data is NULL.  Otherwise
//	it uses the table passed in.
//
//	--------------------------------------------------------------------------
//
//	uint8 QuickMatch(uint8 r, uint8 g, uint8 b)
//
//	Returns the palette index of the closest matching colour in the palette
//	to these RGB values.
//
//	--------------------------------------------------------------------------
//
//	int32 FadeUp(float time)
//
//	Fades the palette up from black to the current palette in time.
//
//	--------------------------------------------------------------------------
//
//	int32 FadeDown(float time)
//
//	Fades the palette down to black from the current palette in time.
//
//	--------------------------------------------------------------------------
//
//	uint8 GetFadeStatus(void)
//
//	Returns the fade status which can be one of RDFADE_UP, RDFADE_DOWN or
//	RDFADE_NONE.
//
//=============================================================================


#define WIN32_LEAN_AND_MEAN

#include "stdafx.h"
//#include <windows.h>
//#include <windowsx.h>
#include <stdio.h>
//#include <mmsystem.h>


#include "common/util.h"
#include "common/engine.h"
#include "d_draw.h"
#include "driver96.h"
#include "../sword2.h"

#define PALTABLESIZE 64*64*64


uint8 palCopy[256][4];
uint8 fadePalette[256][4];

uint8 paletteMatch[PALTABLESIZE];

uint8 fadeStatus=0;

static int32 fadeStartTime;
static int32 fadeTotalTime;


//	--------------------------------------------------------------------------
//	int32 RestorePalette(void)
//
//	This function restores the palette, and should be called whenever the
//	screen mode changes, or something like that.
//	--------------------------------------------------------------------------
int32 RestorePalette(void)

{
	warning("stub RestorePalette");
/*
	int32 hr;

	
	if (bFullScreen)
		hr = IDirectDrawPalette_SetEntries(lpPalette, 0, 0, 256, (LPPALETTEENTRY) &palCopy[0][0]);
	else
		hr = IDirectDrawPalette_SetEntries(lpPalette, 0, 10, 236, (LPPALETTEENTRY) &palCopy[10][0]);

	if (hr != DD_OK)
	{
		DirectDrawError("Unable to restore palette", hr);
		return(hr);
	}

*/
	return(RD_OK);

}


uint8 GetMatch(uint8 r, uint8 g, uint8 b)

{

	int32 diff;
	int32 min;
	int16 diffred, diffgreen, diffblue;
	int16 i;
	uint8 minIndex;


	diffred = palCopy[0][0] - r;
	diffgreen = palCopy[0][1] - g;
	diffblue = palCopy[0][2] - b;

	diff = diffred * diffred + diffgreen * diffgreen + diffblue * diffblue;
	min = diff;
	minIndex = 0;
	if (diff > 0)
	{
		i = 1;
		while (i < 256)
		{
			diffred = palCopy[i][0] - r;
			diffgreen = palCopy[i][1] - g;
			diffblue = palCopy[i][2] - b;

			diff = diffred * diffred + diffgreen * diffgreen + diffblue * diffblue;
			if (diff < min)
			{
				min = diff;
				minIndex = (uint8) i;
				if (min == 0)
					break;
			}
			i += 1;
		}
	}

	// Here, minIndex is the index of the matchpalette which is closest.
	return(minIndex);

}



int32 UpdatePaletteMatchTable(uint8 *data)

{

	if (data == NULL)

	// Create palette match table
	{
		FILE *fp;
		int16 red, green, blue;
		uint8 *p;

		p = &paletteMatch[0];
		for (red = 0; red < 256; red += 4)
		{
			for (green = 0; green < 256; green += 4)
			{
				for (blue = 0; blue < 256; blue += 4)
				{
					*p++ = GetMatch((uint8) red, (uint8) green, (uint8) blue);
				}
			}
		}

		// Write out palette match table
		fp = fopen("r11.rgb", "wb");
		if (fp == NULL)
			return(RDERR_INVALIDFILENAME);
		if (fwrite(paletteMatch, 1, 64*64*64, fp) != 64*64*64)
			return(RDERR_WRITEERROR);
		fclose(fp);
	}
	else
	// Read table from file
	{

		memcpy(paletteMatch, data, PALTABLESIZE);
	
/*
		FILE *fp;

		fp = fopen(filename, "rb");
		if (fp == NULL)
			return(RDERR_INVALIDFILENAME);
		if (fread(paletteMatch, 1, 64*64*64, fp) != 64*64*64)
			return(RDERR_READERROR);
		fclose(fp);
		return(RD_OK);
*/		
	}
	return(RD_OK);

}



__inline uint8 QuickMatch(uint8 r, uint8 g, uint8 b)

{
	return(paletteMatch[((int32) (r >> 2) << 12) + ((int32) (g >> 2) << 6) + (b >> 2)]);
}




int32 SetPalette(int16 startEntry, int16 noEntries, uint8 *colourTable, uint8 fadeNow)

{
	warning("stub SetPalette( %d, %d, %d )", startEntry, noEntries, fadeNow);
/*

	int32 hr;

	if (noEntries == 0)
	{
		RestorePalette();
		return(RD_OK);
	}


	memcpy(&palCopy[startEntry][0], colourTable, noEntries * 4);

	if ((lpPalette) && (fadeNow == RDPAL_INSTANT))
	{
		if (bFullScreen)
			hr = IDirectDrawPalette_SetEntries(lpPalette, 0, startEntry, noEntries, (LPPALETTEENTRY) &palCopy[startEntry][0]);
		else
			hr = IDirectDrawPalette_SetEntries(lpPalette, 0, 10, 236, (LPPALETTEENTRY) &palCopy[10][0]);
		if (hr != DD_OK)
		{
			DirectDrawError("Unable to set palette entries", hr);
			return(hr);
		}
	}
*/
	return(RD_OK);

}


int32 DimPalette(void)
{
	warning("stub DimPalette");
/*
	uint8 *p;
	uint32 i;

	p = (uint8*) &palCopy[0][0];
	for (i=0; i<256*4; i++)
	{
		*p++ /= 2;
	}
	if (lpPalette)
		IDirectDrawPalette_SetEntries(lpPalette, 0, 0, 256, (LPPALETTEENTRY) &palCopy[0][0]);
*/
	return RD_OK;

}



int32 FadeUp(float time)

{

	if ((fadeStatus != RDFADE_BLACK) && (fadeStatus != RDFADE_NONE))
		return(RDERR_FADEINCOMPLETE);

	fadeTotalTime = (int32) (time * 1000);
	fadeStatus = RDFADE_UP;
	fadeStartTime = SVM_timeGetTime();

	return RD_OK;

}


int32 FadeDown(float time)

{

	if ((fadeStatus != RDFADE_BLACK) && (fadeStatus != RDFADE_NONE))
		return(RDERR_FADEINCOMPLETE);

	fadeTotalTime = (int32) (time * 1000);
	fadeStatus = RDFADE_DOWN;
	fadeStartTime = SVM_timeGetTime();

	return RD_OK;

}


uint8 GetFadeStatus(void)
{
	return(fadeStatus);
}


void FadeServer(void *)

{
	int32 currentTime;
	int16 fadeMultiplier;
	int16 i;

	StackLock lock(g_sword2->_paletteMutex);
	
	switch (fadeStatus) {
		case RDFADE_UP:
			currentTime = SVM_timeGetTime();
			if (currentTime >= fadeStartTime + fadeTotalTime) {
				fadeStatus = RDFADE_NONE;
				g_system->set_palette((const byte *) palCopy, 0, 256);
			} else {
				fadeMultiplier = (int16) (((int32) (currentTime - fadeStartTime) * 256) / fadeTotalTime);
				for (i=0; i<256; i++) {
					fadePalette[i][0] = (palCopy[i][0] * fadeMultiplier) >> 8;
					fadePalette[i][1] = (palCopy[i][1] * fadeMultiplier) >> 8;
					fadePalette[i][2] = (palCopy[i][2] * fadeMultiplier) >> 8;
				}
				g_system->set_palette((const byte *) fadePalette, 0, 256);
			}
			break;

		case RDFADE_DOWN:
			currentTime = SVM_timeGetTime();
			if (currentTime >= fadeStartTime + fadeTotalTime) {
				memset(fadePalette, 0, sizeof(fadePalette));
				fadeStatus = RDFADE_BLACK;
			} else {
				fadeMultiplier = (int16) (((int32) (fadeTotalTime - (currentTime - fadeStartTime)) * 256) / fadeTotalTime);
				for (i=0; i<256; i++) {
					fadePalette[i][0] = (palCopy[i][0] * fadeMultiplier) >> 8;
					fadePalette[i][1] = (palCopy[i][1] * fadeMultiplier) >> 8;
					fadePalette[i][2] = (palCopy[i][2] * fadeMultiplier) >> 8;
				}
			}
			g_system->set_palette((const byte *) fadePalette, 0, 256);
			break;
		}


/*
	int32 currentTime;
	int16 fadeMultiplier;
	int16 i;
	HRESULT hr;


	if (lpPalette == NULL)
		return;

	if (fadeStatus)
	{
		if (fadeStatus == RDFADE_UP)
		{
			currentTime = timeGetTime();
			if (currentTime >= fadeStartTime + fadeTotalTime)
			{
				fadeStatus = RDFADE_NONE;
				if (bFullScreen)
					hr = IDirectDrawPalette_SetEntries(lpPalette, 0, 0, 256, (LPPALETTEENTRY) &palCopy[0][0]);
				else
					hr = IDirectDrawPalette_SetEntries(lpPalette, 0, 10, 236, (LPPALETTEENTRY) &palCopy[10][0]);
			}
			else
			{
				fadeMultiplier = (int16) (((int32) (currentTime - fadeStartTime) * 256) / fadeTotalTime);
				for (i=0; i<256; i++)
				{
					fadePalette[i][0] = (palCopy[i][0] * fadeMultiplier) >> 8;
					fadePalette[i][1] = (palCopy[i][1] * fadeMultiplier) >> 8;
					fadePalette[i][2] = (palCopy[i][2] * fadeMultiplier) >> 8;
				}
				if (bFullScreen)
					hr = IDirectDrawPalette_SetEntries(lpPalette, 0, 0, 256, (LPPALETTEENTRY) &fadePalette[0][0]);
				else
					hr = IDirectDrawPalette_SetEntries(lpPalette, 0, 10, 236, (LPPALETTEENTRY) &fadePalette[10][0]);
			}
		}
		else if (fadeStatus == RDFADE_DOWN)
		{
			currentTime = timeGetTime();
			if (currentTime >= fadeStartTime + fadeTotalTime)
			{
				for (i=0; i<256; i++)
				{
					fadePalette[i][0] = 0;
					fadePalette[i][1] = 0;
					fadePalette[i][2] = 0;
				}
				fadeStatus = RDFADE_BLACK;
				if (bFullScreen)
					hr = IDirectDrawPalette_SetEntries(lpPalette, 0, 0, 256, (LPPALETTEENTRY) &fadePalette[0][0]);
				else
					hr = IDirectDrawPalette_SetEntries(lpPalette, 0, 10, 236, (LPPALETTEENTRY) &fadePalette[10][0]);
			}
			else
			{
				fadeMultiplier = (int16) (((int32) (fadeTotalTime - (currentTime - fadeStartTime)) * 256) / fadeTotalTime);
				for (i=0; i<256; i++)
				{
					fadePalette[i][0] = (palCopy[i][0] * fadeMultiplier) >> 8;
					fadePalette[i][1] = (palCopy[i][1] * fadeMultiplier) >> 8;
					fadePalette[i][2] = (palCopy[i][2] * fadeMultiplier) >> 8;
				}
				if (bFullScreen)
					hr = IDirectDrawPalette_SetEntries(lpPalette, 0, 0, 256, (LPPALETTEENTRY) &fadePalette[0][0]);
				else
					hr = IDirectDrawPalette_SetEntries(lpPalette, 0, 10, 236, (LPPALETTEENTRY) &fadePalette[10][0]);
			}
		}
	}
*/
}

