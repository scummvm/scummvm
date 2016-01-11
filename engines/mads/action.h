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

#ifndef MADS_ACTION_H
#define MADS_ACTION_H

#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/str.h"

namespace MADS {

enum TriggerMode {
	SEQUENCE_TRIGGER_NONE = -1,
	SEQUENCE_TRIGGER_PARSER = 0,		// Triggers parser
	SEQUENCE_TRIGGER_DAEMON = 1,		// Triggers step/daemon code
	SEQUENCE_TRIGGER_PREPARE = 2		// Triggers preparser
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

enum VerbType { VERB_ONLY, VERB_THIS, VERB_THAT, VERB_INIT };

enum PrepType {
	PREP_NONE, PREP_WITH, PREP_TO, PREP_AT, PREP_FROM, PREP_ON, PREP_IN,
	PREP_UNDER, PREP_BEHIND, PREP_RELATIONAL = 0xff
};

enum ScrCategory {
	CAT_NONE = 0, CAT_COMMAND = 1, CAT_INV_LIST = 2, CAT_INV_VOCAB = 3,
	CAT_HOTSPOT = 4, CAT_INV_ANIM = 5, CAT_TALK_ENTRY = 6, CAT_INV_SCROLLER = 7,
	CAT_12 = 12
};

class MADSEngine;

struct ActionDetails {
	int _verbId;
	int _objectNameId;
	int _indirectObjectId;

	/**
	 * Synchronize the action details
	 */
	void synchronize(Common::Serializer &s);
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

	/**
	* Synchronize the saved action details
	*/
	void synchronize(Common::Serializer &s);
};

class MADSAction {
private:
	MADSEngine *_vm;
	Common::String _statusText;

	void appendVocab(int vocabId, bool capitalize = false);

	void startWalkingDirectly(int walkType);
public:
	ActionDetails _action, _activeAction;
	int _articleNumber;
	bool _lookFlag;
	int _selectedRow;
	bool _textChanged;
	int _selectedAction;
	int _statusTextIndex;
	int _hotspotId;
	ActionSavedFields _savedFields;
	Common::String _sentence;

	VerbType _verbType;
	PrepType _prepType;
	ScrCategory _commandSource;
	ScrCategory _mainObjectSource;
	int _secondObject;
	ScrCategory _secondObjectSource;
	ScrCategory _recentCommandSource;
	bool _pointEstablished;
	int  _recentCommand;
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
	bool isObject(int objectNameId);
	bool isTarget(int objectNameId);

	/**
	 * Check the result of the current action on the sentence
	 * with the provision that the action is not yet complete.
	 */
	void checkActionAtMousePos();

	/**
	* Execute a click within the scene
	*/
	void leftClick();

	/**
	* Synchronize the saved action details
	*/
	void synchronize(Common::Serializer &s);
};

} // End of namespace MADS

#endif /* MADS_ACTION_H */
