/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "tesoro/core/scriptblock.h"
#include "tesoro/core/namekey.h"
#include "tesoro/core/speechtable.h"
#include "tesoro/core/stdiotext.h"

namespace Tesoro {

/** Reads the rest of a "sendmessage" line after its first two letters into the entry. */
static bool parseSendMessage(StdioText &in, const SpeechTables &speech, const Common::Array<uint64> &characters,
							 ScriptEntry &e, char *buf, int &lastChar) {
	in.scan("ndmessage ");
	int c = in.get();
	lastChar = c & 0xff;
	if (c != '\'') {
		// A digit picks one of the characters listed in the block's header.
		const int index = (c & 0xff) - '1';
		if (index >= 0 && index < (int)characters.size())
			e.setKey(4, characters[index]);
		e.setWord(12, 0);
		in.get();
	} else {
		in.scan("%[^']s", buf);
		in.scan("' ");
		e.setKey(4, nameKey64(buf));
		c = in.get();
		lastChar = c & 0xff;
		if (c == 'i') {
			in.scan("n %s", buf);
			if (strcmp(buf, "INVENTARIO") == 0) {
				e.setWord(12, kInventoryRef);
			} else {
				int32 level = 0;
				StdioText(reinterpret_cast<const byte *>(buf), strlen(buf)).scan("LEVEL%i", &level);
				e.setWord(12, 1 + level);
			}
			in.get();
		} else {
			in.unget(c & 0xff);
			e.setWord(12, 0);
		}
	}

	int32 x = 0, y = 0;
	switch (in.get()) {
	case 'a':
		in.scan("ctivate o");
		e.setWord(16, kMessageActivate);
		e.data[20] = in.get() == 'n';
		break;
	case 'i':
		in.scan("ra %i %i", &x, &y);
		e.setWord(16, kMessageWalk);
		e.setHalf(20, (uint16)x);
		e.setHalf(22, (uint16)y);
		break;
	case 'm':
		in.scan("irar %s", buf);
		if (strcmp(buf, "ARRIBA") == 0)
			e.data[20] = 1;
		else if (strcmp(buf, "ABAJO") == 0)
			e.data[20] = 2;
		else if (strcmp(buf, "IZQUIERDA") == 0)
			e.data[20] = 3;
		else if (strcmp(buf, "DERECHA") == 0)
			e.data[20] = 4;
		e.setWord(16, kMessageLook);
		break;
	case 'p':
		in.scan("lay %i %i %s", &x, &y, buf);
		e.data[20] = (byte)x;
		e.data[21] = (byte)y;
		if (strcmp(buf, "play") == 0)
			e.data[22] = 1;
		else if (strcmp(buf, "stop") == 0)
			e.data[22] = 0;
		else if (strcmp(buf, "loop") == 0)
			e.data[22] = 2;
		else if (strcmp(buf, "wait") == 0)
			e.data[22] = 3;
		e.setWord(16, kMessagePlay);
		break;
	case 's':
		in.scan("how o");
		e.setWord(16, kMessageShow);
		e.data[20] = in.get() == 'n';
		break;
	default:
		return false;
	}
	e.setWord(0, kScriptSendMessage);
	return true;
}

uint32 ScriptBlock::findLabel(uint64 key) const {
	for (uint32 i = 0; i < _labels.size(); ++i) {
		if (_labels[i].key == key)
			return _labels[i].entry;
	}
	return 0;
}

bool ScriptBlock::parse(StdioText &in, const SpeechTables &speech) {
	char buf[128] = "";
	int32 count = 0;

	_entries.clear();
	_labels.clear();
	_characters.clear();
	_type = 0;

	in.scan("%s", buf);
	_key = nameKey64(buf);
	if (in.get() == ' ')
		in.scan("%i\n", &_type);
	else
		_type = 0;
	in.scan("%i\n", &count);
	for (int32 i = 0; i < count; ++i) {
		in.scan("%s\n", buf);
		_characters.push_back(nameKey64(buf));
	}

	uint32 ordinal = 0;
	for (;;) {
		int lastChar = in.get() & 0xff;
		ScriptEntry e;

		// it really suffices with reading the first character of the line to determine the opcode.
		switch (lastChar) {
		case ' ': {
			// A spoken line: "<phrase>, <n>[, <m>]"
			e.setWord(0, kScriptSay);
			in.scan("%[^,]s", buf);
			e.setWord(4, speech.find(buf));
			in.scan(", %i", e.data + 8);
			const int c = in.get();
			lastChar = c & 0xff;
			if (c == ',')
				in.scan("%i", e.data + 12);
			else
				in.unget(c & 0xff);
			break;
		}
		case '.':
			e.setWord(0, kScriptEnd);
			break;
		case 'D':
			e.setWord(0, kScriptDo);
			e.setWord(4, ordinal++);
			break;
		case '_': {
			in.scan("%[0-\xa1]s", buf);
			Label l;
			l.key = nameKey64(buf);
			l.entry = _entries.size();
			_labels.push_back(l);
			goto restOfLine;
		}
		case 'b':
			e.setWord(0, kScriptBreak);
			break;
		case 'i': {
			int32 variable = 0, value = 0;
			in.scan("f v%i == %i jmp %[0-\xa1]s", &variable, &value, buf);
			e.setWord(0, kScriptIfEquals);
			e.data[12] = (byte)variable;
			e.data[13] = (byte)value;
			e.setKey(4, nameKey64(buf));
			break;
		}
		case 'j':
			in.scan("mp %s", buf);
			e.setWord(0, kScriptJump);
			e.setKey(4, nameKey64(buf));
			break;
		case 'm': {
			buf[3] = '\0';
			in.scan("%s", buf);
			in.get();
			if (buf[3] == '\0') {
				e.setWord(0, kScriptMenuBegin);
				break;
			}
			in.scan("%s", buf);
			e.setHalf(4, buf[0] == 'n' ? 1 : 0);
			const int c = in.get();
			if (c == '\n')
				in.unget(c);
			in.scan("%[^'\n']s", buf);
			int32 variable = 0;
			if (buf[0] == '!') {
				e.data[7] = 1;
				StdioText(reinterpret_cast<const byte *>(buf + 2), strlen(buf + 2)).scan("%i", &variable);
				e.data[6] = (byte)variable;
			} else if (buf[0] == 'v') {
				e.data[7] = 0;
				StdioText(reinterpret_cast<const byte *>(buf + 1), strlen(buf + 1)).scan("%i", &variable);
				e.data[6] = (byte)variable;
			} else {
				e.data[7] = 0;
				e.data[6] = 0xff;
			}
			e.setWord(0, kScriptMenuItem);
			break;
		}
		case 'p': {
			in.scan("lay %s", buf);
			in.get();
			switch (buf[0]) {
			case 'a':
				e.setWord(0, kScriptPlayVideo);
				in.scan("%i", e.data + 4);
				break;
			case 'c':
				e.setWord(0, kScriptPlayTrack);
				in.scan("%i", e.data + 4);
				break;
			case 's': {
				e.setWord(0, kScriptPlaySound);
				in.get();
				char name[128] = "";
				in.scan("%[^']s", name);
				memcpy(e.data + 4, name, MIN<uint>(strlen(name) + 1, ScriptEntry::kSize - 4));
				break;
			}
			default:
				goto restOfLine;
			}
			break;
		}
		case 'r': {
			const int c = in.get();
			lastChar = c & 0xff;
			if (c == 'e') {
				e.setWord(0, kScriptReturn);
			} else if (c == 'u') {
				in.scan("n '%[^']s'", buf);
				e.setKey(4, nameKey64(buf));
				in.scan("' %[0-\xa1]s", buf);
				e.setKey(12, nameKey64(buf));
				e.setWord(0, kScriptRun);
			} else {
				return false;
			}
			break;
		}
		case 's': {
			const int c = in.get();
			lastChar = c & 0xff;
			if (c == 'l') {
				e.setWord(0, kScriptSleep);
				in.scan("eep %i", e.data + 4);
			} else if (c == 'e') {
				if (!parseSendMessage(in, speech, _characters, e, buf, lastChar))
					return false;
			} else {
				return false;
			}
			break;
		}
		case 'v':
			in.scan("%i = %i", e.data + 4, e.data + 5);
			e.setWord(0, kScriptSetVariable);
			break;
		default:
			return false;
		}
		_entries.push_back(e);

	restOfLine:
		int c;
		do {
			c = in.get();
		} while (c != '\n' && !in.eof());
		if (lastChar == '.') {
			do {
				c = in.get();
			} while (c != '\n' && !in.eof());
			return true;
		}
	}
}

ScriptLibrary::~ScriptLibrary() {
	clear();
}

void ScriptLibrary::clear() {
	for (uint32 i = 0; i < _blocks.size(); ++i)
		delete _blocks[i];
	_blocks.clear();
}

bool ScriptLibrary::load(const byte *text, uint32 size, const SpeechTables &speech) {
	StdioText in(text, size);
	while (in.get() == '.') {
		ScriptBlock *block = new ScriptBlock();
		const bool ok = block->parse(in, speech);
		_blocks.push_back(block);
		if (!ok)
			return false;

		if ((uint32)(size - 8) < in.consumed())
			break;
	}
	return true;
}

const ScriptBlock *ScriptLibrary::find(uint64 key) const {
	for (uint32 i = 0; i < _blocks.size(); ++i) {
		if (_blocks[i]->key() == key)
			return _blocks[i];
	}
	return nullptr;
}

void ScriptLibrary::resolve() {
	const uint64 throneBlock = nameKey64("FANMTRONO");
	for (uint32 b = 0; b < _blocks.size(); ++b) {
		ScriptBlock &block = *_blocks[b];
		int previousItem = -1;
		for (uint32 i = 0; i < block._entries.size(); ++i) {
			ScriptEntry &e = block._entries[i];
			switch (e.op()) {
			case kScriptJump:
				e.setWord(4, block.findLabel(e.key(4)));
				break;
			case kScriptMenuBegin:
				previousItem = -1;
				break;
			case kScriptMenuItem:
				if (previousItem > 0)
					block._entries[previousItem].setHalf(4, (uint16)i);
				previousItem = i;
				if (e.half(4) == 0)
					previousItem = -1;
				e.setHalf(4, 0xffff);
				break;
			case kScriptIfEquals:
				e.setWord(4, block.findLabel(e.key(4)));
				e.data[9] = e.data[13];
				e.data[8] = e.data[12];
				break;
			case kScriptRun: {
				if (e.key(4) == throneBlock) {
					e.setWord(4, 0);
				} else {
					uint32 ref = 0;
					for (uint32 k = 0; k < _blocks.size(); ++k) {
						if (_blocks[k]->key() == e.key(4)) {
							ref = k + 1;
							break;
						}
					}
					e.setWord(4, ref);
				}
				e.setWord(8, e.word(12));
				e.setWord(12, e.word(16));
				break;
			}
			default:
				break;
			}
		}
	}
}

} // End of namespace Tesoro
