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
#ifndef PELROCK_GRAPHICS_H
#define PELROCK_GRAPHICS_H

#include "common/array.h"
#include "common/scummsys.h"
#include "common/str-array.h"

#include "graphics/font.h"
#include "graphics/managed_surface.h"
#include "graphics/screen.h"

#include "pelrock/types.h"

namespace Pelrock {

class GraphicsManager {
public:
	GraphicsManager();
	~GraphicsManager();

	// Overlay / palette utilities
	Common::Point showOverlay(int height, Graphics::ManagedSurface &buf);
	void fadeToBlack(int stepSize);
	void fadePaletteToTarget(byte *targetPalette, int stepSize);
	void clearScreen();

	// Text rendering
	void drawColoredText(Graphics::ManagedSurface *screen, const Common::String &text, int x, int y, int w, byte &defaultColor, Graphics::Font *font);
	void drawColoredText(Graphics::ManagedSurface &buf, const Common::String &text, int x, int y, int w, byte &defaultColor, Graphics::Font *font);
	void drawColoredTexts(Graphics::ManagedSurface *surface, const Common::StringArray &text, int x, int y, int w, int yPadding, Graphics::Font *font);
	void drawColoredTexts(Graphics::ManagedSurface &buf, const Common::StringArray &text, int x, int y, int w, int yPadding, Graphics::Font *font);

	// Frame / background management
	void copyBackgroundToBuffer();
	void presentFrame();

	// Sticker rendering
	void placeStickersFirstPass();
	void placeStickersSecondPass();
	void placeSticker(Sticker &sticker, byte *pixels);

	// Palette animations
	void updatePaletteAnimations();
	void animateFadePalette(PaletteAnim *anim);
	void animateRotatePalette(PaletteAnim *anim);

	/** Water reflection: mirrors buf pixels at (x,y) for water-palette pixels. */
	void reflectionEffect(byte *buf, int x, int y, int width, int height);

	// scaling
	void calculateScalingMasks();

	/**
	 * Returns the scaled width/height and scale parameters for Alfred at yPos.
	 */
	ScaleCalculation calculateScaling(int yPos, ScalingParams scalingParams);

	/**
	 * Scales a source frame (kAlfredFrameWidth × kAlfredFrameHeight) to
	 * (finalWidth × finalHeight) using the pre-computed scaling tables.
	 * Caller owns the returned buffer.
	 */
	byte *scale(int scaleY, int finalWidth, int finalHeight, byte *buf);

	// Scaling look-up tables initialized by calculateScalingMasks().
	Common::Array<Common::Array<int>> _widthScalingTable;
	Common::Array<Common::Array<int>> _heightScalingTable;
};

} // End of namespace Pelrock
#endif // PELROCK_GRAPHICS_H
