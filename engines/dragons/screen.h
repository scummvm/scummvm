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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef DRAGONS_SCREEN_H
#define DRAGONS_SCREEN_H

#include "graphics/surface.h"
#include "graphics/pixelformat.h"
#include "common/rect.h"

namespace Dragons {
#define DRAGONS_NUM_PALETTES 5
#define DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE 256

#define DRAGONS_SCREEN_WIDTH 320
#define DRAGONS_SCREEN_HEIGHT 200

#define DRAGONS_NUM_FLAT_QUADS 0xf

enum AlphaBlendMode {
	NONE,
	NORMAL,       // 50% x Back + 50% x Sprite
	ADDITIVE,     // 100% x Back + 100% x Sprite
	ADDITIVE_50,  // 100% x Back + 50% x Sprite
	SUBTRACTIVE   // 100% x Back - 100% x Sprite
};

struct FlatQuad {
	uint16 flags;
	uint16 priorityLayer;
	Common::Point points[4];
	uint16 colour;

	FlatQuad() {
		flags = 0;
		priorityLayer = 0;
		colour = 0;
	}
};

class Screen {
private:
	Graphics::PixelFormat _pixelFormat;
	Graphics::Surface *_backSurface;
	byte _palettes[DRAGONS_NUM_PALETTES][512];
	Common::Point _screenShakeOffset;
	FlatQuad _flatQuads[DRAGONS_NUM_FLAT_QUADS];
public:
	virtual ~Screen();

	Screen();

	Graphics::PixelFormat getPixelFormat() { return _pixelFormat; }
	void copyRectToSurface(const Graphics::Surface &srcSurface, int destX, int destY);
	void copyRectToSurface(const Graphics::Surface &srcSurface, int destX, int destY, Common::Rect srcRect, bool flipX = false, AlphaBlendMode alpha = NONE);
	void copyRectToSurface8bpp(const Graphics::Surface &srcSurface, const byte *palette, int destX, int destY, Common::Rect srcRect, bool flipX = false, AlphaBlendMode alpha = NONE, uint16 scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE);
	void copyRectToSurface8bppWrappedX(const Graphics::Surface &srcSurface, const byte *palette, Common::Rect srcRect, AlphaBlendMode alpha = NONE);
	void updateScreen();
	void loadPalette(uint16 paletteNum, const byte *palette);
	byte *getPalette(uint16 paletteNum);
	void setPaletteRecord(uint16 paletteNum, uint16 offset, uint16 newValue);
	void updatePaletteTransparency(uint16 paletteNum, uint16 startOffset, uint16 endOffset, bool isTransparent);
	void clearScreen();
	void drawRect(uint16 colour, Common::Rect rect, int id);
	void fillRect(uint16 colour, Common::Rect rect);
	Common::Rect clipRectToScreen(int destX, int destY, const Common::Rect rect);
	Common::Rect clipRectToRect(int destX, int destY, const Common::Rect rect, const Common::Rect containerRect);

	void setScreenShakeOffset(int16 x, int16 y);

	void copyRectToSurface8bppWrappedY(const Graphics::Surface &srcSurface, const byte *palette, int yOffset);

	int16 addFlatQuad(int16 x0, int16 y0, int16 x1, int16 y1, int16 x3, int16 y3, int16 x2, int16 y2, uint16 colour, int16 priorityLayer, uint16 flags);
	void drawFlatQuads(uint16 priorityLayer);
	FlatQuad *getFlatQuad(uint16 quadId);
	void clearAllFlatQuads();

private:
	void copyRectToSurface(const void *buffer, int srcPitch, int srcWidth, int srcXOffset, int destX, int destY, int width, int height, bool flipX, AlphaBlendMode alpha);
	void copyRectToSurface8bpp(const void *buffer, const byte* palette, int srcPitch, int srcWidth, int srcXOffset, int destX, int destY, int width, int height, bool flipX, AlphaBlendMode alpha);
	void drawScaledSprite(Graphics::Surface *destSurface, const byte *source, int sourceWidth, int sourceHeight, int destX, int destY, int destWidth, int destHeight, const byte *palette, bool flipX, AlphaBlendMode alpha);
};

} // End of namespace Dragons

#endif //DRAGONS_SCREEN_H
