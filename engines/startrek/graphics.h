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
const int MAX_MENUBUTTONS = 16; // This is arbitrary, the original game has no such limit

const int TEXTBOX_WIDTH = 26;
const int MAX_TEXTBOX_LINES = 12;


// Keeps track of data for a list of buttons making up a menu
struct Menu {
	Sprite sprites[MAX_MENUBUTTONS];
	uint16 retvals[MAX_MENUBUTTONS];
	uint32 disabledButtons;
	SharedPtr<FileStream> menuFile;
	uint16 numButtons;
	int16 selectedButton;
	SharedPtr<Menu> nextMenu;

	Menu() : nextMenu(SharedPtr<Menu>()) {}
};

class Graphics;
typedef String (Graphics::*TextGetterFunc)(int, uintptr, String *);


class Graphics {
public:
	Graphics(StarTrekEngine *vm);
	~Graphics();
	
	void loadEGAData(const char *egaFile);
	void drawBackgroundImage(const char *filename);

	void loadPalette(const String &paletteFile);
	void loadPri(const char *priFile);

	SharedPtr<Bitmap> loadBitmap(String basename);

	Common::Point getMousePos();
	void setMouseCursor(SharedPtr<Bitmap> bitmap);

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
	SharedPtr<Bitmap> _backgroundImage;

	Sprite *_sprites[MAX_SPRITES];
	int _numSprites;

	SharedPtr<Bitmap> _mouseBitmap;


	// text.cpp (TODO: separate class)
public:
	int showText(TextGetterFunc textGetter, uintptr var, int xoffset, int yoffset, int textColor, bool loopChoices, int maxTextLines, int arg10);

	String readTextFromRdf(int choiceIndex, uintptr data, String *headerTextOutput);
	String readTextFromBuffer(int choiceIndex, uintptr data, String *headerTextOutput);
	String readTextFromArray(int choiceIndex, uintptr data, String *headerTextOutput);

private:
	int handleTextboxEvents(uint32 ticksUntilClickingEnabled, bool arg4);

	SharedPtr<TextBitmap> initTextSprite(int *xoffsetPtr, int *yoffsetPtr, byte textColor, int numTextLines, bool withHeader, Sprite *sprite);
	void drawMainText(SharedPtr<TextBitmap> bitmap, int numTextLines, int numTextboxLines, const String &text, bool withHeader);

	int getNumLines(const String &str);
	void getTextboxHeader(String *headerTextOutput, String speakerText, int choiceIndex);

	String readLineFormattedText(TextGetterFunc textGetter, uintptr var, int choiceIndex, SharedPtr<TextBitmap> textBitmap, int numTextboxLines, int *numLines);
	String putTextIntoLines(const String &text);
	const char *getNextTextLine(const char *text, char *line, int lineWidth);

	String skipTextAudioPrompt(const String &str);
	String playTextAudio(const String &str);

	int getMenuButtonAt(const Menu &menu, int x, int y);
	void drawMenuButtonOutline(SharedPtr<Bitmap> bitmap, byte color);
	void loadMenuButtons(String mnuFilename, int xpos, int ypos);
	void disableMenuButton(uint32 bits);
	void enableMenuButton(uint32 bits);

public:
	void openTextConfigurationMenu(bool fromOptionMenu);
	int loadTextDisplayMode();
	void saveTextDisplayMode(int value);

private:
	int16 _textDisplayMode;
	uint32 _textboxVar2;
	uint32 _textboxVar3;
	uint16 _textboxVar6;
	uint16 _textboxVar7;
	bool _textboxHasMultipleChoices;

	SharedPtr<Menu> _activeMenu;

	uint16 _textboxButtonVar4;
};

}

#endif
