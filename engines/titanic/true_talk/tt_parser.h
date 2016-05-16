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

#ifndef TITANIC_TT_PARSER_H
#define TITANIC_TT_PARSER_H

#include "titanic/true_talk/tt_input.h"

namespace Titanic {

class CScriptHandler;

class TTparser {
private:
	/**
	 * Normalizes a passed input, taking care of things like removing extra
	 * spaces and lowercasing everything
	 */
	int normalize(TTinput *input);
	
	/**
	 * Submethod called by normalize to handle expanding contacted word pairs
	 * like can't, should've, and so on.
	 */
	bool normalizeContraction(const TTstring &srcLine, int srcIndex, TTstring &destLine);

	/**
	 * Checks for what is likely special developer cheat codes
	 */
	static int isEmoticon(const TTstring &str, int &index);

	/**
	 * Checks if any word within a passed line has an entry in the list of replacements,
	 * and if found, replaces it with it's equivalent replacement string
	 * @param line			Line to check
	 * @param strings		List of strings to check for. Strings come in pairs, with the
	 * first being the string to match, and the second the replacement
	 */
	static void searchAndReplace(TTstring &line, const StringArray &strings);

	/**
	 * Checks the string starting at a given index for any word in the passed string array,
	 * and if found, replaces it in the line with it's replacement
	 * @param line			Line to check
	 * @param startIndex	Starting index in the start to check
	 * @param strings		List of strings to check for. Strings come in pairs, with the
	 * first being the string to match, and the second the replacement
	 * @returns				Index of the start of the following word			
	 */
	static int searchAndReplace(TTstring &line, int startIndex, const StringArray &strings);

	/**
	 * Checks the string starting at a given index for a number representation
	 * such as roman numericals, spelled out numbers, etc. and replaces it with
	 * a plain decimal representation.
	 * @param line			Line to check
	 * @param startIndex	Starting index in the start to check
	 * @returns				True if end of line hasn't been reached yet
	 */
	static bool replaceNumbers(TTstring &line, int *startIndex);
public:
	CScriptHandler *_owner;
	int _field4;
	TTinput *_input;
	int _fieldC;
	int _field10;
	int _field14;
	int _field18;
public:
	TTparser(CScriptHandler *owner) : _owner(owner), _field4(0),
		_input(nullptr), _fieldC(0), _field10(0), _field14(0), _field18(0) {}

	/**
	 * Gets passed a newly created input wrapper during conversation text processing
	 */
	int processInput(TTinput *input);
};

} // End of namespace Titanic

#endif /* TITANIC_TT_PARSER_H */
