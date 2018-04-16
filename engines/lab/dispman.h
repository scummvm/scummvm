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

	void createBox(uint16 y2);

	/**
	 * Sets up either a low-res or a high-res 256 color screen.
	 */
	void createScreen(bool hiRes);

	/**
	 * Scrolls the display to black.
	 */
	void doScrollBlack();
	void copyPage(uint16 width, uint16 height, uint16 nheight, uint16 startline, byte *mem);

	/**
	 * Scrolls the display to a new picture from a black screen.
	 */
	void doScrollWipe(const Common::String filename);

	/**
	 * Does the scroll bounce.  Assumes bitmap already in memory.
	 */
	void doScrollBounce();

	/**
	 * Does the transporter wipe.
	 */
	void doTransWipe(const Common::String filename);

	/**
	 * Draws a vertical line.
	 */
	void drawHLine(uint16 x, uint16 y1, uint16 y2, byte color);

	/**
	 * Draws a horizontal line.
	 */
	void drawVLine(uint16 x1, uint16 y, uint16 x2, byte color);

	/**
	 * Draws the text to the screen.
	 */
	void drawText(TextFont *tf, uint16 x, uint16 y, uint16 color, const Common::String text);

	/**
	 * Gets a line of text for flowText; makes sure that its length is less than
	 * or equal to the maximum width.
	 */
	Common::String getLine(TextFont *tf, const char **mainBuffer, uint16 lineWidth);

	/**
	 * Returns the length of a text in the specified font.
	 */
	uint16 textLength(TextFont *font, const Common::String text);

	bool _actionMessageShown;
	Common::File *_curBitmap;
	byte _curVgaPal[256 * 3];
	byte *_currentDisplayBuffer;

public:
	DisplayMan(LabEngine *lab);
	virtual ~DisplayMan();

	void loadPict(const Common::String filename);
	void loadBackPict(const Common::String fileName, uint16 *highPal);

	/**
	 * Reads in a picture into the display bitmap.
	 */
	void readPict(const Common::String filename, bool playOnce = true, bool onlyDiffData = false, byte *memoryBuffer = nullptr);
	void freePict();

	/**
	 * Does a certain number of pre-programmed wipes.
	 */
	void doTransition(TransitionType transitionType, const Common::String filename);

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

	/**
	 * Draws the control panel display.
	 */
	void drawPanel();

	/**
	 * Sets up the Labyrinth screens, and opens up the initial windows.
	 */
	void setUpScreens();

	int longDrawMessage(Common::String str, bool isActionMessage);

	/**
	 * Draws a message to the message box.
	 */
	void drawMessage(Common::String str, bool isActionMessage);

	void setActionMessage(bool val) { _actionMessageShown = val; }

	/**
	 * Fills in a rectangle.
	 */
	void rectFill(uint16 x1, uint16 y1, uint16 x2, uint16 y2, byte color);
	void rectFill(Common::Rect fillRect, byte color);
	void rectFillScaled(uint16 x1, uint16 y1, uint16 x2, uint16 y2, byte color);
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
				bool centerh, bool centerv, bool output, Common::Rect textRect, const char *text, Image *targetImage = nullptr);

	void screenUpdate();

	/**
	 * Converts a 16-color Amiga palette to a VGA palette, then sets
	 * the VGA palette.
	 */
	void setAmigaPal(uint16 *pal);

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
	void checkerBoardEffect(uint16 penColor, uint16 x1, uint16 y1, uint16 x2, uint16 y2);

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
	void fade(bool fadein);

	/**
	 * Closes a font and frees all memory associated with it.
	 */
	void freeFont(TextFont **font);

	/**
	 * Returns the height of a specified font.
	 */
	uint16 textHeight(TextFont *tf);

	bool _longWinInFront;
	bool _lastMessageLong;
	uint32 _screenBytesPerPage;
	int _screenWidth;
	int _screenHeight;
	byte *_displayBuffer;
	uint16 *_fadePalette;
};

} // End of namespace Lab

#endif // LAB_DISPMAN_H
