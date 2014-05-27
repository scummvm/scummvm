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
#include "mads/dialogs.h"

namespace MADS {

namespace Nebular {

enum CapitalizationMode { kUppercase = 0, kLowercase = 1, kUpperAndLower = 2 };

class DialogsNebular : public Dialogs {
	friend class Dialogs;
private:
	int _dialogWidth;
	CapitalizationMode _capitalizationMode;

	DialogsNebular(MADSEngine *vm): Dialogs(vm), _capitalizationMode(kUppercase) {}

	virtual Common::String getVocab(int vocabId);

	bool textNoun(Common::String &dest, int nounId, const Common::String &source);

	bool commandCheck(const char *idStr, Common::String &valStr, const Common::String &command);
public:
	virtual void showDialog();

	virtual void showItem(int objectId, int messageId, int speech = -1);

	virtual bool show(int messageId, int objectId = -1);
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
	virtual void show();
};

class PictureDialog : public TextDialog {
private:
	int _objectId;
	bool _cyclingActive;
	byte _palette[PALETTE_SIZE];
	uint32 _palFlags[PALETTE_COUNT];
	RGBList _rgbList;
protected:
	virtual void save();

	virtual void restore();
public:
	PictureDialog(MADSEngine *vm, const Common::Point &pos, int maxChars, int objectId);

	virtual ~PictureDialog();
};

enum DialogTextAlign { ALIGN_CENTER = -1, ALIGN_AT_CENTER = -2, ALIGN_RIGHT = -3 };

class ScreenDialog {
	struct DialogLine {
		int _state;
		Common::Point _pos;
		int _textDisplayIndex;
		Common::String _msg;
		Font *_font;
		int _widthAdjust;

		DialogLine();
		DialogLine(const Common::String &s);
	};
protected:
	MADSEngine *_vm;
	MSurface _savedSurface;
	Common::Array<DialogLine> _lines;
	int _v1;
	int _selectedLine;
	bool _dirFlag;
	int _screenId;
	int _menuSpritesIndex;
	int _lineIndex;
	int _textLineCount;

	/**
	 * Reset the lines list for the dialog
	 */
	void clearLines();

	/**
	 * Add a quote to the lines list
	 */
	void addQuote(int id1, int id2, DialogTextAlign align, const Common::Point &pt, Font *font);

	/**
	 * Adds a line to the lines list
	 */
	void addLine(const Common::String &msg, DialogTextAlign align, const Common::Point &pt, Font *font);

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
public:
	/**
	 * Constructor
	 */
	ScreenDialog(MADSEngine *vm);
};

class GameMenuDialog : public ScreenDialog {
private:
	/**
	 * Add the lines for the Game Menu dialog
	 */
	void addLines();
public:
	GameMenuDialog(MADSEngine *vm);

};

} // End of namespace Nebular

} // End of namespace MADS

#endif /* MADS_DIALOGS_NEBULAR_H */
