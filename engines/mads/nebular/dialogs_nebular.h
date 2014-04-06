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

class DialogsNebular: public Dialogs {
	friend class Dialogs;
private:
	int _dialogWidth;
	CapitalizationMode _capitalizationMode;

	DialogsNebular(MADSEngine *vm): Dialogs(vm) {}

	Common::String getVocab(int vocabId);

	bool textNoun(Common::String &dialogText, int nounNum, const Common::String &valStr);

	bool commandCheck(const char *idStr, Common::String &valStr, const Common::String &command);
public:
	virtual void showDialog() {
		warning("TODO: showDialog");
	}
	virtual void showPicture(int objId, int msgId, int arg3) {
		show(msgId);
		warning("TODO: showPicture");
	}
	virtual bool show(int id);
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
	bool show();
};

} // End of namespace Nebular

} // End of namespace MADS

#endif /* MADS_DIALOGS_NEBULAR_H */
