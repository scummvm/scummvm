/** @file movies.cpp
	@brief
	This file contains movies routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flamovies.h"
#include "screens.h"
#include "sdlengine.h"
#include "main.h"
#include "sound.h"
#include "music.h"
#include "filereader.h"
#include "lbaengine.h"
#include "keyboard.h"

/** Config movie types */
#define CONF_MOVIE_NONE    0
#define CONF_MOVIE_FLA     1
#define CONF_MOVIE_FLAWIDE 2
#define CONF_MOVIE_FLAPCX  3

/** FLA movie extension */
#define FLA_EXT ".fla"

/** FLA Frame Opcode types */
enum FlaFrameOpcode {
	kLoadPalette	= 0,
	kFade			= 1,
	kPlaySample		= 2,
	kStopSample		= 4,
	kDeltaFrame		= 5,
	kKeyFrame		= 7
};

/** Auxiliar FLA fade out variable */
int32 _fadeOut;
/** Auxiliar FLA fade out variable to count frames between the fade */
int32 fadeOutFrames;

/** FLA movie sample auxiliar table */
int32 flaSampleTable[100];
/** Number of samples in FLA movie */
int32 samplesInFla;
/** Auxiliar work video buffer */
uint8* workVideoBufferCopy;
/** FLA movie header data */
FLAHeaderStruct flaHeaderData;
/** FLA movie header data */
FLAFrameDataStruct frameData;

FileReader frFla;

/** FLA movie draw key frame
	@param ptr FLA frame buffer pointer
	@param width FLA movie width
	@param height FLA movie height */
void drawKeyFrame(uint8 * ptr, int32 width, int32 height) {
	int32 a, b;
	uint8 * destPtr = (uint8 *)flaBuffer;
	uint8 * startOfLine = destPtr;
	int8 flag1;
	int8 flag2;

	do {
		flag1 = *(ptr++);

		for (a = 0; a < flag1; a++) {
			flag2 = *(ptr++);

			if (flag2 < 0) {
				flag2 = - flag2;
				for (b = 0; b < flag2; b++) {
					*(destPtr++) = *(ptr++);
				}
			} else {
				char colorFill;

				colorFill = *(ptr++);

				for (b = 0; b < flag2; b++) {
					*(destPtr++) = colorFill;
				}
			}
		}

		startOfLine = destPtr = startOfLine + width;
	} while (--height);
}

/** FLA movie draw delta frame
	@param ptr FLA frame buffer pointer
	@param width FLA movie width */
void drawDeltaFrame(uint8 * ptr, int32 width) {
	int32 a, b;
	uint16 skip;
	uint8 * destPtr;
	uint8 * startOfLine;
	int32 height;

	int8 flag1;
	int8 flag2;

	skip = *((uint16*)ptr);
	ptr += 2;
	skip *= width;
	startOfLine = destPtr = (uint8 *)flaBuffer + skip;
	height = *((int16*)ptr);
	ptr += 2;

	do {
		flag1 = *(ptr++);

		for (a = 0; a < flag1; a++) {
			destPtr += (unsigned char) * (ptr++);
			flag2 = *(ptr++);

			if (flag2 > 0) {
				for (b = 0; b < flag2; b++) {
					*(destPtr++) = *(ptr++);
				}
			} else {
				char colorFill;
				flag2 = - flag2;

				colorFill = *(ptr++);

				for (b = 0; b < flag2; b++) {
					*(destPtr++) = colorFill;
				}
			}
		}

		startOfLine = destPtr = startOfLine + width;
	} while (--height);
}

/** Scale FLA movie 2 times

	According with the settins we can put the original aspect radio stretch
	to fullscreen or preserve it and use top and button black bars */
void scaleFla2x() {
	int32 i, j;
	uint8* source = (uint8*)flaBuffer;
	uint8* dest = (uint8*)workVideoBuffer;

	if (cfgfile.Movie == CONF_MOVIE_FLAWIDE) {
		for (i = 0; i < SCREEN_WIDTH / SCALE*40; i++) {
			*(dest++) = 0x00;
		}
	}

	for (i = 0; i < FLASCREEN_HEIGHT; i++) {
		for (j = 0; j < FLASCREEN_WIDTH; j++) {
			*(dest++) = *(source);
			*(dest++) = *(source++);
		}
		if (cfgfile.Movie == CONF_MOVIE_FLAWIDE) { // include wide bars
			memcpy(dest, dest - SCREEN_WIDTH / SCALE, FLASCREEN_WIDTH*2);
			dest += FLASCREEN_WIDTH * 2;
		} else { // stretch the movie like original game.
			if (i % (2)) {
				memcpy(dest, dest - SCREEN_WIDTH / SCALE, FLASCREEN_WIDTH*2);
				dest += FLASCREEN_WIDTH * 2;
			}
			if (i % 10) {
				memcpy(dest, dest - SCREEN_WIDTH / SCALE, FLASCREEN_WIDTH*2);
				dest += FLASCREEN_WIDTH * 2;
			}
		}
	}

	if (cfgfile.Movie == CONF_MOVIE_FLAWIDE) {
		for (i = 0; i < SCREEN_WIDTH / SCALE*40; i++) {
			*(dest++) = 0x00;
		}
	}
}

/** FLA movie process frame */
void processFrame() {
	FLASampleStruct sample;
	uint32 opcodeBlockSize;
	uint8 opcode;
	int32 aux = 0;
	uint8 * ptr;

	frread(&frFla, &frameData.videoSize, 1);
	frread(&frFla, &frameData.dummy, 1);
	frread(&frFla, &frameData.frameVar0, 4);

	frread(&frFla, workVideoBufferCopy, frameData.frameVar0);

	if ((int32)frameData.videoSize <= 0)
		return;

	ptr = workVideoBufferCopy;

	do {
		opcode = *((uint8*)ptr);
		ptr += 2;
		opcodeBlockSize = *((uint16*)ptr);
		ptr += 2;

		switch (opcode - 1) {
		case kLoadPalette: {
			int16 numOfColor = *((int16*)ptr);
			int16 startColor = *((int16*)(ptr + 2));
			memcpy((palette + (startColor*3)), (ptr + 4), numOfColor*3);
			break;
		}
		case kFade: {
			// FLA movies don't use cross fade
			// fade out tricky
			if (_fadeOut != 1) {
				convertPalToRGBA(palette, paletteRGBACustom);
				fadeToBlack(paletteRGBACustom);
				_fadeOut = 1;
			}
			break;
		}
		case kPlaySample: {
			memcpy(&sample, ptr, sizeof(FLASampleStruct));
			playFlaSample(sample.sampleNum, sample.freq, sample.repeat, sample.x, sample.y);
			break;
		}
		case kStopSample: {
			stopSample(sample.sampleNum);
			break;
		}
		case kDeltaFrame: {
			drawDeltaFrame(ptr, FLASCREEN_WIDTH);
			if (_fadeOut == 1)
				fadeOutFrames++;
			break;
		}
		case kKeyFrame: {
			drawKeyFrame(ptr, FLASCREEN_WIDTH, FLASCREEN_HEIGHT);
			break;
		}
		default: {
			return;
		}
		}

		aux++;
		ptr += opcodeBlockSize;

	} while (aux < (int32)frameData.videoSize);
	//free(workVideoBufferCopy);
}

/** Play FLA PCX Screens
	@param flaName FLA movie name */
void fla_pcxList(int8 *flaName) {
	// TODO if is using FLA PCX than show the images instead
}

/** Play FLA movies
	@param flaName FLA movie name */
void playFlaMovie(int8 *flaName) {
	int32 i;
	int32 quit = 0;
	int32 currentFrame;
	int16 tmpValue;
	int8 fileNamePath[256];

	stopSamples();

	// Play FLA PCX instead of movies
	if (cfgfile.Movie == CONF_MOVIE_FLAPCX) {
		fla_pcxList(flaName);
		return;
	}

	stopMusic();

	// take extension if movie name has it
	for (i = 0; i < (int32)strlen(flaName); i++) {
		if(flaName[i] == '.') {
			flaName[i] = 0;
		}
	}

	sprintf(fileNamePath, FLA_DIR);
	strcat(fileNamePath, flaName);
	strcat(fileNamePath, FLA_EXT);

	_fadeOut = -1;
	fadeOutFrames = 0;

	if (!fropen2(&frFla, fileNamePath, "rb"))
		return;

	workVideoBufferCopy = workVideoBuffer;

	frread(&frFla, &flaHeaderData.version, 6);
	frread(&frFla, &flaHeaderData.numOfFrames, 4);
	frread(&frFla, &flaHeaderData.speed, 1);
	frread(&frFla, &flaHeaderData.var1, 1);
	frread(&frFla, &flaHeaderData.xsize, 2);
	frread(&frFla, &flaHeaderData.ysize, 2);

	frread(&frFla, &samplesInFla, 2);
	frread(&frFla, &tmpValue, 2);

	for (i = 0; i < samplesInFla; i++) {
		int16 var0;
		int16 var1;
		frread(&frFla, &var0, 2);
		frread(&frFla, &var1, 2);
		flaSampleTable[i] = var0;
	}

	if (!strcmp(flaHeaderData.version, "V1.3")) {
		currentFrame = 0;

		if (!quit) {
			do {
				if (currentFrame == flaHeaderData.numOfFrames)
					quit = 1;
				else {
					processFrame();
					scaleFla2x();
					copyScreen(workVideoBuffer, frontVideoBuffer);

					// Only blit to screen if isn't a fade
					if (_fadeOut == -1) {
						convertPalToRGBA(palette, paletteRGBACustom);
						if (!currentFrame) // fade in the first frame
							fadeIn(paletteRGBACustom);
						else
							setPalette(paletteRGBACustom);
					}

					// TRICKY: fade in tricky
					if (fadeOutFrames >= 2) {
						flip();
						convertPalToRGBA(palette, paletteRGBACustom);
						fadeToPal(paletteRGBACustom);
						_fadeOut = -1;
						fadeOutFrames = 0;
					}

					currentFrame++;

					fpsCycles(flaHeaderData.speed + 1);

					readKeys();

					if (skipIntro)
						break;
				}
			} while (!quit);
		}
	}

	if (cfgfile.CrossFade) {
		crossFade(frontVideoBuffer, paletteRGBACustom);
	} else {
		fadeToBlack(paletteRGBACustom);
	}

	stopSamples();
}

/*
void fla_pcxList(char *flaName)
{
	// check if FLAPCX file exist
//	if(!checkIfFileExist("FLA_PCX.HQR") || !checkIfFileExist("FLA_GIF.HQR")){
//		printf("FLA_PCX file doesn't exist!");
		//return;
	//}

	// TODO: done this with the HQR 23th entry (movies informations)
	if(!strcmp(flaName,"INTROD"))
	{
		prepareFlaPCX(1);
		WaitTime(5000);
		prepareFlaPCX(2);
		WaitTime(5000);
		prepareFlaPCX(3);
		WaitTime(5000);
		prepareFlaPCX(4);
		WaitTime(5000);
		prepareFlaPCX(5);
		WaitTime(5000);

	}
	else if(!strcmp(flaName,"BAFFE") || !strcmp(flaName,"BAFFE2") || !strcmp(flaName,"BAFFE3") || !strcmp(flaName,"BAFFE4"))
	{
		prepareFlaPCX(6);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"bateau") || !strcmp(flaName,"bateau2"))
	{
		prepareFlaPCX(7);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"flute2"))
	{
		prepareFlaPCX(8);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"navette"))
	{
		prepareFlaPCX(15);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"templebu"))
	{
		prepareFlaPCX(12);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"glass2"))
	{
		prepareFlaPCX(8);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"surf"))
	{
		prepareFlaPCX(9);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"verser") || !strcmp(flaName,"verser2"))
	{
		prepareFlaPCX(10);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"capture"))
	{
		prepareFlaPCX(14);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"neige2"))
	{
		prepareFlaPCX(11);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"sendel"))
	{
		prepareFlaPCX(14);
		WaitTime(5000);
	}
	else if(!strcmp(flaName,"sendel2"))
	{
		prepareFlaPCX(17);
		WaitTime(5000);
	}
}

void prepareFlaPCX(int index)
{
	int i;
	SDL_Surface *image;

	// TODO: Done this without SDL_Image Library
	if(checkIfFileExist("FLA_PCX.HQR"))
		image = IMG_LoadPCX_RW(SDL_RWFromMem(HQR_Get(HQR_FlaPCX,index), Size_HQR("FLA_PCX.HQR", index))); // rwop
	else if(checkIfFileExist("FLA_GIF.HQR"))
		image = IMG_LoadGIF_RW(SDL_RWFromMem(HQR_Get(HQR_FlaGIF,index), Size_HQR("fla_gif.hqr", index))); // rwop

	if(!image) {
		printf("Can't load FLA PCX: %s\n", IMG_GetError());
	}

	osystem_FlaPCXCrossFade(image);
}*/
