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

#ifndef ADL_PARSER_H
#define ADL_PARSER_H

#include "common/types.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

namespace Common {
class ReadStream;
class String;
}

namespace Adl {

class Display;

class Parser {
public:
	Parser(AdlEngine &engine, Display &display);

	void loadVerbs(Common::ReadStream &stream) { loadWords(stream, _verbs); }
	void loadNouns(Common::ReadStream &stream) { loadWords(stream, _nouns); }
	void getInput(uint &verb, uint &noun);

private:
	enum {
		kWordSize = 8
	};

	typedef Common::HashMap<Common::String, uint> WordMap;

	void loadWords(Common::ReadStream &stream, WordMap &map);
	Common::String getLine();
	Common::String getWord(const Common::String &line, uint &index);

	AdlEngine &_engine;
	Display &_display;
	WordMap _verbs;
	WordMap _nouns;
};

} // End of namespace Adl

#endif
