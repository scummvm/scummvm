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
#include "sci/engine/kernel.h"
#include "sci/engine/seg_manager.h"

namespace Sci {

struct MessageRecord {
	MessageTuple tuple;
	MessageTuple refTuple;
	const char *string;
	byte talker;
};

class MessageReader {
public:
	bool init() {
		if (_headerSize > _size)
			return false;

		// Read message count from last word in header
		_messageCount = READ_LE_UINT16(_data + _headerSize - 2);

		if (_messageCount * _recordSize + _headerSize > _size)
			return false;

		return true;
	}

	virtual bool findRecord(const MessageTuple &tuple, MessageRecord &record) = 0;

	virtual ~MessageReader() { };

protected:
	MessageReader(const byte *data, uint size, uint headerSize, uint recordSize)
		: _data(data), _size(size), _headerSize(headerSize), _recordSize(recordSize) { }

	const byte *_data;
	const uint _size;
	const uint _headerSize;
	const uint _recordSize;
	uint _messageCount;
};

class MessageReaderV2 : public MessageReader {
public:
	MessageReaderV2(byte *data, uint size) : MessageReader(data, size, 6, 4) { }

	bool findRecord(const MessageTuple &tuple, MessageRecord &record) {
		const byte *recordPtr = _data + _headerSize;

		for (uint i = 0; i < _messageCount; i++) {
			if ((recordPtr[0] == tuple.noun) && (recordPtr[1] == tuple.verb)) {
				record.tuple = tuple;
				record.refTuple = MessageTuple();
				record.talker = 0;
				record.string = (const char *)_data + READ_LE_UINT16(recordPtr + 2);
				return true;
			}
			recordPtr += _recordSize;
		}

		return false;
	}
};

class MessageReaderV3 : public MessageReader {
public:
	MessageReaderV3(byte *data, uint size) : MessageReader(data, size, 8, 10) { }

	bool findRecord(const MessageTuple &tuple, MessageRecord &record) {
		const byte *recordPtr = _data + _headerSize;

		for (uint i = 0; i < _messageCount; i++) {
			if ((recordPtr[0] == tuple.noun) && (recordPtr[1] == tuple.verb)
				&& (recordPtr[2] == tuple.cond) && (recordPtr[3] == tuple.seq)) {
				record.tuple = tuple;
				record.refTuple = MessageTuple();
				record.talker = recordPtr[4];
				record.string = (const char *)_data + READ_LE_UINT16(recordPtr + 5);
				return true;
			}
			recordPtr += _recordSize;
		}

		return false;
	}
};

class MessageReaderV4 : public MessageReader {
public:
	MessageReaderV4(byte *data, uint size) : MessageReader(data, size, 10, 11) { }

	bool findRecord(const MessageTuple &tuple, MessageRecord &record) {
		const byte *recordPtr = _data + _headerSize;

		for (uint i = 0; i < _messageCount; i++) {
			if ((recordPtr[0] == tuple.noun) && (recordPtr[1] == tuple.verb)
				&& (recordPtr[2] == tuple.cond) && (recordPtr[3] == tuple.seq)) {
				record.tuple = tuple;
				record.refTuple = MessageTuple(recordPtr[7], recordPtr[8], recordPtr[9]);
				record.talker = recordPtr[4];
				record.string = (const char *)_data + READ_LE_UINT16(recordPtr + 5);
				return true;
			}
			recordPtr += _recordSize;
		}

		return false;
	}
};

bool MessageState::getRecord(CursorStack &stack, bool recurse, MessageRecord &record) {
	Resource *res = ((SciEngine *)g_engine)->getResourceManager()->findResource(ResourceId(kResourceTypeMessage, stack.getModule()), 0);

	if (!res) {
		warning("Failed to open message resource %d", stack.getModule());
		return false;
	}

	MessageReader *reader;
	int version = READ_LE_UINT16(res->data) / 1000;

	switch (version) {
	case 2:
		reader = new MessageReaderV2(res->data, res->size);
		break;
	case 3:
		reader = new MessageReaderV3(res->data, res->size);
		break;
	case 4:
		reader = new MessageReaderV4(res->data, res->size);
		break;
	default:
		warning("Message: unsupported resource version");
		return false;
	}

	if (!reader->init()) {
		warning("Message: failed to read resource header");
		return false;
	}

	while (1) {
		MessageTuple &t = stack.top();

		if (!reader->findRecord(t, record)) {
			// Tuple not found
			if (recurse && (stack.size() > 1)) {
				stack.pop();
				continue;
			}

			return false;
		}

		if (recurse) {
			MessageTuple &ref = record.refTuple;

			if (ref.noun || ref.verb || ref.cond) {
				t.seq++;
				stack.push(ref);
				continue;
			}
		}

		return true;
	}
}

int MessageState::getMessage(int module, MessageTuple &t, reg_t buf) {
	_cursorStack.init(module, t);
	return nextMessage(buf);
}

int MessageState::nextMessage(reg_t buf) {
	MessageRecord record;

	if (!buf.isNull()) {
		if (getRecord(_cursorStack, true, record)) {
			outputString(buf, processString(record.string));
			_lastReturned = record.tuple;
			_lastReturnedModule = _cursorStack.getModule();
			_cursorStack.top().seq++;
			return record.talker;
		} else {
			MessageTuple &t = _cursorStack.top();
			outputString(buf, Common::String::printf("Msg %d: %d %d %d %d not found", _cursorStack.getModule(), t.noun, t.verb, t.cond, t.seq));
			return 0;
		}
	} else {
		CursorStack stack = _cursorStack;

		if (getRecord(stack, true, record))
			return record.talker;
		else
			return 0;
	}
}

int MessageState::messageSize(int module, MessageTuple &t) {
	CursorStack stack;
	MessageRecord record;

	stack.init(module, t);
	if (getRecord(stack, true, record))
		return strlen(record.string) + 1;
	else
		return 0;
}

bool MessageState::messageRef(int module, const MessageTuple &t, MessageTuple &ref) {
	CursorStack stack;
	MessageRecord record;

	stack.init(module, t);
	if (getRecord(stack, false, record)) {
		ref = record.refTuple;
		return true;
	}

	return false;
}

void MessageState::pushCursorStack() {
	_cursorStackStack.push(_cursorStack);
}

void MessageState::popCursorStack() {
	if (!_cursorStackStack.empty())
		_cursorStack = _cursorStackStack.pop();
	else
		warning("Message: attempt to pop from empty stack");
}

int MessageState::hexDigitToInt(char h) {
	if ((h >= 'A') && (h <= 'F'))
		return h - 'A' + 10;

	if ((h >= 'a') && (h <= 'f'))
		return h - 'a' + 10;

	if ((h >= '0') && (h <= '9'))
		return h - '0';

	return -1;
}

bool MessageState::stringHex(Common::String &outStr, const Common::String &inStr, uint &index) {
	// Hex escape sequences of the form \nn, where n is a hex digit
	if (inStr[index] != '\\')
		return false;

	// Check for enough room for a hex escape sequence
	if (index + 2 >= inStr.size())
		return false;

	int digit1 = hexDigitToInt(inStr[index + 1]);
	int digit2 = hexDigitToInt(inStr[index + 2]);

	// Check for hex
	if ((digit1 == -1) || (digit2 == -1))
		return false;

	outStr += digit1 * 16 + digit2;
	index += 3;

	return true;
}

bool MessageState::stringLit(Common::String &outStr, const Common::String &inStr, uint &index) {
	// Literal escape sequences of the form \n
	if (inStr[index] != '\\')
		return false;

	// Check for enough room for a literal escape sequence
	if (index + 1 >= inStr.size())
		return false;

	outStr += inStr[index + 1];
	index += 2;

	return true;
}

bool MessageState::stringStage(Common::String &outstr, const Common::String &inStr, uint &index) {
	// Stage directions of the form (n*), where n is anything but a digit or a lowercase character
	if (inStr[index] != '(')
		return false;

	for (uint i = index + 1; i < inStr.size(); i++) {
		if (inStr[i] == ')') {
			// Stage direction found, skip it
			index = i + 1;

			// Skip trailing white space
			while ((index < inStr.size()) && ((inStr[index] == '\n') || (inStr[index] == '\r') || (inStr[index] == ' ')))
				index++;

			return true;
		}

		// If we find a lowercase character or a digit, it's not a stage direction
		if (((inStr[i] >= 'a') && (inStr[i] <= 'z')) || ((inStr[i] >= '0') && (inStr[i] <= '9')))
			return false;
	}

	// We ran into the end of the string without finding a closing bracket
	return false;
}

Common::String MessageState::processString(const char *s) {
	Common::String outStr;
	Common::String inStr = Common::String(s);

	uint index = 0;

	while (index < inStr.size()) {
		// Check for hex escape sequence
		if (stringHex(outStr, inStr, index))
			continue;

		// Check for literal escape sequence
		if (stringLit(outStr, inStr, index))
			continue;

		// Check for stage direction
		if (stringStage(outStr, inStr, index))
			continue;

		// None of the above, copy char
		outStr += inStr[index++];
	}

	return outStr;
}

void MessageState::outputString(reg_t buf, const Common::String &str) {
	SegmentRef buffer_r = _segMan->dereference(buf);

	if ((unsigned)buffer_r.maxSize >= str.size() + 1) {
		_segMan->strcpy(buf, str.c_str());
	} else {
		warning("Message: buffer %04x:%04x invalid or too small to hold the following text of %i bytes: '%s'", PRINT_REG(buf), str.size() + 1, str.c_str());

		// Set buffer to empty string if possible
		if (buffer_r.maxSize > 0)
			_segMan->strcpy(buf, "");
	}
}

void MessageState::lastQuery(int &module, MessageTuple &tuple) {
	module = _lastReturnedModule;
	tuple = _lastReturned;
}

} // End of namespace Sci
