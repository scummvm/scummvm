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

StringArray StringTokenizer::split() {
	StringArray res;

	while (!empty())
		res.push_back(nextToken());

	return res;
}

String StringTokenizer::delimitersAtTokenBegin() const {
	// First token appears at beginning of the string, or no tokens have been extracted yet
	if (_tokenBegin == 0)
		return String();

	// Iterate backwards until we hit either the previous token, or the beginning of the input string
	int delimitersBegin;
	for (delimitersBegin = _tokenBegin - 1; delimitersBegin >= 0 && _delimiters.contains(_str[delimitersBegin]); delimitersBegin--)
		;

	++delimitersBegin;
	
	// Return the delimiters
	return String(_str.c_str() + delimitersBegin, _tokenBegin - delimitersBegin);
}

String StringTokenizer::delimitersAtTokenEnd() const {
	// Last token appears at end of the string, or no tokens have been extracted yet
	if (_tokenEnd == 0 || _tokenEnd == _str.size())
		return String();

	// Iterate forwards until we hit either the next token, or the end of the input string
	uint delimitersEnd;
	for (delimitersEnd = _tokenEnd; delimitersEnd < _str.size() && _delimiters.contains(_str[delimitersEnd]); delimitersEnd++)
		;
	
	// Return the delimiters
	return String(_str.c_str() + _tokenEnd, delimitersEnd - _tokenEnd);
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
	_tokenBegin = _tokenEnd;
	while (_tokenBegin != _str.end() && _delimiters.contains(*_tokenBegin))
		_tokenBegin++;
	_tokenEnd = _tokenBegin;

	// Loop and advance _tokenEnd until we find a delimiter at the end of a word/string
	while (_tokenBegin != _str.end() && _tokenEnd != _str.end()) {
		if (_delimiters.contains(*_tokenEnd)) {
			return U32String(_tokenBegin, _tokenEnd);
		}
		_tokenEnd++;
	}

	// Returning the last word if _tokenBegin iterator isn't at the end.
	if (_tokenBegin != _str.end())
		return U32String(_tokenBegin, _tokenEnd);
	else
		return U32String();
}

U32StringArray U32StringTokenizer::split() {
	U32StringArray res;

	while (!empty())
		res.push_back(nextToken());

	return res;
}

U32String U32StringTokenizer::delimitersAtTokenBegin() const {
	// First token appears at beginning of the string, or no tokens have been extracted yet
	if (_tokenBegin == _str.begin())
		return U32String();

	// Iterate backwards until we hit either the previous token, or the beginning of the input string
	U32String::const_iterator delimitersBegin;
	for (delimitersBegin = _tokenBegin - 1; delimitersBegin >= _str.begin() && _delimiters.contains(*delimitersBegin); delimitersBegin--)
		;

	++delimitersBegin;
	
	// Return the delimiters
	return U32String(delimitersBegin, _tokenBegin - delimitersBegin);
}

U32String U32StringTokenizer::delimitersAtTokenEnd() const {
	// Last token appears at end of the string, or no tokens have been extracted yet
	if (_tokenEnd == _str.begin() || _tokenEnd == _str.end())
		return String();

	// Iterate forwards until we hit either the next token, or the end of the input string
	U32String::const_iterator delimitersEnd;
	for (delimitersEnd = _tokenEnd; delimitersEnd < _str.end() && _delimiters.contains(*delimitersEnd); delimitersEnd++)
		;
	
	// Return the delimiters
	return U32String(_tokenEnd, delimitersEnd - _tokenEnd);
}


} // End of namespace Common
