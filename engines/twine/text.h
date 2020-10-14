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

#ifndef TWINE_TEXT_H
#define TWINE_TEXT_H

#include "common/scummsys.h"
#include "common/str.h"

namespace TwinE {

class TwinEEngine;
class Text {
private:
	TwinEEngine *_engine;
	void initVoxBank(int32 bankIdx);
	/**
	 * Draw a certain character in the screen
	 * @param x X coordinate in screen
	 * @param y Y coordinate in screen
	 * @param character ascii character to display
	 */
	void drawCharacter(int32 x, int32 y, uint8 character);
	/**
	 * Draw character with shadow
	 * @param x X coordinate in screen
	 * @param y Y coordinate in screen
	 * @param character ascii character to display
	 * @param color character color
	 */
	void drawCharacterShadow(int32 x, int32 y, uint8 character, int32 color);
	void initProgressiveTextBuffer();
	void printText8Sub4(int16 a, int16 b, int16 c);
	struct WordSize {
		int32 inChar = 0;
		int32 inPixel = 0;
	};
	WordSize getWordSize(const char *arg1, char *arg2);
	void processTextLine();
	// draw next page arrow polygon
	void printText10Sub();
	void printText10Sub2();
	void TEXT_GetLetterSize(uint8 character, int32 *pLetterWidth, int32 *pLetterHeight, uint8 *pFont);
	/**
	 * Copy dialogue text
	 * @param src source text buffer
	 * @param dst destination text buffer
	 * @param size text size
	 */
	void copyText(const char *src, char *dst, int32 size);

	// RECHECK THIS LATER
	int32 currentBankIdx = -1; // textVar1
	char textVar2[256] = "";
	uint8 textVar3 = 0u;

	/** Dialogue text pointer */
	char *dialTextPtr = nullptr; // bufText
	/** Dialogue entry order pointer */
	char *dialOrderPtr = nullptr; // bufOrder
	/** Number of dialogues text entries */
	int16 numDialTextEntries = 0;

	const int16 spaceChar = 0x20;

	// TODO: refactor all this variables and related functions
	char buf1[256] = "";
	char buf2[256] = "";
	char *printText8Ptr1 = nullptr;
	char *printText8Ptr2 = nullptr;
	int32 printText8Var1 = 0;
	int32 printText8Var2 = 0;
	int32 printText8Var3 = 0;
	int32 TEXT_CurrentLetterX = 0;
	int32 printText8Var5 = 0;
	int32 printText8Var6 = 0;
	int32 TEXT_CurrentLetterY = 0;
	char *printText8Var8 = nullptr;
	int32 printText10Var1 = 0;
	int32 addLineBreakX = 0;
	int16 pt8s4[96];
	int32 printText8PrepareBufferVar2 = 0;
	// ---
public:
	Text(TwinEEngine *engine) : _engine(engine) {}

	/** Current text bank */
	int32 currentTextBank = 0;
	/** Current dialogue text size */
	int32 currDialTextSize = 0;
	/** Current dialogue text pointer */
	char *currDialTextPtr = nullptr;

	/** Font buffer pointer */
	uint8 *fontPtr = nullptr;

	/** Dialogue text size */
	int32 dialTextSize = 0;
	/** Pixel size between dialogue text */
	int32 dialSpaceBetween = 0;
	/** Pixel size of the space character */
	int32 dialCharSpace = 0;
	/** Dialogue text color */
	int32 dialTextColor = 0;

	/** Dialogue text start color for cross coloring dialogues */
	int32 dialTextStartColor = 0;
	/** Dialogue text stop color for cross coloring dialogues */
	int32 dialTextStopColor = 0;
	/** Dialogue text step size for cross coloring dialogues */
	int32 dialTextStepSize = 0;
	/** Dialogue text buffer size for cross coloring dialogues */
	int32 dialTextBufferSize = 0;

	int32 dialTextBoxLeft = 0;   // dialogueBoxLeft
	int32 dialTextBoxTop = 0;    // dialogueBoxTop
	int32 dialTextBoxRight = 0;  // dialogueBoxRight
	int32 dialTextBoxBottom = 0; // dialogueBoxBottom

	int32 dialTextBoxParam1 = 0; // dialogueBoxParam1
	int32 dialTextBoxParam2 = 0; // dialogueBoxParam2

	// TODO: refactor all this variables and related functions
	int32 printTextVar13 = 0;
	int32 newGameVar4 = 0;
	int32 newGameVar5 = 0;
	int32 hasHiddenVox = 0; // printTextVar5
	int32 voxHiddenIndex = 0;
	// ---

	int32 currDialTextEntry = 0; // ordered entry
	int32 nextDialTextEntry = 0; // ordered entry
	Common::String currentVoxBankFile;

	int32 showDialogueBubble = 0;

	/**
	 * Initialize dialogue
	 * @param bankIdx Text bank index
	 */
	void initTextBank(int32 bankIdx);

	/**
	 * Display a certain dialogue text in the screen
	 * @param x X coordinate in screen
	 * @param y Y coordinate in screen
	 * @param dialogue ascii text to display
	 */
	void drawText(int32 x, int32 y, const char *dialogue);

	void drawTextFullscreen(int32 index);

	/**
	 * Gets dialogue text width size
	 * @param dialogue ascii text to display
	 */
	int32 getTextSize(const char *dialogue);

	void initDialogueBox();
	void initInventoryDialogueBox();

	void initText(int32 index);
	int printText10();

	void setFont(uint8 *font, int32 spaceBetween, int32 charSpace);

	/**
	 * Set font type parameters
	 * @param spaceBetween number in pixels of space between characters
	 * @param charSpace number in pixels of the character space
	 */
	void setFontParameters(int32 spaceBetween, int32 charSpace);

	/**
	 * Set the font cross color
	 * @param color color number to choose
	 */
	void setFontCrossColor(int32 color);

	/**
	 * Set the font color
	 * @param color color number to choose
	 */
	void setFontColor(int32 color);

	/**
	 * Set font color parameters to precess cross color display
	 * @param stopColor color number to stop
	 * @param startColor color number to start
	 * @param stepSize step size to change between those colors
	 */
	void setTextCrossColor(int32 stopColor, int32 startColor, int32 stepSize);

	/**
	 * Get dialogue text into text buffer
	 * @param index dialogue index
	 */
	bool getText(int32 index);

	/**
	 * Gets menu dialogue text
	 * @param index text index to display
	 * @param text dialogue text buffer to display
	 * @param textSize The size of the text buffer
	 */
	void getMenuText(int32 index, char *text, uint32 textSize);

	void textClipFull();
	void textClipSmall();

	void drawAskQuestion(int32 index);

	int32 playVox(int32 index);
	int32 playVoxSimple(int32 index);
	void stopVox(int32 index);
	int32 initVoxToPlay(int32 index);
};

} // namespace TwinE

#endif
