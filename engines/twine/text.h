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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

// MAX_CAR
#define TEXT_MAX_FADE_IN_CHR 32
#define INTER_LINE      38
#define INTER_SPACE     7
#define INTER_LEAVE     2

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
	void drawCharacter(int32 x, int32 y, uint16 character);
	/**
	 * Draw character with shadow
	 * @param x X coordinate in screen
	 * @param y Y coordinate in screen
	 * @param character ascii character to display
	 * @param color character color
	 */
	void drawCharacterShadow(int32 x, int32 y, uint16 character, int32 color, Common::Rect& dirtyRect);
	void initEndPage();
	struct WordSize {
		int32 lenWord = 0;
		int32 sizeWord = 0;
	};
	struct LineCharacter {
		int16 chr = 0;
		int16 width = 0;
	};
	WordSize getNextWord(const char *completeText, char *wordBuf, int32 wordBufSize);
	uint16 getNextChar(const char *&dialogue);
	void getNextLine();
	void appendText(const char *s, uint &i);
	// draw next page arrow polygon
	void renderContinueReadingTriangle();
	/**
	 * @see fadeInCharacters
	 */
	void pushChar(int16 baseX, int16 y, const LineCharacter &chr); // PushCar
	/**
	 * Blend in characters for a text scrolling in
	 *
	 * @see pushChar()
	 */
	void fadeInCharacters(int32 counter, int32 fontColor);

	TextBankId _currentBankIdx = TextBankId::None;

	LineCharacter _bufLine[256];
	const char *_ptDial = nullptr;

	int32 _xDial = 0;
	int32 _yDial = 0;

	/** Current position of in the buffer of characters that are currently faded in */
	const LineCharacter *_ptLine = nullptr;

	int32 _nbLineDial = 0;
	struct BlendInCharacter { // T_STACKCAR
		int16 chr = 0;
		int16 x = 0;
		int16 y = 0;
	};
	BlendInCharacter _stackChar[TEXT_MAX_FADE_IN_CHR]; // StackCar
	int32 _nbChar = 0; // NbCar
	int32 _sizeLine = 0;
	int32 _nbSpace = 0;
	int32 _nbBigSpace = 0;
	int32 _sizeSpace = 0;

	/** Current dialogue text pointer */
	const char *_ptText = nullptr;
	/** Current dialogue text size */
	int32 _currDialTextSize = 0;

	char _currMenuTextBuffer[256];
	TextBankId _currMenuTextBank = TextBankId::None;
	TextId _currMenuTextIndex = TextId::kNone;

	/** Pixel size between dialogue text */
	int32 _interLeave = 0;
	/** Pixel size of the space character - recalculated per per line */
	int32 _interSpace = 0;
	/** Dialogue text color */
	int32 _dialTextColor = 0;

	/** Dialogue text start color for cross coloring dialogues */
	int32 _maxDegrade = 0;
	/** Dialogue text stop color for cross coloring dialogues */
	int32 _minDegrade = 0;
	/**
	 * Dialogue text step size for cross coloring dialogues
	 *
	 * The speed in which the color reaches it's destination color while fading in.
	 */
	int32 _stepDegrade = 0;
	/** Dialogue text buffer size for cross coloring dialogues */
	int32 _nbDegrade = 0;

	// Dial_X1, Dial_Y1
	Common::Rect _dialTextBox { 0, 0, 0, 0};

	int32 _maxLineDial = 0;
	int32 _dialMaxSize = 0;

	bool _isShiftJIS = false;
	bool _isVisualRTL = false;

	bool displayText(TextId index, bool showText, bool playVox, bool loop); // MyDial
public:
	Text(TwinEEngine *engine);
	~Text();

	static const int32 lineHeight = INTER_LINE;

	bool _flagRunningDial = false;
	bool _flagEndDial = false;
	bool _flagEnd3Line = false;
	// renders a triangle if the next side of the text can get activated
	bool _renderTextTriangle = false;
	bool _flagMessageShade = false;
	bool _hasHiddenVox = false;
	int32 _voxHiddenIndex = 0;
	// ---

	const TextEntry *_currDialTextEntry = nullptr; // ordered entry
	Common::String _currentVoxBankFile;
	// used for the android version (dotemu)
	Common::String _currentOggBaseFile;

	bool _showDialogueBubble = true;

	/**
	 * Initialize dialogue
	 * @param bankIdx Text bank index
	 */
	void initDial(TextBankId bankIdx);
	void initSceneTextBank();
	inline TextBankId textBank() const {
		return _currentBankIdx;
	}
	void closeDial();
	/**
	 * Display a certain dialogue text in the screen
	 * @param x X coordinate in screen
	 * @param y Y coordinate in screen
	 * @param dialogue ascii text to display
	 */
	void drawText(int32 x, int32 y, const char *dialogue, bool shadow = false);

	bool drawTextProgressive(TextId index, bool playVox = true, bool loop = true);

	/**
	 * Gets dialogue text width size
	 * @param dialogue ascii text to display
	 */
	int32 sizeFont(const char *dialogue);
	int32 getCharWidth(uint16 chr) const;
	int32 getCharHeight(uint16 chr) const;

	void initDialWindow();
	void secondInitDialWindow();

	void commonOpenDial(TextId index);
	void initLine();
	void initInventoryText(InventoryItems index);
	void initItemFoundText(InventoryItems index);
	void fadeInRemainingChars();
	ProgressiveTextState nextDialChar();

	/**
	 * Set font type parameters
	 * @param interLeave number in pixels of space between characters
	 * @param interSpace number in pixels of the character space
	 */
	void setFont(int32 interLeave, int32 interSpace);

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
	 * @sa initDial()
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

	void bigWinDial();
	void normalWinDial();

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
