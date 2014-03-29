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

enum ActionMode { ACTIONMODE_NONE = 0, ACTIONMODE_VERB = 1, ACTIONMODE_OBJECT = 3, ACTIONMODE_TALK = 6 };
enum ActionMode2 { ACTIONMODE2_0 = 0, ACTIONMODE2_2 = 2, ACTIONMODE2_4 = 4, ACTIONMODE2_5 = 5 };
enum AbortTimerMode { ABORTMODE_0 = 0, ABORTMODE_1 = 1, ABORTMODE_2 = 2 };

enum InterAwaiting {
	AWAITING_NONE = 0,
	AWAITING_COMMAND = 1,       // Initial state: waiting for a command verb
	AWAITING_THIS = 2,			// Waiting for object
	AWAITING_THAT = 3,			// Waiting for a second object
	AWAITING_RIGHT_MOUSE = 4	// Waiting for mouse button release
};

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
	int _verbId;
	int _objectNameId;
	int _indirectObjectId;
};

struct ActionSavedFields {
	int _actionMode;
	int _selectedRow;
	int _hotspotId;
	int _v86F3A;
	int _actionMode2;
	int _v86F42;
	int _articleNumber;
	int _lookFlag;
};

class MADSAction {
private:
	MADSEngine *_vm;
	Common::String _statusText;
	Common::String _dialogTitle;

	void appendVocab(int vocabId, bool capitalise = false);

	void checkCustomDest(int v);
public:
	ActionDetails _action, _activeAction;
	int8 _flags1, _flags2;
	ActionMode _actionMode;
	ActionMode2 _actionMode2;
	int _articleNumber;
	bool _lookFlag;
	int _selectedRow;
	bool _textChanged;
	int _selectedAction;
	int _statusTextIndex;
	int _hotspotId;
	ActionSavedFields _savedFields;

	// Unknown fields
	int16 _v86F3A;
	int16 _v86F42;
	int16 _v86F4E;
	bool _v86F4A;
	int16 _v86F4C;
	InterAwaiting _interAwaiting;
	bool _inProgress;
	int _v8453A;

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

	/**
	* Execute a click within the scene
	*/
	void leftClick();
};

} // End of namespace MADS

#endif /* MADS_ACTION_H */
