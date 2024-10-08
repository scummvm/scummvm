/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TWINE_RENDERER_SCREENS_H
#define TWINE_RENDERER_SCREENS_H

#include "common/scummsys.h"
#include "graphics/managed_surface.h"
#include "graphics/surface.h"
#include "twine/twine.h"

namespace TwinE {

class TwinEEngine;
class Screens {
private:
	TwinEEngine *_engine;

	/**
	 * Adjust palette intensity
	 * @param r red component of color
	 * @param g green component of color
	 * @param b blue component of color
	 * @param palette palette to adjust
	 * @param intensity intensity value to adjust
	 */
	void adjustPalette(uint8 r, uint8 g, uint8 b, const uint32 *palette, int32 intensity);

public:
	Screens(TwinEEngine *engine) : _engine(engine) {}

	/** In-game palette (should not be used, except in special case. otherwise use other images functions instead) */
	uint8 _palette[NUMOFCOLORS * 3]{0};

	int32 mapLba2Palette(int32 palIndex);

	/** converted in-game palette */
	uint32 _ptrPal[NUMOFCOLORS]{0};

	/** converted custom palette */
	uint32 _palettePcx[NUMOFCOLORS]{0};

	/** flag to check in the game palette was changed */
	bool _palResetted = false;

	/** flag to check if the main flag is locked */
	bool _flagFade = false;

	/** flag to check if we are using a different palette than the main one */
	bool _flagPalettePcx = false;

	/** converted in-game palette */
	uint32 _mainPaletteRGBA[NUMOFCOLORS]{0};

	/** Load and display Adeline Logo */
	bool adelineLogo();

	void convertPalToRGBA(const uint8 *in, uint32 *out);

	/**
	 * @sa setNormalPal
	 */
	void setDarkPal();
	/**
	 * @sa setDarkPal()
	 * Reset the palette to the main palette after the script changed it via @c setDarkPal()
	 */
	void setNormalPal();

	/**
	 * Load a custom palette
	 * @param index \a RESS.HQR entry index (starting from 0)
	 */
	void loadCustomPalette(const TwineResource &resource);

	/** Load and display Main Menu image */
	void loadMenuImage(bool fadeIn = true);

	/**
	 * Load and display a particularly image on \a RESS.HQR file with cross fade effect
	 * @param index \a RESS.HQR entry index (starting from 0)
	 * @param paletteIndex \a RESS.HQR entry index of the palette for the given image. This is often the @c index + 1
	 * @param fadeIn if we fade in before using the palette
	 */
	void loadImage(TwineImage image, bool fadeIn = true);

	/**
	 * Load and display a particularly image on \a RESS.HQR file with cross fade effect and delay
	 * @param index \a RESS.HQR entry index (starting from 0)
	 * @param paletteIndex \a RESS.HQR entry index of the palette for the given image. This is often the @c index + 1
	 * @param seconds number of seconds to delay
	 * @return @c true if aborted
	 */
	bool loadImageDelay(TwineImage image, int32 seconds);

	bool loadBitmapDelay(const char *image, int32 seconds);

	/**
	 * Fade image in
	 * @param palette current palette to fade in
	 */
	void fadeIn(const uint32 *palette);

	/**
	 * Fade image out
	 * @param palette current palette to fade out
	 */
	void fadeOut(const uint32 *palette);

	/**
	 * Linear interpolation of the given value between start and end
	 * @param value color component
	 * @param start lower range
	 * @param end upper range
	 * @param t the location in given range
	 * @return the lerped value
	 * @note Doesn't clamp
	 */
	int32 lerp(int32 value, int32 start, int32 end, int32 t);

	/**
	 * Adjust between two palettes
	 * @param pal1 palette from adjust
	 * @param pal2 palette to adjust
	 */
	void fadePalToPal(const uint32 *pal1, const uint32 *pal2);

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
	void setBlackPal();

	/**
	 * Fade palette to red palette
	 * @param palette current palette to fade
	 */
	void fadeToRed(const uint32 *palette);

	/**
	 * Fade red to palette
	 * @param palette current palette to fade
	 */
	void fadeRedToPal(const uint32 *palette);

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
