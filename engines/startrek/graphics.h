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
 * $URL: https://scummvm-startrek.googlecode.com/svn/trunk/graphics.h $
 * $Id: graphics.h 2 2009-09-12 20:13:40Z clone2727 $
 *
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

namespace StarTrek {

class Font;
class StarTrekEngine;


const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 200;

const int MAX_SPRITES = 32;

const int TEXTBOX_WIDTH = 26;


class Graphics;
typedef Common::String (Graphics::*TextGetterFunc)(int, int, Common::String *);


class Graphics {
public:
	Graphics(StarTrekEngine *vm);
	~Graphics();
	
	void loadEGAData(const char *egaFile);
	void drawBackgroundImage(const char *filename);

	void loadPalette(const Common::String &paletteFile);
	void loadPri(const char *priFile);

	SharedPtr<Bitmap> loadBitmap(Common::String basename);

	void redrawScreen();
	void drawSprite(const Sprite &sprite);
	void drawSprite(const Sprite &sprite, const Common::Rect &rect);
	void drawAllSprites();

	void addSprite(Sprite *sprite);
	void delSprite(Sprite *sprite);
	
	
private:
	void drawBitmapToScreen(Bitmap *bitmap);


	StarTrekEngine *_vm;
	Font *_font;
	
	bool _egaMode;
	byte *_egaData;
	byte *_priData;
	byte *_lutData;

	Common::Rect _screenRect;
	Bitmap *_backgroundImage;
	Bitmap *_canvas;

	Sprite *_sprites[MAX_SPRITES];
	int _numSprites;

	Common::Point _mousePos;
	Sprite _mouseSprite;


	// text.cpp (TODO: separate class)
public:
	int showText(TextGetterFunc textGetter, int var, int xoffset, int yoffset, int textColor, int argC, int maxTextLines, int arg10);
	Common::String tmpFunction(int choiceIndex, int var, Common::String *speakerTextOutput);
	SharedPtr<TextBitmap> initTextSprite(int *xoffsetPtr, int *yoffsetPtr, byte textColor, int numTextLines, bool withHeader, Sprite *sprite);

private:
	Common::String skipOverAudioPrompt(const Common::String &str);
	int getNumLines(const Common::String &str);
	Common::String readLineFormattedText(TextGetterFunc textGetter, int var, int choiceIndex, SharedPtr<TextBitmap> textBitmap, int numTextboxLines, int *numLines);
	void loadTextButtons(Common::String mnuFilename, int xpos, int ypos);
	void warpMousePosition(int x, int y);

	uint16 _textboxVar1;
	uint32 _textboxVar2;
	uint32 _textboxVar3;
	uint16 _textboxVar4;
	uint16 _textboxVar5;
	uint16 _textboxVar6;
	uint16 _textboxVar7;
	bool _textboxHasMultipleChoices;
};

}

#endif
