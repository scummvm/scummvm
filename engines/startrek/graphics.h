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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STARTREK_GRAPHICS_H
#define STARTREK_GRAPHICS_H

#include "startrek/bitmap.h"
#include "startrek/font.h"
#include "startrek/startrek.h"
#include "startrek/sprite.h"

#include "common/ptr.h"
#include "common/rect.h"
#include "common/stream.h"

using Common::SharedPtr;
using Common::String;

namespace StarTrek {

class Font;
class StarTrekEngine;


const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 200;

const int MAX_SPRITES = 32;


class Graphics {
public:
	Graphics(StarTrekEngine *vm);
	~Graphics();

	void setBackgroundImage(Common::String imageName);
	/**
	 * @param origRect The rectangle containing the original bitmap (must contain the
	 *                 whole bitmap, even if some is outside the drawable space)
	 * @param drawRect The clipped rectangle to draw at (must be within the drawable space)
	 */
	void drawBitmapToBackground(const Common::Rect &origRect, const Common::Rect &drawRect, Bitmap *bitmap);

	void fillBackgroundRect(const Common::Rect &rect, byte color);
	byte *getBackgroundPixels();
	byte *lockScreenPixels();
	void unlockScreenPixels();

	void clearScreenAndPriBuffer();
	/**
	 * Note: this doesn't flush the palette to the screen (must call "setPaletteFadeLevel")
	 */
	void loadPalette(const String &paletteFile);
	void copyRectBetweenBitmaps(Bitmap *destBitmap, int destX, int destY, Bitmap *srcBitmap, int srcX, int srcY, int width, int height);
	void fadeinScreen();
	void fadeoutScreen();
	/**
	 * This flushes the palette to the screen. fadeLevel ranges from 0-100.
	 */
	void setPaletteFadeLevel(byte *palData, int fadeLevel);
	void incPaletteFadeLevel();
	void decPaletteFadeLevel();

	void loadPri(const Common::String &priFile);
	void clearPri();
	void setPri(byte val);
	byte getPriValue(int x, int y);

	Common::Point getMousePos();
	/**
	 * Changes the mouse bitmap. The change won't take effect until drawAllSprites is
	 * called again.
	 */
	void setMouseBitmap(Common::String bitmapName);
	void popMouseBitmap();
	void toggleMouse(bool visible);

	/**
	 * This function is a workaround for when the mouse position needs to be locked in a set
	 * position (used in the action menu). This only affects the position it is drawn at; the
	 * sprite's "real" position is still updated normally.
	 *
	 * This does not call updateScreen.
	 */
	void lockMousePosition(int16 x, int16 y);
	void unlockMousePosition();
	void warpMouse(int16 x, int16 y);

	void drawSprite(const Sprite &sprite, ::Graphics::Surface *surface);
	/**
	 * @param sprite The sprite to draw
	 * @param surface The surface to draw to
	 * @param rect The part of the sprite to draw (only draw the part of the sprite that
	 *             intersects with it)
	 @ @param rectLeft X-offset to subtract before drawing to surface.
	 @ @param rectTop  Y-offset to subtract before drawing to surface.
	 */
	void drawSprite(const Sprite &sprite, ::Graphics::Surface *surface, const Common::Rect &rect, int rectLeft = 0, int rectTop = 0);
	void drawAllSprites(bool updateScreenFlag = true);
	/**
	 * This function should only be called after "drawAllSprites" (so that sprite rects
	 * are updated).
	 */
	void drawAllSpritesInRectToSurface(const Common::Rect &rect, ::Graphics::Surface *surface);
	/**
	 * Sets "bitmapChanged" to true on all sprites before calling drawAllSprites.
	 */
	void forceDrawAllSprites(bool updateScreenFlag = true);
	/**
	 * Flushes the screen. Usually called by "drawAllSprites".
	 */
	void updateScreen();
	/**
	 * Returns the sprite at the given position (ignores mouse).
	 */
	Sprite *getSpriteAt(int16 x, int16 y);
	Sprite *getSpriteAt(Common::Point p) {
		return getSpriteAt(p.x, p.y);
	}

	void addSprite(Sprite *sprite);
	void delSprite(Sprite *sprite);

	// Save/load the current state of sprites. Can only push once for now.
	void pushSprites();
	void popSprites();

	byte *getFontGfx(char c);

	void copyBackgroundScreen();
	void loadEGAData(const char *egaFile);
	void drawBackgroundImage(const char *filename);

public:
	Font *_font;

private:
	StarTrekEngine *_vm;

	bool _egaMode;
	byte *_egaData;
	byte *_palData;
	byte *_lutData;
	byte _priData[SCREEN_WIDTH * SCREEN_HEIGHT / 2];

	int16 _paletteFadeLevel;

	Common::Rect _screenRect;
	Bitmap *_backgroundImage;

	Sprite *_sprites[MAX_SPRITES];
	int _numSprites;

	// Analogous to above, used when pushing/popping
	Sprite *_pushedSprites[MAX_SPRITES];
	int _pushedNumSprites;

	Common::Point _lockedMousePos;
};

} // End of namespace StarTrek

#endif
