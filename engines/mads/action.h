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

enum { ACTIONMODE_NONE = 0, ACTIONMODE_VERB = 1, ACTIONMODE_OBJECT = 3, ACTIONMODE_TALK = 6 };
enum { ACTIONMODE2_0 = 0, ACTIONMODE2_2 = 2, ACTIONMODE2_4 = 4, ACTIONMODE2_5 = 5 };

enum TriggerMode { 
	KERNEL_TRIGGER_PARSER = 0,		// Triggers parser
	KERNEL_TRIGGER_DAEMON = 1,		// Triggers step/daemon code
	KERNEL_TRIGGER_PREPARE = 2		// Triggers preparser
};

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
	bool _commandError;
	int _commandSource;
	int _command;
	int _mainObject;
	int _secondObject;
	int _mainObjectSource;
	int _secondObjectSource;
	int _articleNumber;
	int _lookFlag;
};

class MADSAction {
private:
	MADSEngine *_vm;
	Common::String _statusText;

	void appendVocab(int vocabId, bool capitalise = false);

	void startWalkingDirectly(int walkType);
public:
	ActionDetails _action, _activeAction;
	int8 _flags1, _flags2;
	int _commandSource;
	int _mainObjectSource;
	int _articleNumber;
	bool _lookFlag;
	int _selectedRow;
	bool _textChanged;
	int _selectedAction;
	int _statusTextIndex;
	int _hotspotId;
	ActionSavedFields _savedFields;
	Common::String _sentence;

	// Unknown fields
	int16 _secondObject;
	int16 _secondObjectSource;
	int16 _recentCommandSource;
	bool _pointEstablished;
	int16 _recentCommand;
	InterAwaiting _interAwaiting;
	bool _inProgress;
	int _pickedWord;

public:
	MADSAction(MADSEngine *vm);

	void clear();
	void set();
	const Common::String &statusText() const { return _statusText; }
	void refresh();

	/**
	 * Accepts the currently defined sentence from the ScreenObjects parser.
	 * Copies the data, and checks to see if the action requires the player
	 * to walk to the given hotspot.
	 */
	void startAction();

	void checkAction();
	bool isAction(int verbId, int objectNameId = 0, int indirectObjectId = 0);
	
	/**
	 * Check the result of the current action on the sentence
	 * with the provision that the action is not yet complete.
	 */
	void checkActionAtMousePos();

	/**
	* Execute a click within the scene
	*/
	void leftClick();
};

} // End of namespace MADS

#endif /* MADS_ACTION_H */
