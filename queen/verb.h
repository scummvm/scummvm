
/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef QUEENVERB_H
#define QUEENVERB_H

namespace Queen {

enum VerbEnum {
	VERB_NONE = 0,

	VERB_PANEL_COMMAND_FIRST = 1,
	VERB_OPEN        = 1,
	VERB_CLOSE       = 2,
	VERB_MOVE        = 3,
	// no verb 4
	VERB_GIVE        = 5,
	VERB_USE         = 6,
	VERB_PICK_UP     = 7,
	VERB_LOOK_AT     = 9,
	VERB_TALK_TO     = 8,
	VERB_PANEL_COMMAND_LAST = 9,

	VERB_WALK_TO     = 10,
	VERB_SCROLL_UP   = 11,
	VERB_SCROLL_DOWN = 12,

	VERB_DIGIT_FIRST = 13,
	VERB_DIGIT_1   = 13,
	VERB_DIGIT_2   = 14,
	VERB_DIGIT_3   = 15,
	VERB_DIGIT_4   = 16,
	VERB_DIGIT_LAST = 16,
	
	VERB_INV_FIRST = VERB_DIGIT_FIRST,
	VERB_INV_1 = VERB_DIGIT_1,
	VERB_INV_2 = VERB_DIGIT_2,
	VERB_INV_3 = VERB_DIGIT_3,
	VERB_INV_4 = VERB_DIGIT_4,
	VERB_INV_LAST = VERB_DIGIT_LAST,

	VERB_USE_JOURNAL = 20,
	VERB_SKIP_TEXT   = 101,

	VERB_PREP_WITH = 11,
	VERB_PREP_TO   = 12
};


class Verb {
public:

	Verb() {
		_verb = VERB_NONE;
	}

	Verb(VerbEnum v) {
		_verb = v;
	}

	//! _verb is open/close/move/give/look at/pick up/talk to
	bool isPanelCommand() const {
		return 
			_verb >= VERB_PANEL_COMMAND_FIRST &&
			_verb <= VERB_PANEL_COMMAND_LAST;
	}

	bool isScrollInventory() const {
		return 
			_verb == VERB_SCROLL_UP || 
			_verb == VERB_SCROLL_DOWN;
	}

	bool isInventory() const {
		return 
			_verb >= VERB_INV_FIRST && 
			_verb <= VERB_INV_LAST;
	}

	bool isJournal() const {
		return _verb == VERB_USE_JOURNAL;
	}

	bool isDigit() const {
		return
			_verb >= VERB_DIGIT_FIRST && 
			_verb <= VERB_DIGIT_LAST;
	}

	int digit() const {
		return (int)_verb - VERB_DIGIT_1 + 1;
	}

	bool isSkipText() const {
		return _verb == VERB_SKIP_TEXT;
	}

	bool isAction() const {
		return 
			isPanelCommand() || 
			_verb == VERB_WALK_TO || 
			isScrollInventory();
	}

	bool isNone() const {
		return _verb == VERB_NONE;
	}

	int inventoryItem() const {
		if (isInventory()) {
			return _verb - VERB_INV_FIRST;
		}
		return -1;
	}

	VerbEnum value() const {
		return _verb;
	}

	const char* name() const {
		if (_verb > 0 && _verb < 13) {
			return _verbName[_verb];
		}
		return NULL;
	}

	bool operator==(const Verb& other) const {
		return _verb == other._verb;
	}

	bool operator!=(const Verb& other) const {
		return _verb != other._verb;
	}

	static void initName(int i, char* name) {
		_verbName[i] = name;
	}

private:

	VerbEnum _verb;

	static char* _verbName[13];
};

} // End of namespace Queen

#endif
