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

#ifndef SCI_ENGINE_MESSAGE_H
#define SCI_ENGINE_MESSAGE_H

#include "sci/scicore/resource.h"

namespace Sci {

struct MessageTuple {
	int noun;
	int verb;
	int cond;
	int seq;
};

struct IndexRecordCursor {
	byte *index_record;
	int index;
	byte *resource_beginning;
};

class MessageState {
public:
	int getMessage(MessageTuple *t);
	int getNext();
	int getTalker();
	int getLength();
	void getText(char *buffer);
	int loadRes(int module);
	int isInitialized() { return _initialized; }
	void initialize(ResourceManager *resmgr);
	void setVersion(int version) { _version = version; }

private:
	void initIndexRecordCursor();
	void parse(IndexRecordCursor *cursor, MessageTuple *t);

	int _initialized;
	ResourceManager *_resmgr;
	Resource *_currentResource;
	int _module;
	int _recordCount;
	byte *_indexRecords;
	IndexRecordCursor _engineCursor;
	MessageTuple _lastMessage;
	int _version;
};

void message_state_initialize(ResourceManager *resmgr, MessageState *state);

} // End of namespace Sci

#endif // SCI_ENGINE_MESSAGE_H
