/** @file images.cpp
	@brief
	This file contains image processing.

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

#include "screens.h"
#include "resources.h"
#include "main.h"
#include "sdlengine.h"
#include "music.h"
#include "hqrdepack.h"
#include "lbaengine.h"


/** Load and display Adeline Logo */
void adelineLogo() {
	playMidiMusic(31, 0);

	loadImage(RESSHQR_ADELINEIMG, 1);
	delaySkip(7000);
	fadeOut(paletteRGBACustom);
	palCustom = 1;
}

/** Load and display Main Menu image */
void loadMenuImage(int16 fade_in) {
	hqrGetEntry(workVideoBuffer, HQR_RESS_FILE, RESSHQR_MENUIMG);
	copyScreen(workVideoBuffer, frontVideoBuffer);
	if (fade_in) {
		fadeToPal(paletteRGBA);
	} else {
		setPalette(paletteRGBA);
	}

	palCustom = 0;
}

/** Load a custom palette */
void loadCustomPalette(int32 index) {
	hqrGetEntry(palette, HQR_RESS_FILE, index);
	convertPalToRGBA(palette, paletteRGBACustom);
}

/** Load and display a particulary image on \a RESS.HQR file with cross fade effect
	@param index \a RESS.HQR entry index (starting from 0) */
void loadImage(int32 index, int16 fade_in) {
	hqrGetEntry(workVideoBuffer, HQR_RESS_FILE, index);
	copyScreen(workVideoBuffer, frontVideoBuffer);
	loadCustomPalette(index + 1);
	if (fade_in) {
		fadeToPal(paletteRGBACustom);
	} else {
		setPalette(paletteRGBACustom);
	}

	palCustom = 1;
}

/** Load and display a particulary image on \a RESS.HQR file with cross fade effect and delay
	@param index \a RESS.HQR entry index (starting from 0)
	@param time number of seconds to delay */
void loadImageDelay(int32 index, int32 time) {
	loadImage(index, 1);
	delaySkip(1000*time);
	fadeOut(paletteRGBACustom);
}

/** Converts in-game palette to SDL palette
	@param palSource palette source with RGB
	@param palDest palette destination with RGBA */
void convertPalToRGBA(uint8 * palSource, uint8 * palDest) {
	int i;

	for (i = 0; i < NUMOFCOLORS; i++) {
		palDest[0] = palSource[0];
		palDest[1] = palSource[1];
		palDest[2] = palSource[2];
		palDest += 4;
		palSource += 3;
	}
}

/** Fade image in
	@param palette current palette to fade in */
void fadeIn(uint8 * palette) {
	if (cfgfile.CrossFade)
		crossFade(frontVideoBuffer, palette);
	else
		fadeToPal(palette);

	setPalette(palette);
}

/** Fade image out
	@param palette current palette to fade out */
void fadeOut(uint8 * palette) {
	/*if(cfgfile.CrossFade)
		crossFade(frontVideoBuffer, palette);
	else
		fadeToBlack(palette);*/
	if (!cfgfile.CrossFade)
		fadeToBlack(palette);
}

/** Calculate a new color component according with an intensity
	@param modifier color compenent
	@param color color value
	@param param unknown
	@param intensity intensity value to adjust
	@return new color component*/
int32 crossDot(int32 modifier, int32 color, int32 param, int32 intensity) {
	if (!param)
		return (color);
	return (((color - modifier) * intensity) / param) + modifier;
}

/** Adjust palette intensity
	@param R red component of color
	@param G green component of color
	@param B blue component of color
	@param palette palette to adjust
	@param intensity intensity value to adjust */
void adjustPalette(uint8 R, uint8 G, uint8 B, uint8 * palette, int32 intensity) {
	uint8 localPalette[NUMOFCOLORS*4];
	uint8 *newR;
	uint8 *newG;
	uint8 *newB;
	uint8 *newA;

	int32 local;
	int32 counter = 0;
	int32 i;

	local = intensity;

	newR = &localPalette[0];
	newG = &localPalette[1];
	newB = &localPalette[2];
	newA = &localPalette[3];

	for (i = 0; i < NUMOFCOLORS; i++) {
		*newR = crossDot(R, palette[counter], 100, local);
		*newG = crossDot(G, palette[counter + 1], 100, local);
		*newB = crossDot(B, palette[counter + 2], 100, local);
		*newA = 0;

		newR += 4;
		newG += 4;
		newB += 4;
		newA += 4;

		counter += 4;
	}

	setPalette(localPalette);
}

/** Adjust between two palettes
	@param pal1 palette from adjust
	@param pal2 palette to adjust */
void adjustCrossPalette(uint8 * pal1, uint8 * pal2) {
	uint8 localPalette[NUMOFCOLORS*4];

	uint8 *newR;
	uint8 *newG;
	uint8 *newB;
	uint8 *newA;

	int32 i;
	int32 counter = 0;
	int32 intensity = 0;

	do
	{
		counter = 0;

		newR = &localPalette[counter];
		newG = &localPalette[counter + 1];
		newB = &localPalette[counter + 2];
		newA = &localPalette[counter + 3];

		for (i = 0; i < NUMOFCOLORS; i++) {
			*newR = crossDot(pal1[counter], pal2[counter], 100, intensity);
			*newG = crossDot(pal1[counter + 1], pal2[counter + 1], 100, intensity);
			*newB = crossDot(pal1[counter + 2], pal2[counter + 2], 100, intensity);
			*newA = 0;

			newR += 4;
			newG += 4;
			newB += 4;
			newA += 4;

			counter += 4;
		}

		setPalette(localPalette);
		fpsCycles(50);

		intensity++;
	} while(intensity <= 100);
}

/** Fade image to black
	@param palette current palette to fade */
void fadeToBlack(uint8 *palette) {
	int32 i = 0;

	if (palReseted == 0) {
		for (i = 100; i >= 0; i -= 3) {
			adjustPalette(0, 0, 0, (uint8 *) palette, i);
			fpsCycles(50);
		}
	}

	palReseted = 1;
}

/** Fade image with another palette source
	@param palette current palette to fade */
void fadeToPal(uint8 *palette) {
	int32 i = 100;

	for (i = 0; i <= 100; i += 3) {
		adjustPalette(0, 0, 0, (uint8 *) palette, i);
		fpsCycles(50);
	}

	setPalette((uint8*)palette);

	palReseted = 0;
}

/** Fade black palette to with palette */
void blackToWhite() {
	uint8 palette[NUMOFCOLORS*4];
	int32 i;

	i = 256;
	for (i = 0; i < NUMOFCOLORS; i += 3) {
		memset(palette, i, 1024);

		setPalette(palette);
	}
}

/** Resets both in-game and sdl palettes */
void setBackPal() {
	memset(palette, 0, NUMOFCOLORS*3);
	memset(paletteRGBA, 0, NUMOFCOLORS*4);

	setPalette(paletteRGBA);

	palReseted = 1;
}

/** Fade palette to red palette
	@param palette current palette to fade */
void fadePalRed(uint8 *palette) {
	int32 i = 100;

	for (i = 100; i >= 0; i -= 2) {
		adjustPalette(0xFF, 0, 0, (uint8 *) palette, i);
		fpsCycles(50);
	}
}


/** Fade red to palette
	@param palette current palette to fade */
void fadeRedPal(uint8 *palette) {
	int32 i = 0;

	for (i = 0; i <= 100; i += 2) {
		adjustPalette(0xFF, 0, 0, (uint8 *) palette, i);
		fpsCycles(50);
	}
}

/** Copy a determinate screen buffer to another
	@param source screen buffer
	@param destination screen buffer */
void copyScreen(uint8 * source, uint8 * destination) {
	int32 w, h;

	if (SCALE == 1)
		memcpy(destination, source, SCREEN_WIDTH*SCREEN_HEIGHT);
	else if (SCALE == 2)
		for (h = 0; h < SCREEN_HEIGHT / SCALE; h++) {
			for (w = 0; w < SCREEN_WIDTH / SCALE; w++) {
				*destination++ = *source;
				*destination++ = *source++;
			}
			memcpy(destination, destination - SCREEN_WIDTH, SCREEN_WIDTH);
			destination += SCREEN_WIDTH;
		}

}

/** Clear front buffer screen */
void clearScreen() {
	memset(frontVideoBuffer, 0, SCREEN_WIDTH*SCREEN_HEIGHT);
}
