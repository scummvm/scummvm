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

	bool findRecord(const MessageTuple &tuple, MessageRecord &record) {
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

	bool findRecord(const MessageTuple &tuple, MessageRecord &record) {
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

	bool findRecord(const MessageTuple &tuple, MessageRecord &record) {
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

#ifdef ENABLE_SCI32_MAC
// SCI32 Mac decided to add an extra byte (currently unknown in meaning) between
// the talker and the string...
class MessageReaderV4_MacSCI32 : public MessageReader {
public:
	MessageReaderV4_MacSCI32(const SciSpan<const byte> &data) : MessageReader(data, 10, 12) { }

	bool findRecord(const MessageTuple &tuple, MessageRecord &record) {
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
	Resource *res = g_sci->getResMan()->findResource(ResourceId(kResourceTypeMessage, stack.getModule()), false);

	if (!res) {
		warning("Failed to open message resource %d", stack.getModule());
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
#endif
#ifdef ENABLE_SCI32_MAC
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

	while (1) {
		MessageTuple &t = stack.top();

		// Fix known incorrect message tuples
		// TODO: Add a more generic mechanism, like the one we have for
		// script workarounds, for cases with incorrect sync resources,
		// like the ones below.
		if (g_sci->getGameId() == GID_QFG1VGA && stack.getModule() == 322 &&
			t.noun == 14 && t.verb == 1 && t.cond == 19 && t.seq == 1) {
			// Talking to Kaspar the shopkeeper - bug #3604944
			t.verb = 2;
		}

		if (g_sci->getGameId() == GID_PQ1 && stack.getModule() == 38 &&
			t.noun == 10 && t.verb == 4 && t.cond == 8 && t.seq == 1) {
			// Using the hand icon on Keith in the Blue Room - bug #3605654
			t.cond = 9;
		}

		if (g_sci->getGameId() == GID_PQ1 && stack.getModule() == 38 &&
			t.noun == 10 && t.verb == 1 && t.cond == 0 && t.seq == 1) {
			// Using the eye icon on Keith in the Blue Room - bug #3605654
			t.cond = 13;
		}

		if (g_sci->getGameId() == GID_QFG4 && stack.getModule() == 16 &&
			t.noun == 49 && t.verb == 1 && t.cond == 0 && t.seq == 2) {
			// Examining the statue inventory item from the monastery - bug #10770
			// The description says "squid-like monster", yet the icon is
			// clearly an insect. It turned Chief into "an enormous beetle". We
			// change the phrase to "monstrous insect".
			//
			// Note: The German string contains accented characters.
			//  0x84 "a with diaeresis"
			//  0x94 "o with diaeresis"
			//
			// Values were pulled from SCI Companion's raw message data. They
			// are escaped that way here, as encoded bytes.
			record.tuple = t;
			record.refTuple = MessageTuple();
			record.talker = 99;
			if (g_sci->getSciLanguage() == K_LANG_GERMAN) {
				record.string = "Die groteske Skulptur eines schrecklichen, monstr\x94sen insekts ist sorgf\x84ltig in die Einkaufstasche eingewickelt.";
				record.length = 112;
			} else {
				record.string = "Carefully wrapped in a shopping bag is the grotesque sculpture of a horrible, monstrous insect.";
				record.length = 95;
			}
			delete reader;
			return true;
		}

		if (g_sci->getGameId() == GID_QFG4 && stack.getModule() == 579 &&
			t.noun == 0 && t.verb == 0 && t.cond == 0 && t.seq == 1) {
			// Talking with the Leshy and telling him about "bush in goo" - bug #10137
			t.verb = 1;
		}

		if (g_sci->getGameId() == GID_QFG4 && g_sci->isCD() && stack.getModule() == 520 &&
			t.noun == 2 && t.verb == 59 && t.cond == 0) {
			// The CD edition mangled the Rusalka flowers dialogue. - bug #10849
			// In the floppy edition, there are 3 lines, the first from
			// the narrator, then two from Rusalka. The CD edition omits
			// narration and only has the 3rd text, with the 2nd audio! The
			// 3rd audio is orphaned but available.
			//
			// We only restore Rusalka's lines, providing the correct text
			// for seq:1 to match the audio. We respond to seq:2 requests
			// with Rusalka's last text. The orphaned audio (seq:3) has its
			// tuple adjusted to seq:2 in resource_audio.cpp.
			if (t.seq == 1) {
				record.tuple = t;
				record.refTuple = MessageTuple();
				record.talker = 28;
				record.string = "Thank you for the beautiful flowers.  No one has been so nice to me since I can remember.";
				record.length = 89;
				delete reader;
				return true;
			} else if (t.seq == 2) {
				// The CD edition ships with this text at seq:1.
				//  Look it up instead of hardcoding.
				t.seq = 1;
				if (!reader->findRecord(t, record)) {
					delete reader;
					return false;
				}
				t.seq = 2;             // Prevent an endless 2=1 -> 2=1 -> 2=1... loop.
				record.tuple.seq = 2;  // Make the record seq:2 to get the seq:2 audio.
				record.refTuple = MessageTuple();
				delete reader;
				return true;
			}
		}

		if (g_sci->getGameId() == GID_LAURABOW2 && !g_sci->isCD() && stack.getModule() == 1885 &&
			t.noun == 1 && t.verb == 6 && t.cond == 16 && t.seq == 4 &&
			(g_sci->getEngineState()->currentRoomNumber() == 350 ||
			 g_sci->getEngineState()->currentRoomNumber() == 360 ||
			 g_sci->getEngineState()->currentRoomNumber() == 370)) {
			// Asking Yvette about Tut in act 2 party - bug #10723
			// Skip the last two lines of dialogue about a murder that hasn't occurred yet.
			// Sierra fixed this in cd version by creating a copy of this message without those lines.
			// Room-specific as the message is used again later where it should display in full.
			t.seq += 2;
		}

		// Fill in known missing message tuples
		if (g_sci->getGameId() == GID_SQ4 && stack.getModule() == 16 &&
			t.noun == 7 && t.verb == 0 && t.cond == 3 && t.seq == 1) {
			// This fixes the error message shown when speech and subtitles are
			// enabled simultaneously in SQ4 - the (very) long dialog when Roger
			// is talking with the aliens is missing - bug #3538416.
			record.tuple = t;
			record.refTuple = MessageTuple();
			record.talker = 7;	// Roger
			// The missing text is just too big to fit in one speech bubble, and
			// if it's added here manually and drawn on screen, it's painted over
			// the entrance in the back where the Sequel Police enters, so it
			// looks very ugly. Perhaps this is why this particular text is missing,
			// as the text shown in this screen is very short (one-liners).
			// Just output an empty string here instead of showing an error.
			record.string = "";
			record.length = 0;
			delete reader;
			return true;
		}

		// FPFP CD has several message sequences where audio and text were left
		//  out of sync. This is probably because this version didn't formally
		//  support text mode. Most of these texts just say "Dummy Msg". All the
		//  real text is there but it's either concatenated or in a different
		//  tuple. We extract and return the correct text. Fixes #10964
		if (g_sci->getGameId() == GID_FREDDYPHARKAS && g_sci->isCD() &&
			g_sci->getLanguage() == Common::EN_ANY && !g_sci->isDemo()) {
			byte textSeq = 0;
			uint32 substringIndex = 0;
			uint32 substringLength = 0;

			// lever brothers' introduction
			if (stack.getModule() == 220 && t.noun == 24 && t.verb == 0 && t.cond == 0) {
				switch (t.seq) {
				case 1: textSeq = 1; substringIndex = 0;   substringLength = 25; break;
				case 2: textSeq = 1; substringIndex = 26;  substringLength = 20; break;
				case 3: textSeq = 1; substringIndex = 47;  substringLength = 58; break;
				case 4: textSeq = 1; substringIndex = 106; substringLength = 34; break;
				case 5: textSeq = 1; substringIndex = 141; substringLength = 27; break;
				case 6: textSeq = 1; substringIndex = 169; substringLength = 29; break;
				case 7: textSeq = 1; substringIndex = 199; substringLength = 52; break;
				case 8: textSeq = 1; substringIndex = 252; substringLength = 37; break;
				}
			}

			// kenny's introduction
			if (stack.getModule() == 220 && t.noun == 30 && t.verb == 0 && t.cond == 0) {
				switch (t.seq) {
				case 3: textSeq = 3; substringIndex = 0;  substringLength = 14;  break;
				case 4: textSeq = 3; substringIndex = 15; substringLength = 245; break;
				case 5: textSeq = 4; break;
				}
			}

			// helen swatting flies
			if (stack.getModule() == 660 && t.noun == 35 && t.verb == 0 && t.cond == 0) {
				switch (t.seq) {
				case 1: textSeq = 1; substringIndex = 0;   substringLength = 42; break;
				case 2: textSeq = 1; substringIndex = 43;  substringLength = 93; break;
				case 3: textSeq = 1; substringIndex = 137; substringLength = 72; break;
				case 4: textSeq = 2; break;
				case 5: textSeq = 1; substringIndex = 210; substringLength = 57; break;
				case 6: textSeq = 3; break;
				}
			}

			// find the original message record and the record that contains the
			//  correct text, then use the correct substring. the original must
			//  be used to preserve its correct talker and tuple values.
			if (textSeq != 0 && reader->findRecord(t, record)) {
				MessageTuple textTuple(t.noun, t.verb, t.cond, textSeq);
				MessageRecord textRecord;
				if (reader->findRecord(textTuple, textRecord)) {
					uint32 textLength = (substringLength == 0) ? textRecord.length : substringLength;
					if (substringIndex + textLength <= textRecord.length) {
						record.string = textRecord.string + substringIndex;
						record.length = textLength;
						delete reader;
						return true;
					}
				}
			}
		}

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
