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

#include "adl/adl.h"
#include "adl/parser.h"
#include "adl/display.h"

#include "engines/engine.h"

#include "common/str.h"
#include "common/stream.h"
#include "common/debug.h"
#include "common/textconsole.h"

namespace Adl {

Parser::Parser(AdlEngine &engine, Display &display) :
		_engine(engine),
		_display(display) {

}

void Parser::loadWords(Common::ReadStream &stream, WordMap &map) {
	uint index = 0;

	while (1) {
		++index;

		byte buf[kWordSize];

		if (stream.read(buf, kWordSize) < kWordSize)
			error("Error reading word list");

		Common::String word((char *)buf, kWordSize);

		if (!map.contains(word))
			map[word] = index;

		byte synonyms = stream.readByte();

		if (stream.err() || stream.eos())
			error("Error reading word list");

		if (synonyms == 0xff)
			break;

		for (uint i = 0; i < synonyms; ++i) {
			if (stream.read((char *)buf, kWordSize) < kWordSize)
				error("Error reading word list");

			word = Common::String((char *)buf, kWordSize);

			if (!map.contains(word))
				map[word] = index;
		}
	}
}

Common::String Parser::getLine() {
 	// Original engine uses a global here, which isn't reset between
 	// calls and may not match actual mode
	bool textMode = false;

	while (1) {
		Common::String line = _display.inputString(A2CHAR('?'));

		if (g_engine->shouldQuit())
			return "";

		if ((byte)line[0] == ('\r' | 0x80)) {
			textMode = !textMode;
			_display.setMode(textMode ? Display::kModeText : Display::kModeMixed);
			continue;
		}

		// Remove the return
		line.deleteLastChar();
		return line;
	}
}

Common::String Parser::getWord(const Common::String &line, uint &index) {
	Common::String str;

	for (uint i = 0; i < 8; ++i)
		str += (char)(A2CHAR(' '));

	int copied = 0;

	// Skip initial whitespace
	while (1) {
		if (index == line.size())
			return str;
		if (line[index] != (char)(A2CHAR(' ')))
			break;
		++index;
	}

	// Copy up to 8 characters
	while (1) {
		if (copied < 8)
			str.setChar(line[index], copied++);

		index++;

		if (index == line.size() || line[index] == (char)(A2CHAR(' ')))
			return str;
	}
}

void Parser::getInput(uint &verb, uint &noun) {
	while (1) {
		_display.printString(_engine.getExeString(STR_COMMON_ENTERCMD));
		Common::String line = getLine();

		if (g_engine->shouldQuit())
			return;

		uint index = 0;
		Common::String verbStr = getWord(line, index);
		debug("Verb: \"%s\"", appleToAscii(verbStr).c_str());

		if (!_verbs.contains(verbStr)) {
			Common::String err = _engine.getExeString(STR_COMMON_VERBERR);
			for (uint i = 0; i < verbStr.size(); ++i)
				err.setChar(verbStr[i], i + 19);
			_display.printString(err);
			continue;
		}

		verb = _verbs[verbStr];
		debug("Verb ID: %i", verb);

		Common::String nounStr = getWord(line, index);
		debug("Noun: \"%s\"", appleToAscii(nounStr).c_str());

		if (!_nouns.contains(nounStr)) {
			Common::String err = _engine.getExeString(STR_COMMON_NOUNERR);
			for (uint i = 0; i < verbStr.size(); ++i)
				err.setChar(verbStr[i], i + 19);
			for (uint i = 0; i < nounStr.size(); ++i)
				err.setChar(nounStr[i], i + 30);
			_display.printString(err);
			continue;
		}

		noun = _nouns[nounStr];
		debug("Noun ID: %i", noun);
		return;
	}
}

} // End of namespace Adl
