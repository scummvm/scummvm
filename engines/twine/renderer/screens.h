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
#include "graphics/managed_surface.h"
#include "graphics/surface.h"
#include "twine/twine.h"

namespace TwinE {

class TwinEEngine;
class Screens {
private:
	TwinEEngine *_engine;

public:
	Screens(TwinEEngine *engine) : _engine(engine) {}

	/** In-game palette (should not be used, except in special case. otherwise use other images functions instead) */
	uint8 palette[NUMOFCOLORS * 3]{0};

	/** converted in-game palette */
	uint32 paletteRGBA[NUMOFCOLORS]{0};

	/** converted custom palette */
	uint32 paletteRGBACustom[NUMOFCOLORS]{0};

	/** flag to check in the game palette was changed */
	bool palResetted = false;

	/** flag to check if the main flag is locked */
	bool lockPalette = false;

	/** flag to check if we are using a different palette than the main one */
	bool useAlternatePalette = false;

	/** main game palette */
	uint8 *mainPalette = nullptr;

	/** converted in-game palette */
	uint32 mainPaletteRGBA[NUMOFCOLORS]{0};

	/** Load and display Adeline Logo */
	bool adelineLogo();

	void convertPalToRGBA(const uint8 *in, uint32 *out);

	/**
	 * Load a custom palette
	 * @param index \a RESS.HQR entry index (starting from 0)
	 */
	void loadCustomPalette(int32 index);

	/** Load and display Main Menu image */
	void loadMenuImage(bool fadeIn = true);

	/**
	 * Load and display a particulary image on \a RESS.HQR file with cross fade effect
	 * @param index \a RESS.HQR entry index (starting from 0)
	 * @param paletteIndex \a RESS.HQR entry index of the palette for the given image. This is often the @c index + 1
	 * @param fadeIn if we fade in before using the palette
	 */
	void loadImage(int32 index, int32 paletteIndex, bool fadeIn = true);

	/**
	 * Load and display a particulary image on \a RESS.HQR file with cross fade effect and delay
	 * @param index \a RESS.HQR entry index (starting from 0)
	 * @param paletteIndex \a RESS.HQR entry index of the palette for the given image. This is often the @c index + 1
	 * @param seconds number of seconds to delay
	 */
	bool loadImageDelay(int32 index, int32 paletteIndex, int32 seconds);

	/**
	 * Fade image in
	 * @param palette current palette to fade in
	 */
	void fadeIn(uint32 *palette);

	/**
	 * Fade image out
	 * @param palette current palette to fade out
	 */
	void fadeOut(uint32 *palette);

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
	 * @param r red component of color
	 * @param g green component of color
	 * @param b blue component of color
	 * @param palette palette to adjust
	 * @param intensity intensity value to adjust
	 */
	void adjustPalette(uint8 r, uint8 g, uint8 b, const uint32 *palette, int32 intensity);

	/**
	 * Adjust between two palettes
	 * @param pal1 palette from adjust
	 * @param pal2 palette to adjust
	 */
	void adjustCrossPalette(const uint32 *pal1, const uint32 *pal2);

	/**
	 * Fade image to black
	 * @param palette current palette to fade
	 */
	void fadeToBlack(const uint32 *palette);

	/**
	 * Fade image with another palette source
	 * @param palette current palette to fade
	 */
	void fadeToPal(const uint32 *palette);

	/** Fade black palette to white palette */
	void blackToWhite();

	/** Resets both in-game and sdl palettes */
	void setBackPal();

	/**
	 * Fade palette to red palette
	 * @param palette current palette to fade
	 */
	void fadePalRed(const uint32 *palette);

	/**
	 * Fade red to palette
	 * @param palette current palette to fade
	 */
	void fadeRedPal(const uint32 *palette);

	/**
	 * Copy a determinate screen buffer to another
	 * @param source screen buffer
	 * @param destination screen buffer
	 */
	void copyScreen(const Graphics::ManagedSurface &source, Graphics::ManagedSurface &destination);

	/** Clear front buffer screen */
	void clearScreen();

	/** Init palettes */
	void initPalettes();
};

} // namespace TwinE

#endif
