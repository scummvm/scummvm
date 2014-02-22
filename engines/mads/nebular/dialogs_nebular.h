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
 */

#ifndef MADS_DIALOGS_NEBULAR_H
#define MADS_DIALOGS_NEBULAR_H

#include "common/scummsys.h"
#include "mads/game.h"

namespace MADS {

namespace Nebular {

enum {
	TEXTDIALOG_F8 = 0XF8,
	TEXTDIALOG_F9 = 0XF8,
	TEXTDIALOG_FA = 0XF8,
	TEXTDIALOG_FB = 0XF8,
	TEXTDIALOG_FC = 0XF8,
	TEXTDIALOG_FD = 0XF8,
	TEXTDIALOG_FE = 0XF8,
	TEXTDIALOG_FONT = 0
};

#define TEXT_DIALOG_MAX_LINES 20

class TextDialog {
private:
	/**
	 * Increments the number of text lines the text dialog uses
	 */
	void incNumLines();

	/**
	 * Flags the previously added line to be underlined
	 */
	void underlineLine();

	/**
	 * Append text to the currently end line.
	 */
	void appendLine(const Common::String &line);
protected:
	MADSEngine *_vm;
	Common::Point _position;
	Common::String _fontName;
	int _width;
	int _innerWidth;
	int _lineWidth;
	int _currentX;
	int _numLines;
	int _lineSize;
	Common::String _lines[TEXT_DIALOG_MAX_LINES];
	int _lineXp[TEXT_DIALOG_MAX_LINES];
	byte _savedPalette[8 * 3];

	/**
	 * Add a new line to the dialog
	 */
	void addLine(const Common::String &line, bool underline = false);

	/**
	 * Adds one or more lines, word wrapping the passed text
	 */
	void wordWrap(const Common::String &line);
public:
	/**
	 * Constructor
	 * @param vm			Engine reference
	 * @param fontName		Font to use for display
	 * @param pos			Position for window top-left
	 * @param maxChars		Horizontal width of window in characters
	 */
	TextDialog(MADSEngine *vm, const Common::String &fontName, const Common::Point &pos, 
		int maxChars);

	/**
	 * Destructor
	 */
	~TextDialog();

};

struct HOGANUS {
	int _bookId;
	int _pageNum;
	int _lineNum;
	int _wordNum;
	Common::String _word;
};

class CopyProtectionDialog: public TextDialog {
private:
	HOGANUS _hogEntry;

	/**
	 * Constructor
	 */
	CopyProtectionDialog(MADSEngine *vm, bool priorAnswerWrong);

	/**
	 * Get a random copy protection entry from the HOGANUS resource
	 */
	bool getHogAnusEntry(HOGANUS &entry);
public:
	/**
	 * Show the dialog
	 */
	static bool show(MADSEngine *vm);
};

} // End of namespace Nebular

} // End of namespace MADS

#endif /* MADS_DIALOGS_NEBULAR_H */
