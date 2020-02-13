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

#ifndef MADS_DIALOGS_NEBULAR_H
#define MADS_DIALOGS_NEBULAR_H

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/dialogs.h"

namespace MADS {

namespace Nebular {

enum CapitalizationMode { kUppercase = 0, kLowercase = 1, kUpperAndLower = 2 };

class DialogsNebular : public Dialogs {
	friend class Dialogs;
private:
	int _dialogWidth;
	CapitalizationMode _capitalizationMode;

	DialogsNebular(MADSEngine *vm): Dialogs(vm), _capitalizationMode(kUppercase), _dialogWidth(0) {}

	Common::String getVocab(int vocabId) override;

	bool textNoun(Common::String &dest, int nounId, const Common::String &source);

	bool commandCheck(const char *idStr, Common::String &valStr, const Common::String &command);

	void showScummVMSaveDialog();
	void showScummVMRestoreDialog();

public:
	void showDialog() override;

	void showItem(int objectId, int messageId, int speech = -1) override;

	bool show(int messageId, int objectId = -1) override;
};

struct HOGANUS {
	int _bookId;
	int _pageNum;
	int _lineNum;
	int _wordNum;
	Common::String _word;
};

class CopyProtectionDialog : public TextDialog {
private:
	HOGANUS _hogEntry;
	Common::String _textInput;

	/**
	 * Get a random copy protection entry from the HOGANUS resource
	 */
	bool getHogAnusEntry(HOGANUS &entry);
public:
	/**
	 * Constructor
	 */
	CopyProtectionDialog(MADSEngine *vm, bool priorAnswerWrong);

	/**
	 * Show the dialog
	 */
	void show() override;

	bool isCorrectAnswer();
};

class PictureDialog : public TextDialog {
private:
	int _objectId;
	bool _cyclingActive;
	byte _palette[PALETTE_SIZE];
	uint32 _palFlags[PALETTE_COUNT];
	RGBList _rgbList;
protected:
	void save() override;

	void restore() override;
public:
	PictureDialog(MADSEngine *vm, const Common::Point &pos, int maxChars, int objectId);

	~PictureDialog() override;
};

enum DialogTextAlign { ALIGN_NONE = 0, ALIGN_CENTER = -1, ALIGN_AT_CENTER = -2, ALIGN_RIGHT = -3 };

enum DialogState { DLGSTATE_UNSELECTED = 0, DLGSTATE_SELECTED = 1, DLGSTATE_FOCUSED = 2 };

class GameDialog: public FullScreenDialog {
	struct DialogLine {
		bool _active;
		DialogState _state;
		Common::Point _pos;
		int _textDisplayIndex;
		Common::String _msg;
		Font *_font;
		int _widthAdjust;

		DialogLine();
		DialogLine(const Common::String &s);
	};
protected:
	Common::Array<DialogLine> _lines;
	int _tempLine;
	bool _movedFlag;
	bool _redrawFlag;
	int _selectedLine;
	bool _dirFlag;
	int _menuSpritesIndex;
	int _lineIndex;
	int _textLineCount;

	/**
	 * Display the dialog
	 */
	void display() override;

	/**
	 * Reset the lines list for the dialog
	 */
	void clearLines();

	/**
	 * Setup lines to be clickable
	 */
	void setClickableLines();

	/**
	 * Add a quote to the lines list
	 */
	void addQuote(int id1, int id2, DialogTextAlign align, const Common::Point &pt, Font *font = nullptr);

	/**
	 * Adds a line to the lines list
	 */
	void addLine(const Common::String &msg, DialogTextAlign align, const Common::Point &pt, Font *font = nullptr);

	/**
	 * Initializes variables
	 */
	void initVars();

	/**
	 * Sets the display for the screen background behind the dialog
	 */
	void setFrame(int frameNumber, int depth);

	/**
	 * Choose the background to display for the dialog
	 */
	void chooseBackground();

	/**
	 * Handle events whilst the dialog is active
	 */
	void handleEvents();

	/**
	 * Refresh the display of the dialog's text
	 */
	void refreshText();
public:
	/**
	 * Constructor
	 */
	GameDialog(MADSEngine *vm);

	/**
	 * Destructor
	 */
	~GameDialog() override;

	/**
	 * Show the dialog
	 */
	virtual void show();
};

class DifficultyDialog : public GameDialog {
private:
	/**
	 * Set the lines for the dialog
	 */
	void setLines();
public:
	DifficultyDialog(MADSEngine *vm);

	/**
	 * Display the dialog
	 */
	void display() override;

	/**
	* Show the dialog
	*/
	void show() override;
};

class GameMenuDialog : public GameDialog {
private:
	/**
	 * Set the lines for the dialog
	 */
	void setLines();
public:
	GameMenuDialog(MADSEngine *vm);

	/**
	* Display the dialog
	*/
	void display() override;

	/**
	* Show the dialog
	*/
	void show() override;
};

class OptionsDialog : public GameDialog {
private:
	/**
	 * Set the lines for the dialog
	 */
	void setLines();

	/**
	 * Gets the quote to be shown for an option
	 */
	int getOptionQuote(int option);
public:
	OptionsDialog(MADSEngine *vm);

	/**
	* Display the dialog
	*/
	void display() override;

	/**
	* Show the dialog
	*/
	void show() override;
};

} // End of namespace Nebular

} // End of namespace MADS

#endif /* MADS_DIALOGS_NEBULAR_H */
