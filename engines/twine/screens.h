/** @file images.h
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

#ifndef SCREENS_H
#define SCREENS_H

#include "sys.h"
#include "main.h"

/** In-game palette (should not be used, except in special case. otherwise use other images functions instead) */
uint8 palette[NUMOFCOLORS * 3];

/** SDL converted in-game palette */
uint8 paletteRGBA[NUMOFCOLORS * 4];

/** SDL converted custom palette */
uint8 paletteRGBACustom[NUMOFCOLORS * 4];

/** flag to check if a custom palette is in use */
int16 palCustom;

/** flag to check in the game palette was changed */
int16 palReseted;

/** flag to check if the main flag is locked */
int16 lockPalette;

/** flag to check if we are using a different palette than the main one */
int16 useAlternatePalette;

/** main game palette */
uint8* mainPalette;

/** SDL converted in-game palette */
uint8 mainPaletteRGBA[NUMOFCOLORS * 4];

/** Load and display Adeline Logo */
void adelineLogo();

/** Load a custom palette
	@param index \a RESS.HQR entry index (starting from 0) */
void loadCustomPalette(int32 index);

/** Load and display Main Menu image */
void loadMenuImage(int16 fade_in);

/** Load and display a particulary image on \a RESS.HQR file with cross fade effect
	@param index \a RESS.HQR entry index (starting from 0)
	@param fade_in if we fade in before using the palette */
void loadImage(int32 index, int16 fade_in);

/** Load and display a particulary image on \a RESS.HQR file with cross fade effect and delay
	@param index \a RESS.HQR entry index (starting from 0)
	@param time number of seconds to delay */
void loadImageDelay(int32 index, int32 time);

/** Converts in-game palette to SDL palette
	@param palSource palette source with RGB
	@param palDest palette destination with RGBA */
void convertPalToRGBA(uint8 * palSource, uint8 * palDest);

/** Fade image in
	@param palette current palette to fade in */
void fadeIn(uint8 * palette);

/** Fade image out
	@param palette current palette to fade out */
void fadeOut(uint8 * palette);

/** Calculate a new color component according with an intensity
	@param modifier color compenent
	@param color color value
	@param param unknown
	@param intensity intensity value to adjust
	@return new color component*/
int32 crossDot(int32 modifier, int32 color, int32 param, int32 intensity);

/** Adjust palette intensity
	@param R red component of color
	@param G green component of color
	@param B blue component of color
	@param palette palette to adjust
	@param intensity intensity value to adjust */
void adjustPalette(uint8 R, uint8 G, uint8 B, uint8 * palette, int32 intensity);

/** Adjust between two palettes
	@param pal1 palette from adjust
	@param pal2 palette to adjust */
void adjustCrossPalette(uint8 * pal1, uint8 * pal2);

/** Fade image to black
	@param palette current palette to fade */
void fadeToBlack(uint8 *palette);

/** Fade image with another palette source
	@param palette current palette to fade */
void fadeToPal(uint8 *palette);

/** Fade black palette to white palette */
void blackToWhite();

/** Resets both in-game and sdl palettes */
void setBackPal();

/** Fade palette to red palette
	@param palette current palette to fade */
void fadePalRed(uint8 *palette);

/** Fade red to palette
	@param palette current palette to fade */
void fadeRedPal(uint8 *palette);

/** Copy a determinate screen buffer to another
	@param source screen buffer
	@param destination screen buffer */
void copyScreen(uint8 * source, uint8 * destination);

/** Clear front buffer screen */
void clearScreen();

/** Init palettes */
void initPalettes();

#endif
