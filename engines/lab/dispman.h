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

	/**
	 * Does the fading of the Palette on the screen.
	 */
	uint16 fadeNumIn(uint16 num, uint16 res, uint16 counter);
	uint16 fadeNumOut(uint16 num, uint16 res, uint16 counter);

	/**
	 * Extracts the first word from a string.
	 */
	Common::String getWord(const char *mainBuffer);

	byte _curPen;
	Common::File *_curBitmap;
	byte _curvgapal[256 * 3];

public:
	DisplayMan(LabEngine *lab);
	virtual ~DisplayMan();

	void loadPict(const Common::String filename);
	void loadBackPict(const Common::String fileName, uint16 *highPal);

	/**
	 * Reads in a picture into the display bitmap.
	 */
	void readPict(Common::String filename, bool playOnce, bool onlyDiffData = false, byte *memoryBuffer = nullptr, uint16 maxHeight = 0);
	void freePict();

	/**
	 * Scrolls the display to black.
	 */
	void doScrollBlack();
	void copyPage(uint16 width, uint16 height, uint16 nheight, uint16 startline, byte *mem);

	/**
	 * Scrolls the display to a new picture from a black screen.
	 */
	void doScrollWipe(Common::String filename);

	/**
	 * Does the scroll bounce.  Assumes bitmap already in memory.
	 */
	void doScrollBounce();

	/**
	 * Does the transporter wipe.
	 */
	void doTransWipe(CloseDataPtr *closePtrList, Common::String filename);

	/**
	 * Does a certain number of pre-programmed wipes.
	 */
	void doTransition(TransitionType transitionType, CloseDataPtr *closePtrList, Common::String filename);

	/**
	 * Changes the front screen to black.
	 */
	void blackScreen();

	/**
	 * Changes the front screen to white.
	 */
	void whiteScreen();

	/**
	 * Changes the entire screen to black.
	*/
	void blackAllScreen();
	void createBox(uint16 y2);

	/**
	 * Draws the control panel display.
	 */
	void drawPanel();

	/**
	 * Sets up the Labyrinth screens, and opens up the initial windows.
	 */
	void setUpScreens();
	int32 longDrawMessage(Common::String str);

	/**
	 * Draws a message to the message box.
	 */
	void drawMessage(Common::String str);

	/**
	 * Sets the pen number to use on all the drawing operations.
	 */
	void setPen(byte pennum);

	/**
	 * Fills in a rectangle.
	 */
	void rectFill(uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	void rectFill(Common::Rect fillRect);
	void rectFillScaled(uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	/**
	 * Dumps a chunk of text to an arbitrary box; flows it within that box and
	 * optionally centers it. Returns the number of characters that were processed.
	 * @note Every individual word MUST be int16 enough to fit on a line, and
	 * each line less than 255 characters.
	 * @param font Pointer on the font used
	 * @param spacing How much vertical spacing between the lines
	 * @param penColor Pen number to use for text
	 * @param backPen Background color
	 * @param fillBack Whether to fill the background
	 * @param centerh Whether to center the text horizontally
	 * @param centerv Whether to center the text vertically
	 * @param output Whether to output any text
	 * @param textRect Coords
	 * @param text The text itself
	 */
	int flowText(TextFont *font, int16 spacing, byte penColor, byte backPen, bool fillBack,
				bool centerh, bool centerv, bool output, Common::Rect textRect, const char *text);

	/**
	 * Calls flowText, but flows it to memory.  Same restrictions as flowText.
	 * @param destIm Destination buffer
	 * @param font Pointer on the font used
	 * @param penColor Pen number to use for text
	 * @param backPen Background color
	 * @param fillBack Whether to fill the background
	 * @param centerh Whether to center the text horizontally
	 * @param centerv Whether to center the text vertically
	 * @param output Whether to output any text
	 * @param textRect Coords
	 * @param text The text itself
	 */
	int flowTextToMem(Image *destIm, TextFont *font, int16 spacing, byte penColor, byte backPen,
				bool fillBack, bool centerh, bool centerv, bool output, Common::Rect textRect,
				const char *text);

	/**
	 * Draws a vertical line.
	 */
	void drawHLine(uint16 x, uint16 y1, uint16 y2);

	/**
	 * Draws a horizontal line.
	 */
	void drawVLine(uint16 x1, uint16 y, uint16 x2);
	void screenUpdate();

	/**
	 * Sets up either a low-res or a high-res 256 color screen.
	 */
	void createScreen(bool hiRes);

	/**
	 * Converts an Amiga palette (up to 16 colors) to a VGA palette, then sets
	 * the VGA palette.
	 */
	void setAmigaPal(uint16 *pal, uint16 numColors);

	/**
	 * Writes any number of the 256 color registers.
	 * @param buf A char pointer which contains the selected color registers.
	 * Each value representing a color register occupies 3 bytes in the array.  The
	 * order is red, green then blue. The first byte in the array is the red component
	 * of the first element selected. The length of the buffer is 3 times the number
	 * of registers selected.
	 * @param first The number of the first color register to write.
	 * @param numReg The number of registers to write.
	 */
	void writeColorRegs(byte *buf, uint16 first, uint16 numReg);
	void setPalette(void *newPal, uint16 numColors);

	/**
	 * Overlays a region on the screen using the desired pen color.
	 */
	void overlayRect(uint16 penColor, uint16 x1, uint16 y1, uint16 x2, uint16 y2);

	/**
	 * Returns the base address of the current VGA display.
	 */
	byte *getCurrentDrawingBuffer();

	/**
	 * Scrolls the display in the x direction by blitting.
	 * The _tempScrollData variable must be initialized to some memory, or this
	 * function will fail.
	 */
	void scrollDisplayX(int16 dx, uint16 x1, uint16 y1, uint16 x2, uint16 y2, byte *buffer);

	/**
	 * Scrolls the display in the y direction by blitting.
	 */
	void scrollDisplayY(int16 dy, uint16 x1, uint16 y1, uint16 x2, uint16 y2, byte *buffer);
	void fade(bool fadein, uint16 res);

	/**
	 * Closes a font and frees all memory associated with it.
	 */
	void closeFont(TextFont **font);

	/**
	 * Returns the length of a text in the specified font.
	 */
	uint16 textLength(TextFont *font, Common::String text);

	/**
	 * Returns the height of a specified font.
	 */
	uint16 textHeight(TextFont *tf);

	/**
	 * Draws the text to the screen.
	 */
	void drawText(TextFont *tf, uint16 x, uint16 y, uint16 color, Common::String text);

	/**
	 * Gets a line of text for flowText; makes sure that its length is less than
	 * or equal to the maximum width.
	 */
	Common::String getLine(TextFont *tf, const char **mainBuffer, uint16 lineWidth);

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
