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
	
	void setBackgroundImage(SharedPtr<Bitmap> bitmap);

	void loadPalette(const String &paletteFile);
	void fadeinScreen();
	void fadeoutScreen();
	void setPaletteFadeLevel(byte *palData, int fadeLevel);
	void incPaletteFadeLevel();
	void decPaletteFadeLevel();

	void loadPri(const Common::String &priFile);
	void clearPri();
	byte getPriValue(int x, int y);

	SharedPtr<Bitmap> loadBitmap(String basename);

	Common::Point getMousePos();
	void setMouseBitmap(SharedPtr<Bitmap> bitmap);
	void lockMousePosition(int16 x, int16 y);
	void unlockMousePosition();
	SharedPtr<Bitmap> getMouseBitmap();
	void warpMouse(int16 x, int16 y);

	void drawSprite(const Sprite &sprite);
	void drawSprite(const Sprite &sprite, const Common::Rect &rect);
	void drawAllSprites(bool updateScreen=true);
	void forceDrawAllSprites(bool updateScreen=true);
	Sprite *getSpriteAt(int16 x, int16 y);
	Sprite *getSpriteAt(Common::Point p) { return getSpriteAt(p.x, p.y); }

	void addSprite(Sprite *sprite);
	void delSprite(Sprite *sprite);
	
	// Save/load the current state of sprites. Can only push once for now.
	void pushSprites();
	void popSprites();

	void copyBackgroundScreen();
	void drawDirectToScreen(SharedPtr<Bitmap> bitmap);
	void loadEGAData(const char *egaFile);
	void drawBackgroundImage(const char *filename);
	
private:
	StarTrekEngine *_vm;
	Font *_font;
	
	bool _egaMode;
	byte *_egaData;
	byte *_palData;
	byte *_lutData;
	byte _priData[SCREEN_WIDTH * SCREEN_HEIGHT / 2];

	int16 _paletteFadeLevel;

	Common::Rect _screenRect;
	SharedPtr<Bitmap> _backgroundImage;

	Sprite *_sprites[MAX_SPRITES];
	int _numSprites;

	// Analagous to above, used when pushing/popping
	Sprite *_pushedSprites[MAX_SPRITES];
	int _pushedNumSprites;

	// Any changes to the mouse image are buffered until the next time "drawAllSprites" is
	// called (since the original game treats it like a sprite).
	bool _mouseToBeShown;
	bool _mouseToBeHidden;
	int16 _mouseWarpX, _mouseWarpY;
	SharedPtr<Bitmap> _mouseBitmapLastFrame;
	SharedPtr<Bitmap> _mouseBitmap;

	// These are used as a workaround for when the mouse position must be locked.
	// The mouse is turned into a native game sprite when this happens.
	bool _mouseLocked;
	Sprite _lockedMouseSprite;


public:
};

}

#endif
