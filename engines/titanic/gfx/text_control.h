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

#ifndef TITANIC_TEXT_CONTROL_H
#define TITANIC_TEXT_CONTROL_H

#include "common/keyboard.h"
#include "titanic/support/screen_manager.h"
#include "titanic/support/strings.h"

namespace Titanic {

class CScreenManager;
class CTextCursor;
class SimpleFile;

class CTextControl {
	struct ArrayEntry {
		CString _line;
		CString _rgb;
		CString _string3;
	};

private:
	Common::Array<ArrayEntry> _array;
	CString _lines;
	bool _stringsMerged;
	Rect _bounds;
	int _maxCharsPerLine;
	int _lineCount;
	int _displayEndCharIndex;
	int _unused1;
	int _unused2;
	int _unused3;
	int _backR;
	int _backG;
	int _backB;
	int _textR;
	int _textG;
	int _textB;
	int _fontNumber;
	int _npcFlag;
	int _npcId;
	bool _hasBorder;
	int _scrollTop;
	CTextCursor *_textCursor;
private:
	void setupArrays(int count);

	void freeArrays();

	/**
	 * Merges the strings in the strings array
	 */
	void mergeStrings();

	/**
	 * Append text to the current text line
	 */
	void appendText(const CString &str);

	void updateStr3(int lineNum);

	/**
	 * Ensures the Y scrolling for the text is in the valid range
	 */
	void constrainScrollUp(CScreenManager *screenManager);

	/**
	 * Ensures the Y scrolling for the text is in the valid range
	 */
	void constrainScrollDown(CScreenManager *screenManager);

	/**
	 * Get the page height for paging up and down
	 */
	int getPageHeight(CScreenManager *screenManager);
public:
	CTextControl(uint count = 10);

	/**
	 * Set up the control
	 */
	void setup();

	/**
	 * Load the data for the control
	 */
	void load(SimpleFile *file, int param);

	/**
	 * Save the data for the control
	 */
	void save(SimpleFile *file, int indent);

	/**
	 * Set the bounds for the control
	 */
	void setBounds(const Rect &bounds) { _bounds = bounds; }

	/**
	 * Sets the flag for whether to draw a frame border around the control
	 */
	void setHasBorder(bool val) { _hasBorder = val; }

	/**
	 * Draw the control
	 */
	void draw(CScreenManager *screenManager);

	void resize(uint count);

	/**
	 * Returns the text from all the lines as a single string
	 */
	CString getText() const;

	/**
	 * Set the text
	 */
	void setText(const CString &str);

	/**
	 * Set the text
	 */
	void setText(StringId stringId);

	/**
	 * Set text color
	 */
	void setColor(uint col);

	/**
	 * Set text color
	 */
	void setColor(byte r, byte g, byte b);

	/**
	 * Set the color for a line
	 */
	void setLineColor(uint lineNum, byte r, byte g, byte b);

	/**
	 * Gets the text string representing a color encoding
	 */
	static CString getColorText(byte r, byte g, byte b);

	/**
	 * Set the color for a line
	 */
	void setLineColor(uint lineNum, uint col);

	/**
	 * Sets the maximum number of characters per line
	 */
	void setMaxCharsPerLine(int maxChars);

	/**
	 * Delete the last character from the last line
	 */
	void deleteLastChar();

	/**
	 * Sets the current NPC text is being added for
	 */
	void setNPC(int npcFlag, int npcId);

	/**
	 * Returns the character index into _lines of the last
	 * character to be displayed on-screen
	 */
	int displayEndIndex() const { return _displayEndCharIndex; }

	/**
	 * Scroll the text up
	 */
	void scrollUp(CScreenManager *screenManager);

	/**
	 * Scroll the text down
	 */
	void scrollDown(CScreenManager *screenManager);

	/**
	 * Scroll the text up one page
	 */
	void scrollUpPage(CScreenManager *screenManager);

	/**
	 * Scroll the text down one page
	 */
	void scrollDownPage(CScreenManager *screenManager);

	/**
	 * Scroll to the top of the text
	 */
	void scrollToTop(CScreenManager *screenManager);

	/**
	 * Scroll to the bottom of the text
	 */
	void scrollToBottom(CScreenManager *screenManager);

	/**
	 * Add a line to the text
	 */
	void addLine(const CString &str);

	/**
	 * Add a line to the text
	 */
	void addLine(const CString &str, uint color);

	/**
	 * Add a line to the text
	 */
	void addLine(const CString &str, byte r, byte g, byte b);

	/**
	 * Handles character processing to add or remove characters to
	 * the current text line
	 * @returns		True if the Enter key was pressed
	 */
	bool handleKey(char c);

	/**
	 * Attaches the current system cursor to the text control,
	 * and give it suitable defaults
	 */
	void showCursor(int mode);

	/**
	 * Removes the cursor attached to the text
	 */
	void hideCursor();

	/**
	 * Get an NPC Number embedded within on-screen text.
	 * Used by the PET log to encode which NPC spoke
	 * @param ident			Npc Type. Always passed as 1
	 * @param startIndex	Starting index to scan backwards
	 *		through the log text to find an NPC ident sequence
	 */
	int getNPCNum(uint ident, uint startIndex);

	/**
	 * Replaces any occurrences of line colors that appear in the
	 * first list with the entry at the same index in the dest list
	 */
	void remapColors(uint count, uint *srcColors, uint *destColors);

	/**
	 * Set the font number to use
	 */
	void setFontNumber(int fontNumber);

	/**
	 * Get the width of the text
	 */
	int getTextWidth(CScreenManager *screenManager);

	/**
	 * Get the required height to draw the text
	 */
	int getTextHeight(CScreenManager *screenManager);
};

} // End of namespace Titanic

#endif /* TITANIC_TEXT_CONTROL_H */
