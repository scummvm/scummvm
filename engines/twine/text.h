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
#include "common/rect.h"
#include "twine/shared.h"

namespace TwinE {

class TextEntry;

#define TEXT_MAX_FADE_IN_CHR 32

#define COLOR_BLACK 0
#define COLOR_BRIGHT_BLUE 4
#define COLOR_9 9
#define COLOR_14 14
// color 1 = yellow
// color 2 - 15 = white
// color 16 - 19 = brown
// color 20 - 24 = orange to yellow
// color 25 orange
// color 26 - 30 = bright gray or white
#define COlOR_31 31 // green dark
#define COlOR_47 47 // green bright
#define COLOR_48 48 // brown dark
#define COLOR_63 63 // brown bright
#define COLOR_64 64 // blue dark
#define COLOR_68 68 // blue
#define COLOR_73 73 // blue
#define COLOR_75 75
#define COLOR_79 79 // blue bright
#define COLOR_80 80
#define COLOR_91 91
#define COLOR_BRIGHT_BLUE2 69
#define COLOR_WHITE 15
#define COLOR_GOLD 155
#define COLOR_158 158

enum class ProgressiveTextState {
	End = 0,				/**< Text has reached its end and we are waiting for user input */
	ContinueRunning = 1,	/**< Text is fading in */
	NextPage = 2			/**< Waiting for user input to abort or start the next page to fade in */
};

class TwinEEngine;

class Text {
private:
	TwinEEngine *_engine;
	void initVoxBank(TextBankId bankIdx);
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
	void drawCharacterShadow(int32 x, int32 y, uint8 character, int32 color, Common::Rect& dirtyRect);
	void initProgressiveTextBuffer();
	struct WordSize {
		int32 inChar = 0;
		int32 inPixel = 0;
	};
	WordSize getWordSize(const char *completeText, char *wordBuf, int32 wordBufSize);
	void processTextLine();
	// draw next page arrow polygon
	void renderContinueReadingTriangle();
	/**
	 * @see fadeInCharacters
	 */
	void fillFadeInBuffer(int16 x, int16 y, int16 chr);
	/**
	 * Blend in characters for a text scrolling in
	 *
	 * @see fillFadeInBuffer
	 * @param counter The amount of characters to handle - max 32
	 */
	void fadeInCharacters(int32 counter, int32 fontColor);

	TextBankId _currentBankIdx = TextBankId::None;

	char _progressiveTextBuffer[256] {'\0'};
	const char *_currentTextPosition = nullptr;

	int32 _dialTextXPos = 0;
	int32 _dialTextYPos = 0;

	/** Current position of in the buffer of characters that are currently faded in */
	char *_progressiveTextBufferPtr = nullptr;

	int32 _dialTextBoxCurrentLine = 0;
	struct BlendInCharacter {
		int16 chr = 0;
		int16 x = 0;
		int16 y = 0;
	};
	BlendInCharacter _fadeInCharacters[TEXT_MAX_FADE_IN_CHR];
	int32 _fadeInCharactersPos = 0;

	/** Current dialogue text pointer */
	const char *_currDialTextPtr = nullptr;
	/** Current dialogue text size */
	int32 _currDialTextSize = 0;

	char _currMenuTextBuffer[256];
	TextBankId _currMenuTextBank = TextBankId::None;
	TextId _currMenuTextIndex = TextId::kNone;

	/** Pixel size between dialogue text */
	int32 _dialSpaceBetween = 0;
	/** Pixel size of the space character - recalculated per per line */
	int32 _dialCharSpace = 0;
	/** Dialogue text color */
	int32 _dialTextColor = 0;

	/** Dialogue text start color for cross coloring dialogues */
	int32 _dialTextStartColor = 0;
	/** Dialogue text stop color for cross coloring dialogues */
	int32 _dialTextStopColor = 0;
	/**
	 * Dialogue text step size for cross coloring dialogues
	 *
	 * The speed in which the color reaches it's destination color while fading in.
	 */
	int32 _dialTextStepSize = 0;
	/** Dialogue text buffer size for cross coloring dialogues */
	int32 _dialTextBufferSize = 0;

	Common::Rect _dialTextBox { 0, 0, 0, 0};

	int32 _dialTextBoxLines = 0; // dialogueBoxParam1
	int32 _dialTextBoxMaxX = 0; // dialogueBoxParam2

	bool displayText(TextId index, bool showText, bool playVox, bool loop);
public:
	Text(TwinEEngine *engine);
	~Text();

	static const int32 lineHeight = 38;

	// TODO: refactor all this variables and related functions
	bool _hasValidTextHandle = false;
	// renders a triangle if the next side of the text can get activated
	bool renderTextTriangle = false;
	bool drawTextBoxBackground = false;
	bool hasHiddenVox = false; // printTextVar5
	int32 voxHiddenIndex = 0;
	// ---

	const TextEntry *currDialTextEntry = nullptr; // ordered entry
	Common::String currentVoxBankFile;

	bool showDialogueBubble = true;

	/**
	 * Initialize dialogue
	 * @param bankIdx Text bank index
	 */
	void initTextBank(TextBankId bankIdx);
	void initSceneTextBank();
	inline TextBankId textBank() const {
		return _currentBankIdx;
	}

	/**
	 * Display a certain dialogue text in the screen
	 * @param x X coordinate in screen
	 * @param y Y coordinate in screen
	 * @param dialogue ascii text to display
	 */
	void drawText(int32 x, int32 y, const char *dialogue);

	bool drawTextProgressive(TextId index, bool playVox = true, bool loop = true);

	/**
	 * Gets dialogue text width size
	 * @param dialogue ascii text to display
	 */
	int32 getTextSize(const char *dialogue);
	int32 getCharWidth(uint8 chr) const;
	int32 getCharHeight(uint8 chr) const;

	void initDialogueBox();
	void initInventoryDialogueBox();

	void initText(TextId index);
	void initInventoryText(InventoryItems index);
	void initItemFoundText(InventoryItems index);
	void fadeInRemainingChars();
	ProgressiveTextState updateProgressiveText();

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
	 * Get dialogue text into text buffer from the currently loaded text bank
	 * @sa initTextBank()
	 * @param index dialogue index
	 */
	bool getText(TextId index);

	/**
	 * Gets menu dialogue text
	 * @param index text index to display
	 * @param text dialogue text buffer to display
	 * @param textSize The size of the text buffer
	 */
	bool getMenuText(TextId index, char *text, uint32 textSize);

	void textClipFull();
	void textClipSmall();

	void drawAskQuestion(TextId index);
	void drawHolomapLocation(TextId index);

	bool playVox(const TextEntry *text);
	bool playVoxSimple(const TextEntry *text);
	bool stopVox(const TextEntry *text);
	bool initVoxToPlay(const TextEntry *text);
	bool initVoxToPlayTextId(TextId index);
};

} // namespace TwinE

#endif
