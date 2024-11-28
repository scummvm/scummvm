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

template<class T>
BaseStringTokenizer<T>::BaseStringTokenizer(const T &str, const String &delimiters) : _str(str), _delimiters(delimiters) {
	reset();
}

template<class T>
void BaseStringTokenizer<T>::reset() {
	_tokenBegin = _tokenEnd = _str.begin();
}

template<class T>
bool BaseStringTokenizer<T>::empty() const {
	// Search for the next token's start (i.e. the next non-delimiter character)
	for (typename T::const_iterator itr = _tokenEnd; itr != _str.end(); itr++) {
		if (!_delimiters.contains(*itr)) {
			return false; // Found a token so the tokenizer is not empty
		}
	}

	// Didn't find any more tokens so the tokenizer is empty
	return true;
}

template<class T>
T BaseStringTokenizer<T>::nextToken() {
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
			return T(_tokenBegin, _tokenEnd);
		}
		_tokenEnd++;
	}

	// Returning the last word if _tokenBegin iterator isn't at the end.
	if (_tokenBegin != _str.end())
		return T(_tokenBegin, _tokenEnd);
	else
		return T();
}

template<class T>
Array<T> BaseStringTokenizer<T>::split() {
	Array<T> res;

	while (!empty())
		res.push_back(nextToken());

	return res;
}

template<class T>
T BaseStringTokenizer<T>::delimitersAtTokenBegin() const {
	// First token appears at beginning of the string, or no tokens have been extracted yet
	if (_tokenBegin == _str.begin())
		return T();

	// Iterate backwards until we hit either the previous token, or the beginning of the input string
	typename T::const_iterator delimitersBegin;
	for (delimitersBegin = _tokenBegin - 1; delimitersBegin >= _str.begin() && _delimiters.contains(*delimitersBegin); delimitersBegin--)
		;

	++delimitersBegin;
	
	// Return the delimiters
	return T(delimitersBegin, _tokenBegin - delimitersBegin);
}

template<class T>
T BaseStringTokenizer<T>::delimitersAtTokenEnd() const {
	// Last token appears at end of the string, or no tokens have been extracted yet
	if (_tokenEnd == _str.begin() || _tokenEnd == _str.end())
		return T();

	// Iterate forwards until we hit either the next token, or the end of the input string
	typename T::const_iterator delimitersEnd;
	for (delimitersEnd = _tokenEnd; delimitersEnd < _str.end() && _delimiters.contains(*delimitersEnd); delimitersEnd++)
		;
	
	// Return the delimiters
	return T(_tokenEnd, delimitersEnd - _tokenEnd);
}

template class BaseStringTokenizer<String>;
template class BaseStringTokenizer<U32String>;

} // End of namespace Common
