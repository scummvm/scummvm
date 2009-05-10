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

void MessageState::initIndexRecordCursor() {
	_engineCursor.resource_beginning = _currentResource->data;
	_engineCursor.index_record = _indexRecords;
	_engineCursor.index = 0;
	_lastMessage.seq = 0;
}

void MessageState::parse(IndexRecordCursor *cursor, MessageTuple *t) {
	t->noun = *(cursor->index_record + 0);
	t->verb = *(cursor->index_record + 1);
	if (_version == 2101) {
		t->cond = 0;
		t->seq = 0;
	} else {
		t->cond = *(cursor->index_record + 2);
		t->seq = *(cursor->index_record + 3);
	}
}

int MessageState::getMessage(MessageTuple *t) {
	MessageTuple looking_at;

	initIndexRecordCursor();

	while (_engineCursor.index != _recordCount) {
		parse(&_engineCursor, &looking_at);
		if (t->noun == looking_at.noun && 
			t->verb == looking_at.verb && 
			t->cond == looking_at.cond && 
			t->seq == looking_at.seq)
			return 1;

		_engineCursor.index_record += ((_version == 2101) ? 4 : 11);
		_engineCursor.index++;
	}

	// FIXME: Recursion not handled yet

	return 0;
}

int MessageState::getNext() {
	if (_engineCursor.index == _recordCount)
		return 0;

	MessageTuple mesg;
	parse(&_engineCursor, &mesg);

	if (_lastMessage.seq == mesg.seq - 1)
		return 1;

	return 0;
}

int MessageState::getTalker() {
	return (_version == 2101) ? -1 : *(_engineCursor.index_record + 4);
}

void MessageState::getText(char *buffer) {
	int offset = READ_LE_UINT16(_engineCursor.index_record + ((_version == 2101) ? 2 : 5));
	char *stringptr = (char *)_engineCursor.resource_beginning + offset;
	parse(&_engineCursor, &_lastMessage);
	strcpy(buffer, stringptr);
	_engineCursor.index_record += ((_version == 2101) ? 4 : 11);
	_engineCursor.index++;
}

int MessageState::getLength() {
	int offset = READ_LE_UINT16(_engineCursor.index_record + ((_version == 2101) ? 2 : 5));
	char *stringptr = (char *)_engineCursor.resource_beginning + offset;
	return strlen(stringptr);
}

int MessageState::loadRes(int module) {
	if (_module == module)
		return 1;

	// Unlock old resource
	if (_module != -1)
		_resmgr->unlockResource(_currentResource, _module, kResourceTypeMessage);

	_module = module;
	_currentResource = _resmgr->findResource(kResourceTypeMessage, module, 1);

	if (_currentResource == NULL || _currentResource->data == NULL) {
		sciprintf("Message subsystem: Failed to load %d.MSG\n", module);
		_module = -1;
		return 0;
	}

	int offs = (_version == 2101) ? 0 : 4;
	_recordCount = READ_LE_UINT16(_currentResource->data + 4 + offs);
	_indexRecords = _currentResource->data + 6 + offs;

	initIndexRecordCursor();
	return 1;
}

void MessageState::initialize(ResourceManager *resmgr) {
	_module = -1;
	_resmgr = resmgr;
	_currentResource = NULL;
	_recordCount = 0;
	_initialized = 1;
}

void message_state_initialize(ResourceManager *resmgr, MessageState *state) {
	Resource *tester = resmgr->findResource(kResourceTypeMessage, 0, 0);

	if (tester) {
		int version = READ_LE_UINT16(tester->data);
		state->initialize(resmgr);
		state->setVersion(version);
	}
}

} // End of namespace Sci
