/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TWINE_SCREENS_H
#define TWINE_SCREENS_H

#include "common/scummsys.h"
#include "twine/twine.h"

namespace TwinE {

class TwinEEngine;
class Screens {
private:
	TwinEEngine *_engine;

public:
	Screens(TwinEEngine *engine) : _engine(engine) {}

	/** In-game palette (should not be used, except in special case. otherwise use other images functions instead) */
	uint8 palette[NUMOFCOLORS * 3] {0};

	/** converted in-game palette */
	uint8 paletteRGB[NUMOFCOLORS * 3] {0};

	/** converted custom palette */
	uint8 paletteRGBCustom[NUMOFCOLORS * 3] {0};

	/** flag to check if a custom palette is in use */
	int16 palCustom = 0;

	/** flag to check in the game palette was changed */
	int16 palReseted = 0;

	/** flag to check if the main flag is locked */
	int16 lockPalette = 0;

	/** flag to check if we are using a different palette than the main one */
	int16 useAlternatePalette = 0;

	/** main game palette */
	uint8 *mainPalette = nullptr;

	/** converted in-game palette */
	uint8 mainPaletteRGB[NUMOFCOLORS * 3] {0};

	/** Load and display Adeline Logo */
	void adelineLogo();

	void copyPal(const uint8* in, uint8* out);

	/**
	 * Load a custom palette
	 * @param index \a RESS.HQR entry index (starting from 0)
	 */
	void loadCustomPalette(int32 index);

	/** Load and display Main Menu image */
	void loadMenuImage(bool fade_in = true);

	/**
	 * Load and display a particulary image on \a RESS.HQR file with cross fade effect
	 * @param index \a RESS.HQR entry index (starting from 0)
	 * @param fade_in if we fade in before using the palette
	 */
	void loadImage(int32 index, bool fade_in = true);

	/**
	 * Load and display a particulary image on \a RESS.HQR file with cross fade effect and delay
	 * @param index \a RESS.HQR entry index (starting from 0)
	 * @param time number of seconds to delay
	 */
	void loadImageDelay(int32 index, int32 time);

	/**
	 * Fade image in
	 * @param palette current palette to fade in
	 */
	void fadeIn(uint8 *palette);

	/**
	 * Fade image out
	 * @param palette current palette to fade out
	 */
	void fadeOut(uint8 *palette);

	/**
	 * Calculate a new color component according with an intensity
	 * @param modifier color compenent
	 * @param color color value
	 * @param param unknown
	 * @param intensity intensity value to adjust
	 * @return new color component
	 */
	int32 crossDot(int32 modifier, int32 color, int32 param, int32 intensity);

	/**
	 * Adjust palette intensity
	 * @param R red component of color
	 * @param G green component of color
	 * @param B blue component of color
	 * @param palette palette to adjust
	 * @param intensity intensity value to adjust
	 */
	void adjustPalette(uint8 R, uint8 G, uint8 B, uint8 *palette, int32 intensity);

	/**
	 * Adjust between two palettes
	 * @param pal1 palette from adjust
	 * @param pal2 palette to adjust
	 */
	void adjustCrossPalette(uint8 *pal1, uint8 *pal2);

	/**
	 * Fade image to black
	 * @param palette current palette to fade
	 */
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
	void copyScreen(const uint8 *source, uint8 *destination);

	/** Clear front buffer screen */
	void clearScreen();

	/** Init palettes */
	void initPalettes();
};

} // namespace TwinE

#endif
