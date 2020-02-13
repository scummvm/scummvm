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

#ifndef MADS_DIALOGS_H
#define MADS_DIALOGS_H

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/font.h"
#include "mads/msurface.h"

namespace MADS {

#define DIALOG_TOP 22
#define POPUP_CENTER 0x8000

class Dialog {
private:
	void setDialogPalette();
protected:
	MADSEngine *_vm;
	MSurface *_savedSurface;
	Common::Point _position;
	int _width;
	int _height;
	byte _dialogPalette[8 * 3];

	int TEXTDIALOG_CONTENT1;
	int TEXTDIALOG_CONTENT2;
	int TEXTDIALOG_EDGE;
	int TEXTDIALOG_BACKGROUND;
	int TEXTDIALOG_FC;
	int TEXTDIALOG_FD;
	int TEXTDIALOG_FE;
	int TEXTDIALOG_BLACK;
protected:
	/**
	 * Draw the dialog
	 */
	virtual void draw();

	/**
	 * Calculate bounds for the dialog
	 */
	virtual void calculateBounds();

	/**
	* Save the section of the passed surface the dialog will cover.
	*/
	virtual void save();

	/**
	* Restore saved dialog surface
	*/
	virtual void restore();

	/**
	* Draws the content of a dialog with a gravelly alternating color.
	*/
	void drawContent(const Common::Rect &r, int seed, byte color1, byte color2);
public:
	/**
	 * Constructor
	 */
	Dialog(MADSEngine *vm);

	/**
	 * Destructor
	 */
	virtual ~Dialog();

	/**
	 * Return the bounds of the dialog.
	 */
	Common::Rect getBounds() const {
		return Common::Rect(_position.x, _position.y,
			_position.x + _width, _position.y + _height);
	}
};

#define TEXT_DIALOG_MAX_LINES 20

class TextDialog : protected Dialog {
private:
	/**
	 * Append text to the currently end line.
	 */
	void appendLine(const Common::String &line);

	/**
	 * Clean up after finishing displaying the dialog
	 */
	void restorePalette();

	/**
	 * Used by the constructors to initialize the dialog fields
	 */
	void init(int maxTextChars);
protected:
	Font *_font;
	int _innerWidth;
	int _lineWidth;
	int _currentX;
	int _numLines;
	int _lineSize;
	int _askXp;
	int _askLineNum;
	Common::String _lines[TEXT_DIALOG_MAX_LINES];
	int _lineXp[TEXT_DIALOG_MAX_LINES];
	SpriteAsset *_edgeSeries;
	MSurface *_portrait;
	int _piecesPerCenter;
	int _fontSpacing;

	/**
	 * Calculate the bounds for the dialog
	 */
	void calculateBounds() override;
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
	 * Constructor
	 * @param vm			Engine reference
	 * @param fontName		Font to use for display
	 * @param pos			Position for window top-left
	 * @param portrait		Speaker portrait to show in dialog
	 * @param maxTextChars	Horizontal width of text portion of window in characters
	 */
	TextDialog(MADSEngine *vm, const Common::String &fontName, const Common::Point &pos,
		MSurface *portrait, int maxTextChars);

	/**
	 * Destructor
	 */
	~TextDialog() override;

	/**
	 * Draw the dialog
	 */
	void draw() override;

	/**
	 * Draw the dialog along with any input box
	 */
	void drawWithInput();

	/**
	* Add a new line to the dialog
	*/
	void addLine(const Common::String &line, bool underline = false);

	/**
	* Adds one or more lines, word wrapping the passed text
	*/
	void wordWrap(const Common::String &line);

	/**
	* Increments the number of text lines the text dialog uses
	*/
	void incNumLines();

	/**
	* Adds an input area following previously added text
	*/
	void addInput();

	/**
	 * Adds a bar line to separate sections of text
	 */
	void addBarLine();

	/**
	* Flags the previously added line to be underlined
	*/
	void underlineLine();

	void downPixelLine();

	/**
	 * Set the x position for the given line
	 */
	void setLineXp(int xp);

	/**
	 * Estimates the maximum dialog length for text dialogs with icons
	 */
	int estimatePieces(int maxLen);

	/**
	 * Show the dialog, and wait until a key or mouse press.
	 */
	virtual void show();
};

class MessageDialog : public TextDialog {
public:
	MessageDialog(MADSEngine *vm, int lines, ...);

	~MessageDialog() override {}
};

enum DialogId {
	DIALOG_NONE = 0, DIALOG_GAME_MENU = 1, DIALOG_SAVE = 2, DIALOG_RESTORE = 3,
	DIALOG_OPTIONS = 4, DIALOG_DIFFICULTY = 5, DIALOG_ERROR = 6,
	DIALOG_MAIN_MENU = 7, DIALOG_TEXTVIEW = 8, DIALOG_ANIMVIEW = 9,
	DIALOG_ADVERT = 10
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
	int _indexList[10];

	virtual ~Dialogs() {}

	virtual void showDialog() = 0;
	virtual void showItem(int objectId, int messageId, int speech = 0) = 0;
	virtual Common::String getVocab(int vocabId) = 0;
	virtual bool show(int messageId, int objectId = -1) = 0;

	/**
	* Show a spinning picture of an object, used in V2+ games
	*/
	virtual void spinObject(int idx) { warning("TODO: spinObject"); }
};

class FullScreenDialog: public EventTarget {
protected:
	/**
	 * Engine reference
	 */
	MADSEngine *_vm;

	/**
	 * Screen/scene to show background from
	 */
	int _screenId;

	/**
	 * Flag for palette initialization
	 */
	bool _palFlag;

	/**
	 * Handles displaying the screen background and dialog
	 */
	virtual void display();
public:
	/**
	 * Constructor
	 */
	FullScreenDialog(MADSEngine *vm);

	~FullScreenDialog() override;
};

} // End of namespace MADS

#endif /* MADS_DIALOGS_H */
