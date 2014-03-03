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

#ifndef MADS_ACTION_H
#define MADS_ACTION_H

#include "common/scummsys.h"
#include "common/str.h"

namespace MADS {

enum ActionMode { ACTMODE_NONE = 0, ACTMODE_VERB = 1, ACTMODE_OBJECT = 3, ACTMODE_TALK = 6 };
enum ActionMode2 { ACTMODE2_0 = 0, ACTMODE2_2 = 2, ACTMODE2_4 = 4, ACTMODE2_5 = 5 };
enum AbortTimerMode { ABORTMODE_0 = 0, ABORTMODE_1 = 1, ABORTMODE_2 = 2 };

enum {
	VERB_NONE = 0,
	VERB_LOOK = 3,
	VERB_TAKE = 4,
	VERB_PUSH = 5,
	VERB_OPEN = 6,
	VERB_PUT = 7,
	VERB_TALKTO = 8,
	VERB_GIVE = 9,
	VERB_PULL = 10,
	VERB_CLOSE = 11,
	VERB_THROW = 12,
	VERB_WALKTO = 13
};

class MADSEngine;

struct ActionDetails {
	int verbId;
	int objectNameId;
	int indirectObjectId;
};

struct MADSActionSavedFields {
	int articleNumber;
	int actionMode;
	int actionMode2;
	bool lookFlag;
	int selectedRow;
};

class MADSAction {
private:
	MADSEngine *_vm;
	Common::String _statusText;
	Common::String _dialogTitle;

	void appendVocab(int vocabId, bool capitalise = false);
public:
	ActionDetails _action, _activeAction;
	int _currentAction;
	int8 _flags1, _flags2;
	ActionMode _actionMode;
	ActionMode2 _actionMode2;
	int _articleNumber;
	bool _lookFlag;
	int _selectedRow;
	bool _textChanged;
	int _selectedAction;
	bool _startWalkFlag;
	int _statusTextIndex;
	int _hotspotId;
	MADSActionSavedFields _savedFields;
	bool _walkFlag;

	// Unknown fields
	int16 _v86F3A;
	int16 _v86F42;
	int16 _v86F4E;
	bool _v86F4A;
	int16 _v86F4C;
	int _v83338;
	bool _inProgress;
	AbortTimerMode _v8453A;

public:
	MADSAction(MADSEngine *vm);

	void clear();
	void set();
	const Common::String &statusText() const { return _statusText; }
	void refresh();
	void startAction();
	void checkAction();
	bool isAction(int verbId, int objectNameId = 0, int indirectObjectId = 0);
	
	void checkActionAtMousePos();
};

} // End of namespace MADS

#endif /* MADS_ACTION_H */
