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
#include "bs2/header.h"		// HACK: For cutscenes instruction message
#include "bs2/memory.h"		// HACK: For cutscenes instruction message
#include "bs2/maketext.h"	// HACK: For cutscenes instruction message
#include "bs2/sword2.h"
#include "sound/mixer.h"
#include "rdwin.h"
#include "_mouse.h"
#include "d_draw.h"
#include "palette.h"
#include "render.h"

#define SCREENYOFFSET	40
#define MILLISECSPERCYCLE 83

byte *lpBackBuffer;

// Game screen metrics
int16			screenDeep;
int16			screenWide;


// Set to 1 if vertical blank status cannot be checked.
BOOL			bFullScreen = 0;



// Scroll variables.  scrollx and scrolly hold the current scroll position, 

int16			scrollx;
int16			scrolly;

int32			renderCaps = 0;
int32			dxHalCaps = 0;
int32			dxHelCaps = 0;

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


int32 EraseBackBuffer( void ) {
	memset(lpBackBuffer + MENUDEEP * screenWide, 0, screenWide * RENDERDEEP);
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
	if (obj->textSprite)
		CreateSurface(obj->textSprite, &textSurface);
}

void CloseTextObject(_movieTextObject *obj) {
	if (textSurface) {
		DeleteSurface(textSurface);
		textSurface = 0;
	}
}

void DrawTextObject(_movieTextObject *obj) {
	if (obj->textSprite && textSurface)
		DrawSurface(obj->textSprite, textSurface);
}

int32 PlaySmacker(char *filename, _movieTextObject *text[], uint8 *musicOut) {
	warning("semi-stub PlaySmacker %s", filename);

	// WORKAROUND: For now, we just do the voice-over parts of the
	// movies, since they're separate from the actual smacker files.

	// Do we really need to pre-cache the text sprites and speech data
	// like this? It'd be simpler to just store the text id and construct
	// the data as we go along.

	if (text) {
		uint8 oldPal[1024];
		uint8 tmpPal[1024];

		EraseBackBuffer();

		// HACK: Draw instructions
		//
		// I'm using the the menu area, because that's unlikely to be
		// touched by anything else during the cutscene.

		memset(lpBackBuffer, 0, screenWide * MENUDEEP);

		uint8 msg[] = "Cutscene - Press ESC to exit";
		mem *data = MakeTextSprite(msg, 640, 255, speech_font_id);
		_frameHeader *frame = (_frameHeader *) data->ad;
		_spriteInfo msgSprite;
		uint8 *msgSurface;

		msgSprite.x = screenWide / 2 - frame->width / 2;
		msgSprite.y = RDMENU_MENUDEEP / 2 - frame->height / 2;
		msgSprite.w = frame->width;
		msgSprite.h = frame->height;
		msgSprite.type = RDSPR_DISPLAYALIGN | RDSPR_NOCOMPRESSION | RDSPR_TRANS;
		msgSprite.data = data->ad + sizeof(_frameHeader);

		CreateSurface(&msgSprite, &msgSurface);
		DrawSurface(&msgSprite, msgSurface);
		DeleteSurface(msgSurface);
		Free_mem(data);

		// In case the cutscene has a long lead-in, start just before
		// the first line of text.

		int frameCounter = text[0]->startFrame - 12;
		int textCounter = 0;

		// Fake a palette that will hopefully make the text visible.
		// In the opening cutscene it seems to use colours 1 (black?)
		// and 255 (white?).
		//
		// The text should probably be colored the same as the rest of
		// the in-game text.

		memcpy(oldPal, palCopy, 1024);
		memset(tmpPal, 0, 1024);
		tmpPal[255 * 4 + 0] = 255;
		tmpPal[255 * 4 + 1] = 255;
		tmpPal[255 * 4 + 2] = 255;
		BS2_SetPalette(0, 256, tmpPal, RDPAL_INSTANT);

		PlayingSoundHandle handle = 0;

		while (1) {
			if (!text[textCounter])
				break;

			if (frameCounter == text[textCounter]->startFrame) {
				EraseBackBuffer();
				OpenTextObject(text[textCounter]);
				DrawTextObject(text[textCounter]);
				if (text[textCounter]->speech) {
					g_sword2->_mixer->playRaw(&handle, text[textCounter]->speech, text[textCounter]->speechBufferSize, 22050, SoundMixer::FLAG_16BITS);
				}
			}

			if (frameCounter == text[textCounter]->endFrame) {
				CloseTextObject(text[textCounter]);
				EraseBackBuffer();
				textCounter++;
			}

			frameCounter++;

			ServiceWindows();

			char key;

			if (ReadKey(&key) == RD_OK && key == 27) {
				g_sword2->_mixer->stopHandle(handle);
				break;
			}

			// Simulate ~12 frames per second. I don't know what
			// frame rate the original movies had, or even if it
			// was constant, but this seems to work reasonably.
			g_system->delay_msecs(80);
		}

		BS2_SetPalette(0, 256, oldPal, RDPAL_INSTANT);

		CloseTextObject(text[textCounter]);

		// HACK: Remove the instructions created above
		ScummVM::Rect r;

		memset(lpBackBuffer, 0, screenWide * MENUDEEP);
		r.left = r.top = 0;
		r.right = screenWide;
		r.bottom = MENUDEEP;
		UploadRect(&r);
	}

	return RD_OK;
}
