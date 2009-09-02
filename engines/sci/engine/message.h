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

#include "sci/resource.h"
#include "common/stack.h"

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
	int nextSeq;
};

typedef Common::Stack<IndexRecordCursor> CursorStack;

// FIXME: Documentation
class MessageState {
public:
	MessageState() : _module(-1), _locked(false) { }
	int findTuple(MessageTuple &t);
	MessageTuple getTuple();
	MessageTuple getRefTuple();
	int getMessage();
	void gotoNext();
	Common::String getText();
	int getTalker();
	int getLength();
	MessageTuple &getLastTuple();
	int getLastModule();
	int loadRes(ResourceManager *resMan, int module, bool lock);

private:
	void initCursor();
	void advanceCursor(bool increaseSeq);

	Resource *_currentResource;
	int _module;
	bool _locked;
	int _recordCount;
	byte *_indexRecords;
	CursorStack _cursorStack;
	IndexRecordCursor _engineCursor;
	MessageTuple _lastReturned;
	int _lastReturnedModule;
	int _offsetCondSeq;
	int _offsetRef;
	int _offsetTalker;
	int _offsetText;
	int _recordSize;
};

} // End of namespace Sci

#endif // SCI_ENGINE_MESSAGE_H
