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

#ifndef MADS_DIALOGS_H
#define MADS_DIALOGS_H

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/msurface.h"

namespace MADS {

class Dialog {
protected:
	MADSEngine *_vm;
	MSurface *_savedSurface;
	Common::Point _position;
	int _width;
	int _height;

	/**
	 * Save the section of the passed surface the dialog will cover.
	 * @param s		Screen surface to save
	 */
	void save(MSurface *s);

	/**
	 * Restore saved dialog surface
	 * @param s		Screen surface to restore to.
	 */
	void restore(MSurface *s);

	/**
	 * Draws the content of a dialog with a gravelly alternating color.
	 */
	void drawContent(const Common::Rect &r, int seed, byte color1, byte color2);
protected:
	/**
	 * Draw the dialog
	 */
	virtual void draw();
public:
	/**
	 * Constructor
	 */
	Dialog(MADSEngine *vm);

	/**
	 * Destructor
	 */
	virtual ~Dialog();
};

enum {
	TEXTDIALOG_CONTENT1 = 0XF8,
	TEXTDIALOG_CONTENT2 = 0XF9,
	TEXTDIALOG_EDGE = 0XFA,
	TEXTDIALOG_BACKGROUND = 0XFB,
	TEXTDIALOG_FC = 0XFC,
	TEXTDIALOG_FD = 0XFD,
	TEXTDIALOG_FE = 0XFE,
	TEXTDIALOG_BLACK = 0
};

#define TEXT_DIALOG_MAX_LINES 20

class TextDialog: protected Dialog {
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

	/**
	 * Clean up after finishing displaying the dialog
	 */
	void restorePalette();
protected:
	Common::String _fontName;
	int _innerWidth;
	int _lineWidth;
	int _currentX;
	int _numLines;
	int _lineSize;
	int _askXp;
	int _askLineNum;
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

	/**
	 * Adds an input area following previously added text
	 */
	void addInput();
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
	virtual ~TextDialog();

	/**
	 * Draw the dialog
	 */
	virtual void draw();

	/**
	 * Draw the dialog along with any input box
	 */
	void drawWithInput();
};

class MessageDialog: protected TextDialog {
public:
	MessageDialog(MADSEngine *vm, int lines, ...);

	virtual ~MessageDialog() {}

	/**
	 * Show the dialog, and wait until a key or mouse press.
	 */
	void show();
};

enum DialogId {
	DIALOG_NONE = 0, DIALOG_GAME_MENU = 1, DIALOG_SAVE = 2, DIALOG_RESTORE = 3,
	DIALOG_OPTIONS = 4, DIALOG_DIFFICULTY = 5, DIALOG_ERROR = 6
};

class Dialogs {
protected:
	MADSEngine *_vm;

	Dialogs(MADSEngine *vm);
public:
	static Dialogs *init(MADSEngine *vm);
public:
	Common::Point _defaultPosition;
	DialogId _pendingDialog;

	virtual ~Dialogs() {}

	virtual void showDialog() = 0;
	virtual void showPicture(int objId, int msgId, int arg3 = 0) = 0;
	void show(int msgId);
};

} // End of namespace MADS

#endif /* MADS_DIALOGS_H */
