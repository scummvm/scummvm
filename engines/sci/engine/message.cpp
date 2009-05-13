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

	t.noun = *(_engineCursor.index_record + 0);
	t.verb = *(_engineCursor.index_record + 1);
	if (_version == 2101) {
		t.cond = 0;
		t.seq = 1;
	} else {
		t.cond = *(_engineCursor.index_record + 2);
		t.seq = *(_engineCursor.index_record + 3);
	}

	return t;
}

MessageTuple MessageState::getRefTuple() {
	MessageTuple t;

	if (_version == 2101) {
		t.noun = 0;
		t.verb = 0;
		t.cond = 0;
	} else {
		t.noun = *(_engineCursor.index_record + 7);
		t.verb = *(_engineCursor.index_record + 8);
		t.cond = *(_engineCursor.index_record + 9);
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
	_engineCursor.index_record += ((_version == 2101) ? 4 : 11);
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
	return (_version == 2101) ? -1 : *(_engineCursor.index_record + 4);
}

MessageTuple &MessageState::getLastTuple() {
	return _lastReturned;
}

int MessageState::getLastModule() {
	return _lastReturnedModule;
}

char *MessageState::getText() {
	int offset = READ_LE_UINT16(_engineCursor.index_record + ((_version == 2101) ? 2 : 5));
	return (char *)_currentResource->data + offset;
}

void MessageState::gotoNext() {
	_lastReturned = getTuple();
	_lastReturnedModule = _module;
	advanceCursor(true);
}

int MessageState::getLength() {
	int offset = READ_LE_UINT16(_engineCursor.index_record + ((_version == 2101) ? 2 : 5));
	char *stringptr = (char *)_currentResource->data + offset;
	return strlen(stringptr);
}

int MessageState::loadRes(ResourceManager *resmgr, int module, bool lock) {
	if (_locked) {
		// We already have a locked resource
		if (_module == module) {
			// If it's the same resource, we are done
			return 1;
		}

		// Otherwise, free the old resource
		resmgr->unlockResource(_currentResource, _module, kResourceTypeMessage);
		_locked = false;
	}

	_currentResource = resmgr->findResource(kResourceTypeMessage, module, lock);

	if (_currentResource == NULL || _currentResource->data == NULL) {
		warning("Message: failed to load %d.msg", module);
		return 0;
	}

	_module = module;
	_locked = lock;

	_version = READ_LE_UINT16(_currentResource->data);

	int offs = (_version == 2101) ? 0 : 4;
	_recordCount = READ_LE_UINT16(_currentResource->data + 4 + offs);
	_indexRecords = _currentResource->data + 6 + offs;

	_cursorStack.clear();
	initCursor();

	return 1;
}

} // End of namespace Sci
