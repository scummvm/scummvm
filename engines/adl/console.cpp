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

#include "adl/console.h"
#include "adl/adl.h"

namespace Adl {

Console::Console(AdlEngine *engine) : GUI::Debugger() {
	_engine = engine;

	registerCmd("help", WRAP_METHOD(Console, Cmd_Help));
	registerCmd("nouns", WRAP_METHOD(Console, Cmd_Nouns));
	registerCmd("verbs", WRAP_METHOD(Console, Cmd_Verbs));
}

static Common::String toAscii(const Common::String &str) {
	Common::String ascii(str);

	for (uint i = 0; i < ascii.size(); ++i)
		ascii.setChar(ascii[i] & 0x7f, i);

	return ascii;
}

bool Console::Cmd_Help(int argc, const char **argv) {
	debugPrintf("Parser:\n");
	debugPrintf(" verbs - Lists the vocabulary verbs\n");
	debugPrintf(" nouns - Lists the vocabulary nouns\n");
	return true;
}

bool Console::Cmd_Verbs(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	debugPrintf("Verbs in alphabetical order:\n");
	printWordMap(_engine->_verbs);
	return true;
}

bool Console::Cmd_Nouns(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	debugPrintf("Nouns in alphabetical order:\n");
	printWordMap(_engine->_nouns);
	return true;
}

void Console::printWordMap(const WordMap &wordMap) {
	Common::StringArray words;
	WordMap::const_iterator verb;

	for (verb = wordMap.begin(); verb != wordMap.end(); ++verb)
		words.push_back(verb->_key);

	Common::sort(words.begin(), words.end());

	Common::StringArray::const_iterator word;
	for (word = words.begin(); word != words.end(); ++word)
		debugPrintf("%s: %d\n", toAscii(*word).c_str(), wordMap[*word]);
}

} // End of namespace Adl
