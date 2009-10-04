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
 * $URL$
 * $Id$
 *
 */

#include "sci/engine/message.h"
#include "sci/tools.h"

namespace Sci {

MessageTuple MessageState::getTuple() {
	MessageTuple t;

	t.noun = _engineCursor.index_record[0];
	t.verb = _engineCursor.index_record[1];
	if (_offsetCondSeq == -1) {
		t.cond = 0;
		t.seq = 1;
	} else {
		t.cond = _engineCursor.index_record[_offsetCondSeq];
		t.seq = _engineCursor.index_record[_offsetCondSeq + 1];
	}

	return t;
}

MessageTuple MessageState::getRefTuple() {
	MessageTuple t;

	if (_offsetRef == -1) {
		t.noun = 0;
		t.verb = 0;
		t.cond = 0;
	} else {
		t.noun = _engineCursor.index_record[_offsetRef];
		t.verb = _engineCursor.index_record[_offsetRef + 1];
		t.cond = _engineCursor.index_record[_offsetRef + 2];
	}
	t.seq = 1;

	return t;
}

void MessageState::initCursor() {
	_engineCursor.index_record = _indexRecords;
	_engineCursor.index = 0;
	_engineCursor.nextSeq = 0;
}

void MessageState::advanceCursor(bool increaseSeq) {
	_engineCursor.index_record += _recordSize;
	_engineCursor.index++;

	if (increaseSeq)
		_engineCursor.nextSeq++;
}

int MessageState::findTuple(MessageTuple &t) {
	if (_module == -1)
		return 0;

	// Reset the cursor
	initCursor();
	_engineCursor.nextSeq = t.seq;

	// Do a linear search for the message
	while (1) {
		MessageTuple looking_at = getTuple();

		if (t.noun == looking_at.noun &&
			t.verb == looking_at.verb &&
			t.cond == looking_at.cond &&
			t.seq == looking_at.seq)
			break;

		advanceCursor(false);

		// Message tuple is not present
		if (_engineCursor.index == _recordCount)
			return 0;
	}

	return 1;
}

int MessageState::getMessage() {
	if (_module == -1)
		return 0;

	if (_engineCursor.index != _recordCount) {
		MessageTuple mesg = getTuple();
		MessageTuple ref = getRefTuple();

		if (_engineCursor.nextSeq == mesg.seq) {
			// We found the right sequence number, check for recursion

			if (ref.noun != 0) {
				// Recursion, advance the current cursor and load the reference
				advanceCursor(true);

				_cursorStack.push(_engineCursor);

				if (findTuple(ref))
					return getMessage();
				else {
					// Reference not found
					return 0;
				}
			} else {
				// No recursion, we are done
				return 1;
			}
		}
	}

	// We either ran out of records, or found an incorrect sequence number. Go to previous stack frame.
	if (!_cursorStack.empty()) {
		_engineCursor = _cursorStack.pop();
		return getMessage();
	}

	// Stack is empty, no message available
	return 0;
}

int MessageState::getTalker() {
	return (_offsetTalker == -1) ? -1 : _engineCursor.index_record[_offsetTalker];
}

MessageTuple &MessageState::getLastTuple() {
	return _lastReturned;
}

int MessageState::getLastModule() {
	return _lastReturnedModule;
}

Common::String MessageState::getText() {
	char *str = (char *)_currentResource->data + READ_LE_UINT16(_engineCursor.index_record + _offsetText);

	Common::String strippedStr;
	Common::String skippedSubstr;
	bool skipping = false;

	for (uint i = 0; i < strlen(str); i++) {
		if (skipping) {
			// Skip stage direction
			skippedSubstr += str[i];

			// Hopefully these locale-dependant functions are good enough
			if (islower((unsigned char)str[i]) || isdigit((unsigned char)str[i])) {
				// Lowercase or digit found, this is not a stage direction
				strippedStr += skippedSubstr;
				skipping = false;
			} else if (str[i] == ')') {
				// End of stage direction, skip trailing white space
				while ((i + 1 < strlen(str)) && isspace(str[i + 1]))
					i++;
				skipping = false;
			}
		} else {
			if (str[i] == '(') {
				// Start skipping stage direction
				skippedSubstr = str[i];
				skipping = true;
			} else if (str[i] == '\\') {
				// Escape sequence
				if ((i + 2 < strlen(str)) && isdigit(str[i + 1]) && isdigit(str[i + 2])) {
					// Hex escape sequence
					char hexStr[3];

					hexStr[0] = str[++i];
					hexStr[1] = str[++i];
					hexStr[2] = 0;

					char *endptr;
					int hexNr = strtol(hexStr, &endptr, 16);
					if (*endptr == 0)
						strippedStr += hexNr;
				} else if (i + 1 < strlen(str)) {
					// Literal escape sequence
					strippedStr += str[++i];
				}
			} else {
				strippedStr += str[i];
			}
		}
	}

	return strippedStr;
}

void MessageState::gotoNext() {
	_lastReturned = getTuple();
	_lastReturnedModule = _module;
	advanceCursor(true);
}

int MessageState::getLength() {
	int offset = READ_LE_UINT16(_engineCursor.index_record + _offsetText);
	char *stringptr = (char *)_currentResource->data + offset;
	return strlen(stringptr);
}

int MessageState::loadRes(ResourceManager *resMan, int module, bool lock) {
	_cursorStack.clear();

	if (_locked) {
		// We already have a locked resource
		if (_module == module) {
			// If it's the same resource, we are done
			return 1;
		}

		// Otherwise, free the old resource
		resMan->unlockResource(_currentResource);
		_locked = false;
	}

	_currentResource = resMan->findResource(ResourceId(kResourceTypeMessage, module), lock);

	if (_currentResource == NULL || _currentResource->data == NULL) {
		warning("Message: failed to load %d.msg", module);
		return 0;
	}

	_module = module;
	_locked = lock;

	int version = READ_LE_UINT16(_currentResource->data);
	debug(5, "Message: reading resource %d.msg, version %d.%03d", _module, version / 1000, version % 1000);

	int offsetCount;

	// FIXME: Correct/extend this data by examining more games
	if (version < 3000) {
		_offsetCondSeq = -1;
		_offsetTalker = -1;
		_offsetRef = -1;
		_offsetText = 2;
		_recordSize = 4;
		offsetCount = 4;
	} else if (version < 4000) {
		_offsetCondSeq = 2;
		_offsetTalker = 4;
		_offsetRef = -1;
		_offsetText = 5;
		_recordSize = 10;
		offsetCount = 6;
	} else {
		_offsetCondSeq = 2;
		_offsetTalker = 4;
		_offsetRef = 7;
		_offsetText = 5;
		_recordSize = 11;
		offsetCount = 8;
	}

	_recordCount = READ_LE_UINT16(_currentResource->data + offsetCount);
	_indexRecords = _currentResource->data + offsetCount + 2;

	initCursor();

	return 1;
}

} // End of namespace Sci
