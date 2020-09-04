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

#include "common/tokenizer.h"

namespace Common {

StringTokenizer::StringTokenizer(const String &str, const String &delimiters) : _str(str), _delimiters(delimiters) {
	reset();
}

void StringTokenizer::reset() {
	_tokenBegin = _tokenEnd = 0;
}

bool StringTokenizer::empty() const {
	// Search for the next token's start (i.e. the next non-delimiter character)
	for (uint i = _tokenEnd; i < _str.size(); i++) {
		if (!_delimiters.contains(_str[i]))
			return false; // Found a token so the tokenizer is not empty
	}
	// Didn't find any more tokens so the tokenizer is empty
	return true;
}

String StringTokenizer::nextToken() {
	// Seek to next token's start (i.e. jump over the delimiters before next token)
	for (_tokenBegin = _tokenEnd; _tokenBegin < _str.size() && _delimiters.contains(_str[_tokenBegin]); _tokenBegin++)
		;
	// Seek to the token's end (i.e. jump over the non-delimiters)
	for (_tokenEnd = _tokenBegin; _tokenEnd < _str.size() && !_delimiters.contains(_str[_tokenEnd]); _tokenEnd++)
		;
	// Return the found token
	return String(_str.c_str() + _tokenBegin, _tokenEnd - _tokenBegin);
}

U32StringTokenizer::U32StringTokenizer(const U32String &str, const String &delimiters) : _str(str), _delimiters(delimiters) {
	reset();
}

void U32StringTokenizer::reset() {
	_tokenBegin = _tokenEnd = _str.begin();
}

bool U32StringTokenizer::empty() const {
	// Search for the next token's start (i.e. the next non-delimiter character)
	for (U32String::const_iterator itr = _tokenEnd; itr != _str.end(); itr++) {
		if (!_delimiters.contains(*itr)) {
			return false; // Found a token so the tokenizer is not empty
		}
	}

	// Didn't find any more tokens so the tokenizer is empty
	return true;
}

U32String U32StringTokenizer::nextToken() {
	// Skip delimiters when present at the beginning, to point to the next token
	// For example, the below loop will set _tokenBegin & _tokenEnd to 'H' for the string -> "!!--=Hello World"
	// And subsequently, skip all delimiters in the beginning of the next word.
	while (_tokenBegin != _str.end() && _delimiters.contains(*_tokenBegin)) {
		_tokenBegin++;
		_tokenEnd++;
	}

	// Loop and advance _tokenEnd until we find a delimiter at the end of a word/string
	while (_tokenBegin != _str.end() && _tokenEnd != _str.end()) {
		if (_delimiters.contains(*_tokenEnd)) {
			U32String token(_tokenBegin, _tokenEnd);
			_tokenEnd++;
			_tokenBegin = _tokenEnd;
			return token;
		}
		_tokenEnd++;
	}

	// Returning the last word if _tokenBegin iterator isn't at the end.
	if (_tokenBegin != _str.end())
		return U32String(_tokenBegin, _tokenEnd);
	else
		return U32String();
}


} // End of namespace Common
