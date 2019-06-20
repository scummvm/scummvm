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

#ifndef SUPERNOVA2_SCREEN_H
#define SUPERNOVA2_SCREEN_H

#include "common/array.h"
#include "common/rect.h"
#include "common/scummsys.h"

#include "supernova2/imageid.h"
#include "supernova2/ms2_def.h"

namespace Supernova2 {

class Supernova2Engine;
class GameManager;
class ResourceManager;
class GuiElement;
class Room;
class MS2Image;
class Screen;

const int kScreenWidth  = 320;
const int kScreenHeight = 200;
const int kFontWidth = 4;
const int kFontHeight = 8;

enum Color {
	kColorBlack       =  0,
	kColorWhite25     =  1,
	kColorWhite35     =  2,
	kColorWhite44     =  3,
	kColorWhite99     =  4,
	kColorDarkGreen   =  5,
	kColorGreen       =  6,
	kColorDarkRed     =  7,
	kColorRed         =  8,
	kColorDarkBlue    =  9,
	kColorBlue        = 10,
	kColorWhite63     = 11,
	kColorLightBlue   = 12,
	kColorLightGreen  = 13,
	kColorLightYellow = 14,
	kColorLightRed    = 15,
	kColorPurple	  = 35,
	kColorCursorTransparent = kColorWhite25
};

class ScreenBuffer {
	friend class ScreenBufferStack;

public:
	ScreenBuffer();

private:
	byte *_pixels;
	int _x;
	int _y;
	int _width;
	int _height;
};

class ScreenBufferStack {
public:
	ScreenBufferStack();

	void push(int x, int y, int width, int height);
	void restore();

private:
	ScreenBuffer _buffer[8];
	ScreenBuffer *_last;
};

class Marquee {
public:
	enum MarqueeId {
		kMarqueeIntro,
		kMarqueeOutro
	};

	Marquee(Screen *screen, MarqueeId id, const char *text);

	void renderCharacter();

private:
	void clearText();

	Screen *_screen;
	const char *const _textBegin;
	const char *_text;
	bool _loop;
	int _delay;
	int _color;
	int _x;
	int _y;
	int _textWidth;
};

class Screen {
	friend class Marquee;

public:
	struct ImageInfo {
		int filenumber;
		int section;
	};

public:
	static void initPalette();
	static int textWidth(const uint16 key);
	static int textWidth(const char *text);
	static int textWidth(const Common::String &text);

public:
	Screen(Supernova2Engine *vm, ResourceManager *resMan);

	int getViewportBrightness() const;
	void setViewportBrightness(int brightness);
	int getGuiBrightness() const;
	void setGuiBrightness(int brightness);
	MS2Image *getCurrentImage();
	const ImageInfo *getImageInfo(ImageId id) const;
	bool isMessageShown() const;
	void paletteFadeIn(int maxViewportBrightness);
	void paletteFadeOut();
	void paletteBrightness();
	void renderImage(ImageId id, bool removeImage = false);
	void renderImage(int section);
	bool setCurrentImage(int filenumber);
	void saveScreen(int x, int y, int width, int height);
	void saveScreen(const GuiElement &guiElement);
	void restoreScreen();
	void renderRoom(Room &room);
	void renderMessage(const char *text, MessagePosition position = kMessageNormal, int positionX = -1, int positionY = -1);
	void renderMessage(const Common::String &text, MessagePosition position = kMessageNormal);
	void renderMessage(StringId stringId, MessagePosition position = kMessageNormal,
					   Common::String var1 = "", Common::String var2 = "");
	void removeMessage();
	void renderText(const uint16 character);
	void renderText(const char *text);
	void renderText(const Common::String &text);
	void renderText(StringId stringId);
	void renderText(const uint16 character, int x, int y, byte color);
	void renderText(const char *text, int x, int y, byte color);
	void renderText(const Common::String &text, int x, int y, byte color);
	void renderText(StringId stringId, int x, int y, byte color);
	void renderText(const GuiElement &guiElement);
	void renderBox(int x, int y, int width, int height, byte color);
	void renderBox(const GuiElement &guiElement);
	void setColor63(byte value);
	Common::Point getTextCursorPos();
	void setTextCursorPos(int x, int y);
	byte getTextCursorColor();
	void setTextCursorColor(byte color);
	void update();

private:
	void renderImageSection(const MS2Image *image, int section, bool invert);

private:
	Supernova2Engine *_vm;
	ResourceManager *_resMan;
	MS2Image *_currentImage;
	ScreenBufferStack _screenBuffer;
	int _screenWidth;
	int _screenHeight;
	int  _textCursorX;
	int  _textCursorY;
	int  _textColor;
	byte _viewportBrightness;
	byte _guiBrightness;
	bool _messageShown;
};

}

#endif
