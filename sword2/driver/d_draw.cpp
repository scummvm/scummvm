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

#include <stdio.h>
#include "stdafx.h"
#include "driver96.h"
#include "rdwin.h"
#include "_mouse.h"
#include "d_draw.h"
#include "palette.h"

#define SCREENYOFFSET	40
#define MILLISECSPERCYCLE 83

byte *lpBackBuffer;

/*
static LPDIRECTDRAW		lpDraw;				// DirectDraw object
LPDIRECTDRAW2			lpDD2;				// DirectDraw2 object
LPDIRECTDRAWSURFACE		lpBackBuffer;		// DirectDraw back surface
LPDIRECTDRAWPALETTE		lpPalette = NULL;	// DirectDraw palette
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

void FatalDirectDrawError(char *str, int32 code, char *filename, int32 line) {
	char string[256];

	sprintf(string, "FATAL: %s - code 0x%.8x - file %s - line %d", str, code, filename, line);
	warning("%s", string);
}

int32 PlotDots(int16 x, int16 y, int16 count) {

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

int32 InitialiseDisplay(int16 width, int16 height, int16 colourDepth, int32 windowType) {
	screenWide = width;
	screenDeep = height;

	lpBackBuffer = (byte *) malloc(screenWide * screenDeep);
	return(RD_OK);
}

// FIXME: Clean up this mess. I don't want to add any new flags, but some of
// them should be renamed. Or maybe we should abandon the whole renderCaps
// thing and simply check the numeric value of the graphics quality setting
// instead.

// Note that SetTransFx() actually clears a bit. That's intentional.

void SetTransFx(void) {
	renderCaps &= ~RDBLTFX_ALLHARDWARE;
}

void ClearTransFx(void) {
	renderCaps |= RDBLTFX_ALLHARDWARE;
}

void SetBltFx(void) {
	renderCaps |= (RDBLTFX_EDGEBLEND | RDBLTFX_ARITHMETICSTRETCH);
}

void ClearBltFx(void) { 
	renderCaps &= ~(RDBLTFX_EDGEBLEND | RDBLTFX_ARITHMETICSTRETCH);
}

void SetShadowFx(void) {
	renderCaps |= RDBLTFX_SHADOWBLEND;
}

void ClearShadowFx(void) {
	renderCaps &= ~RDBLTFX_SHADOWBLEND;
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

int32 EraseBackBuffer( void ) {
	// Since the entire screen is redrawn each time, there probably isn't
	// any need to actually clear the back buffer.
	//
	// At the very least, since the menu code now is solely responsible
	// for its own parts of the screen, we'd only need to clear the
	// picture area.

	// memset(lpBackBuffer + MENUDEEP * screnWide, 0, screenWide * (screenDeep - 2 * MENUDEEP));
	return RD_OK;
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

int32 GrabScreenShot(void) {
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



int32 NextSmackerFrame(void) {
	warning("stub NextSmackerFrame");
	return(RD_OK);
}


uint8 *textSurface = NULL;

void OpenTextObject(_movieTextObject *obj) {
	CreateSurface(obj->textSprite, &textSurface);
}

void CloseTextObject(_movieTextObject *obj) {
	DeleteSurface(textSurface);
	textSurface = 0;
}

void DrawTextObject(_movieTextObject *obj) {
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

int32 PlaySmacker(char *filename, _movieTextObject *text[], uint8 *musicOut) {
	warning("stub PlaySmacker %s", filename);
	return(RD_OK);

}

void GetDrawStatus(_drvDrawStatus *s)
{
//	s->hwnd				= hwnd;
//	s->lpDraw			= lpDraw;
//	s->lpDD2			= lpDD2;
//	s->lpPrimarySurface = lpPrimarySurface;
	s->lpBackBuffer		= lpBackBuffer;
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
