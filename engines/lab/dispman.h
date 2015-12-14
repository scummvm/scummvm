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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#ifndef LAB_DISPMAN_H
#define LAB_DISPMAN_H

namespace Lab {

struct BitMap;
class LabEngine;
class Image;

struct TextFont {
	uint32 _dataLength;
	uint16 _height;
	byte _widths[256];
	uint16 _offsets[256];
	byte *_data;
};

enum TransitionType {
	kTransitionNone,
	kTransitionWipe,
	kTransitionScrollWipe,
	kTransitionScrollBlack,
	kTransitionScrollBounce,
	kTransitionTransporter,
	kTransitionReadFirstFrame,
	kTransitionReadNextFrame
};

class DisplayMan {
private:
	LabEngine *_vm;

	uint16 fadeNumIn(uint16 num, uint16 res, uint16 counter);
	uint16 fadeNumOut(uint16 num, uint16 res, uint16 counter);
	void getWord(char *wordBuffer, const char *mainBuffer, uint16 *wordWidth);

	byte _curPen;
	byte *_curBitmap;
	byte _curvgapal[256 * 3];

public:
	DisplayMan(LabEngine *lab);
	virtual ~DisplayMan();

	void loadPict(const char *filename);
	void loadBackPict(const char *fileName, uint16 *highPal);
	void readPict(const char *filename, bool playOnce, bool onlyDiffData = false, byte *memoryBuffer = nullptr, uint16 maxHeight = 0);
	void freePict();
	void doScrollBlack();
	void copyPage(uint16 width, uint16 height, uint16 nheight, uint16 startline, byte *mem);
	void doScrollWipe(char *filename);
	void doScrollBounce();
	void doTransWipe(CloseDataPtr *closePtrList, char *filename);
	void doTransition(TransitionType transitionType, CloseDataPtr *closePtrList, char *filename);
	void blackScreen();
	void whiteScreen();
	void blackAllScreen();
	void createBox(uint16 y2);
	void drawPanel();
	void setUpScreens();
	int32 longDrawMessage(const char *str);
	void drawMessage(const char *str);
	void setAPen(byte pennum);
	void rectFill(uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	void rectFillScaled(uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	// Window text stuff
	int flowText(TextFont *font,		// the TextAttr pointer
				int16 spacing,			// How much vertical spacing between the lines
				byte pencolor,			// pen number to use for text
				byte backpen,			// the background color
				bool fillback,			// Whether to fill the background
				bool centerh,			// Whether to center the text horizontally
				bool centerv,			// Whether to center the text vertically
				bool output,			// Whether to output any text
				uint16 x1, uint16 y1,	// Cords
				uint16 x2, uint16 y2,
				const char *text);		// The text itself

	int flowTextScaled(
				TextFont *font,			// the TextAttr pointer
				int16 spacing,			// How much vertical spacing between the lines
				byte pencolor,			// pen number to use for text
				byte backpen,			// the background color
				bool fillback,			// Whether to fill the background
				bool centerh,			// Whether to center the text horizontally
				bool centerv,			// Whether to center the text vertically
				bool output,			// Whether to output any text
				uint16 x1, uint16 y1,	// Cords
				uint16 x2, uint16 y2,
				const char *text);		// The text itself

	int flowTextToMem(Image *destIm,
				TextFont *font,			// the TextAttr pointer
				int16 spacing,			// How much vertical spacing between the lines
				byte pencolor,			// pen number to use for text
				byte backpen,			// the background color
				bool fillback,			// Whether to fill the background
				bool centerh,			// Whether to center the text horizontally
				bool centerv,			// Whether to center the text vertically
				bool output,			// Whether to output any text
				uint16 x1, uint16 y1,	// Cords
				uint16 x2, uint16 y2,
				const char *str);		// The text itself

	void drawHLine(uint16 x, uint16 y1, uint16 y2);
	void drawVLine(uint16 x1, uint16 y, uint16 x2);
	void screenUpdate();
	void createScreen(bool hiRes);
	void setAmigaPal(uint16 *pal, uint16 numColors);
	void writeColorRegs(byte *buf, uint16 first, uint16 numReg);
	void setPalette(void *cmap, uint16 numColors);
	void overlayRect(uint16 penColor, uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	byte *getCurrentDrawingBuffer();
	void scrollDisplayX(int16 dx, uint16 x1, uint16 y1, uint16 x2, uint16 y2, byte *buffer);
	void scrollDisplayY(int16 dy, uint16 x1, uint16 y1, uint16 x2, uint16 y2, byte *buffer);
	void fade(bool fadein, uint16 res);
	void closeFont(TextFont *font);
	uint16 textLength(TextFont *font, const char *text, uint16 numChars);
	uint16 textHeight(TextFont *tf);
	void text(TextFont *tf, uint16 x, uint16 y, uint16 color, const char *text, uint16 numchars);
	void getLine(TextFont *tf, char *lineBuffer, const char **mainBuffer, uint16 lineWidth);

	bool _longWinInFront;
	bool _lastMessageLong;
	uint32 _screenBytesPerPage;
	int _screenWidth;
	int _screenHeight;
	byte *_displayBuffer;
	byte *_currentDisplayBuffer;
	bool _doNotDrawMessage;
	uint16 *_fadePalette;
	BitMap *_dispBitMap;
};

} // End of namespace Lab

#endif // LAB_DISPMAN_H
