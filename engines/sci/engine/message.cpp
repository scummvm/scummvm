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


#include "sci/engine/message.h"
#include "sci/engine/kernel.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/state.h"
#include "sci/engine/workarounds.h"
#include "sci/util.h"

namespace Sci {

struct MessageRecord {
	MessageTuple tuple;
	MessageTuple refTuple;
	const char *string;
	uint32 length;
	byte talker;
};

class MessageReader {
public:
	bool init() {
		if (_headerSize > _data.size())
			return false;

		// Read message count from last word in header
		_messageCount = _data.getUint16SEAt(_headerSize - 2);

		if (_messageCount * _recordSize + _headerSize > _data.size())
			return false;

		return true;
	}

	virtual bool findRecord(const MessageTuple &tuple, MessageRecord &record) = 0;

	virtual ~MessageReader() { }

protected:
	MessageReader(const SciSpan<const byte> &data, uint headerSize, uint recordSize)
		: _data(data), _headerSize(headerSize), _recordSize(recordSize), _messageCount(0) { }

	const SciSpan<const byte> _data;
	const uint _headerSize;
	const uint _recordSize;
	uint _messageCount;
};

class MessageReaderV2 : public MessageReader {
public:
	MessageReaderV2(const SciSpan<const byte> &data) : MessageReader(data, 6, 4) { }

	bool findRecord(const MessageTuple &tuple, MessageRecord &record) override {
		SciSpan<const byte> recordPtr = _data.subspan(_headerSize);

		for (uint i = 0; i < _messageCount; i++) {
			if ((recordPtr[0] == tuple.noun) && (recordPtr[1] == tuple.verb)) {
				record.tuple = tuple;
				record.refTuple = MessageTuple();
				record.talker = 0;
				const uint16 stringOffset = recordPtr.getUint16LEAt(2);
				const uint32 maxSize = _data.size() - stringOffset;
				record.string = (const char *)_data.getUnsafeDataAt(stringOffset, maxSize);
				record.length = Common::strnlen(record.string, maxSize);
				if (record.length == maxSize) {
					warning("Message %s from %s appears truncated at %d", tuple.toString().c_str(), _data.name().c_str(), recordPtr - _data);
				}
				return true;
			}
			recordPtr += _recordSize;
		}

		return false;
	}
};

class MessageReaderV3 : public MessageReader {
public:
	MessageReaderV3(const SciSpan<const byte> &data) : MessageReader(data, 8, 10) { }

	bool findRecord(const MessageTuple &tuple, MessageRecord &record) override {
		SciSpan<const byte> recordPtr = _data.subspan(_headerSize);
		for (uint i = 0; i < _messageCount; i++) {
			if ((recordPtr[0] == tuple.noun) && (recordPtr[1] == tuple.verb)
				&& (recordPtr[2] == tuple.cond) && (recordPtr[3] == tuple.seq)) {
				record.tuple = tuple;
				record.refTuple = MessageTuple();
				record.talker = recordPtr[4];
				const uint16 stringOffset = recordPtr.getUint16LEAt(5);
				const uint32 maxSize = _data.size() - stringOffset;
				record.string = (const char *)_data.getUnsafeDataAt(stringOffset, maxSize);
				record.length = Common::strnlen(record.string, maxSize);
				if (record.length == maxSize) {
					warning("Message %s from %s appears truncated at %d", tuple.toString().c_str(), _data.name().c_str(), recordPtr - _data);
				}
				return true;
			}
			recordPtr += _recordSize;
		}

		return false;
	}
};

class MessageReaderV4 : public MessageReader {
public:
	MessageReaderV4(const SciSpan<const byte> &data) : MessageReader(data, 10, 11) { }

	bool findRecord(const MessageTuple &tuple, MessageRecord &record) override {
		SciSpan<const byte> recordPtr = _data.subspan(_headerSize);
		for (uint i = 0; i < _messageCount; i++) {
			if ((recordPtr[0] == tuple.noun) && (recordPtr[1] == tuple.verb)
				&& (recordPtr[2] == tuple.cond) && (recordPtr[3] == tuple.seq)) {
				record.tuple = tuple;
				record.refTuple = MessageTuple(recordPtr[7], recordPtr[8], recordPtr[9]);
				record.talker = recordPtr[4];
				const uint16 stringOffset = recordPtr.getUint16SEAt(5);
				const uint32 maxSize = _data.size() - stringOffset;
				record.string = (const char *)_data.getUnsafeDataAt(stringOffset, maxSize);
				record.length = Common::strnlen(record.string, maxSize);
				if (record.length == maxSize) {
					warning("Message %s from %s appears truncated at %d", tuple.toString().c_str(), _data.name().c_str(), recordPtr - _data);
				}
				return true;
			}
			recordPtr += _recordSize;
		}

		return false;
	}
};

#ifdef ENABLE_SCI32
// SCI32 Mac decided to add an extra byte (currently unknown in meaning) between
// the talker and the string...
class MessageReaderV4_MacSCI32 : public MessageReader {
public:
	MessageReaderV4_MacSCI32(const SciSpan<const byte> &data) : MessageReader(data, 10, 12) { }

	bool findRecord(const MessageTuple &tuple, MessageRecord &record) override {
		SciSpan<const byte> recordPtr = _data.subspan(_headerSize);
		for (uint i = 0; i < _messageCount; i++) {
			if ((recordPtr[0] == tuple.noun) && (recordPtr[1] == tuple.verb)
				&& (recordPtr[2] == tuple.cond) && (recordPtr[3] == tuple.seq)) {
				record.tuple = tuple;
				record.refTuple = MessageTuple(recordPtr[8], recordPtr[9], recordPtr[10]);
				record.talker = recordPtr[4];
				const uint16 stringOffset = recordPtr.getUint16BEAt(6);
				const uint32 maxSize = _data.size() - stringOffset;
				record.string = (const char *)_data.getUnsafeDataAt(stringOffset, maxSize);
				record.length = Common::strnlen(record.string, maxSize);
				if (record.length == maxSize) {
					warning("Message %s from %s appears truncated at %d", tuple.toString().c_str(), _data.name().c_str(), recordPtr - _data);
				}
				return true;
			}
			recordPtr += _recordSize;
		}

		return false;
	}
};
#endif

bool MessageState::getRecord(CursorStack &stack, bool recurse, MessageRecord &record) {
	// find a workaround for the requested message and use the prescribed module
	int module = stack.getModule();
	MessageTuple &tuple = stack.top();
	SciMessageWorkaroundSolution workaround = findMessageWorkaround(module, tuple.noun, tuple.verb, tuple.cond, tuple.seq);
	if (workaround.type != MSG_WORKAROUND_NONE) {
		module = workaround.module;
	}
	Resource *res = g_sci->getResMan()->findResource(ResourceId(kResourceTypeMessage, module), false);

	if (!res) {
		warning("Failed to open message resource %d", module);
		return false;
	}

	MessageReader *reader;
	int version = res->getUint32SEAt(0) / 1000;

	switch (version) {
	case 2:
		reader = new MessageReaderV2(*res);
		break;
	case 3:
		reader = new MessageReaderV3(*res);
		break;
	case 4:
#ifdef ENABLE_SCI32
	case 5: // v5 seems to be compatible with v4
		// SCI32 Mac is different than SCI32 DOS/Win here
		if (g_sci->getPlatform() == Common::kPlatformMacintosh && getSciVersion() >= SCI_VERSION_2_1_EARLY)
			reader = new MessageReaderV4_MacSCI32(*res);
		else
#endif
			reader = new MessageReaderV4(*res);
		break;
	default:
		error("Message: unsupported resource version %d", version);
		return false;
	}

	if (!reader->init()) {
		delete reader;

		warning("Message: failed to read resource header");
		return false;
	}

	// apply the message workaround
	if (workaround.type == MSG_WORKAROUND_REMAP) {
		// remap the request to a different message record.
		//  this alters the stack, nextMessage() will return the next
		//  record in the sequence following the returned record.
		stack.setModule(module);
		tuple.noun = workaround.noun;
		tuple.verb = workaround.verb;
		tuple.cond = workaround.cond;
		tuple.seq = workaround.seq;
	} else if (workaround.type == MSG_WORKAROUND_FAKE) {
		// return a fake message record hard-coded in the workaround.
		//  this leaves the stack unchanged.
		record.tuple = tuple;
		record.refTuple = MessageTuple();
		record.string = workaround.text;
		record.length = strlen(workaround.text);
		record.talker = workaround.talker;
		delete reader;
		return true;
	} else if (workaround.type == MSG_WORKAROUND_EXTRACT) {
		// extract and return text from a different message record.
		//  use the talker provided by the workaround since the correct value
		//  could be in either, or neither, of the records.
		//  this leaves the stack unchanged.
		MessageTuple textTuple(workaround.noun, workaround.verb, workaround.cond, workaround.seq);
		MessageRecord textRecord;
		if (reader->findRecord(textTuple, textRecord)) {
			uint32 textLength = (workaround.substringLength == 0) ? textRecord.length : workaround.substringLength;
			if (workaround.substringIndex + textLength <= textRecord.length) {
				record.tuple = tuple;
				record.refTuple = MessageTuple();
				record.string = textRecord.string + workaround.substringIndex;
				record.length = textLength;
				record.talker = workaround.talker;
				delete reader;
				return true;
			}
		}
	}

	while (1) {
		MessageTuple &t = stack.top();

		if (!reader->findRecord(t, record)) {
			// Tuple not found
			if (recurse && (stack.size() > 1)) {
				stack.pop();
				continue;
			}

			delete reader;
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

		delete reader;
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
			outputString(buf, processString(record.string, record.length));
			_lastReturned = record.tuple;
			_lastReturnedModule = _cursorStack.getModule();
			_cursorStack.top().seq++;
			return record.talker;
		} else {
			MessageTuple &t = _cursorStack.top();
			outputString(buf, Common::String::format("Msg %d: %s not found", _cursorStack.getModule(), t.toString().c_str()));
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
		return record.length + 1;
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
		error("Message: attempt to pop from empty stack");
}

int MessageState::hexDigitToWrongInt(char h) {
	// Hex digits above 9 are incorrectly interpreted by SSCI as 11-16 instead
	// of 10-15 because of a never-fixed typo
	if ((h >= 'A') && (h <= 'F'))
		return h - 'A' + 11;

	if ((h >= 'a') && (h <= 'f'))
		return h - 'a' + 11;

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

	int digit1 = hexDigitToWrongInt(inStr[index + 1]);
	int digit2 = hexDigitToWrongInt(inStr[index + 2]);

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
	// Stage directions of the form (n *), where n is anything but a digit or a lowercase character
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

		// For Russian we allow all upper characters
		if (g_sci->getLanguage() == Common::RU_RUS) {
			if (((byte)inStr[i] >= 'a') || ((inStr[i] >= '0') && (inStr[i] <= '9') && (getSciVersion() < SCI_VERSION_2)))
				return false;
		}

		// If we find a lowercase character or a digit, it's not a stage direction
		// SCI32 seems to support having digits in stage directions
		if (((inStr[i] >= 'a') && (inStr[i] <= 'z')) || ((inStr[i] >= '0') && (inStr[i] <= '9') && (getSciVersion() < SCI_VERSION_2)))
			return false;
	}

	// We ran into the end of the string without finding a closing bracket
	return false;
}

Common::String MessageState::processString(const char *s, uint32 maxLength) {
	Common::String outStr;
	Common::String inStr = Common::String(s);

	uint index = 0;

	while (index < inStr.size() && index < maxLength) {
		// Check for hex escape sequence.
		//  SQ4CD predates this interpreter feature but has a message on the
		//  hintbook screen which appears to contain hex strings and renders
		//  incorrectly if converted, so exclude it. Fixes #11070
		if (g_sci->getGameId() != GID_SQ4) {
			if (stringHex(outStr, inStr, index))
				continue;
		}

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
#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		SciArray *sciString = _segMan->lookupArray(buf);
		sciString->fromString(str);
	} else {
#endif
		SegmentRef buffer_r = _segMan->dereference(buf);

		if ((unsigned)buffer_r.maxSize >= str.size() + 1) {
			_segMan->strcpy(buf, str.c_str());
		} else {
			// LSL6 sets an exit text here, but the buffer size allocated
			// is too small. Don't display a warning in this case, as we
			// don't use the exit text anyway - bug report #3035533
			if (g_sci->getGameId() == GID_LSL6 && str.hasPrefix("\r\n(c) 1993 Sierra On-Line, Inc")) {
				// LSL6 buggy exit text, don't show warning
			} else {
				warning("Message: buffer %04x:%04x invalid or too small to hold the following text of %i bytes: '%s'", PRINT_REG(buf), str.size() + 1, str.c_str());
			}

			// Set buffer to empty string if possible
			if (buffer_r.maxSize > 0)
				_segMan->strcpy(buf, "");
		}
#ifdef ENABLE_SCI32
	}
#endif
}

void MessageState::lastQuery(int &module, MessageTuple &tuple) {
	module = _lastReturnedModule;
	tuple = _lastReturned;
}

} // End of namespace Sci
