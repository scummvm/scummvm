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

#include "titanic/sound/music_song.h"
#include "titanic/support/files_manager.h"
#include "titanic/titanic.h"
#include "common/util.h"

namespace Titanic {

CMusicSong::CMusicSong(int index) {
	// Read in the list of song strings
	Common::SeekableReadStream *res = g_vm->_filesManager->getResource("MUSIC/PARSER");
	Common::StringArray parserStrings;
	while (res->pos() < res->size())
		parserStrings.push_back(readStringFromStream(res));
	delete res;

	// Set up a new song parser with the desired string
	CSongParser parser(parserStrings[index].c_str());

	// Count how many encoded values there are
	CValuePair r;
	int count = 0;
	while (parser.parse(r))
		++count;
	assert(count > 0);

	// Read in the values to the array
	_data.resize(count);
	parser.reset();
	for (int idx = 0; idx < count; ++idx)
		parser.parse(_data[idx]);

	// Figure out the range of values in the array
	_minVal = 0x7FFFFFFF;
	int maxVal = -0x7FFFFFFF;

	for (int idx = 0; idx < count; ++idx) {
		CValuePair &vp = _data[idx];
		if (vp._data != 0x7FFFFFFF) {
			if (vp._data < _minVal)
				_minVal = vp._data;
			if (vp._data > maxVal)
				maxVal = vp._data;
		}
	}

	_range = maxVal - _minVal;
}

CMusicSong::~CMusicSong() {
	_data.clear();
}

/*------------------------------------------------------------------------*/

#define FETCH_CHAR _currentChar = _str[_strIndex++]

CSongParser::CSongParser(const char *str) : _str(str), _strIndex(0),
		_field8(0), _priorChar('A'), _field10(32), _field14(0), _flag(false),
		_field1C(0), _currentChar(' '), _numValue(1) {
}

void CSongParser::reset() {
	_strIndex = 0;
	_field8 = 0;
	_field10 = 0;
	_field14 = 0;
	_currentChar = ' ';
	_priorChar = 'A';
	_numValue = 1;
	_field1C = 0;
}

bool CSongParser::parse(CValuePair &r) {
	const int INDEXES[8] = { 0, 2, 3, 5, 7, 8, 10, 0 };

	while (_currentChar) {
		skipSpaces();

		if (Common::isDigit(_currentChar)) {
			// Parse the number
			Common::String numStr;
			do {
				numStr += _currentChar;
				FETCH_CHAR;
			} while (_currentChar && Common::isDigit(_currentChar));

			_numValue = atoi(numStr.c_str());
		} else if (_currentChar == ',') {
			_field10 = _numValue;
			FETCH_CHAR;
		} else if (_currentChar == ':') {
			_priorChar = 'A';
			_field8 = _numValue * 12;
			FETCH_CHAR;
		} else if (_currentChar == '/') {
			r._length += _field10;
			_field1C += _field10;
			FETCH_CHAR;
		} else if (_currentChar == '+') {
			++_field14;
			FETCH_CHAR;
		} else if (_currentChar == '-') {
			--_field14;
			FETCH_CHAR;
		} else if (_currentChar == '^') {
			if (_flag)
				break;

			_flag = true;
			r._data = 0x7FFFFFFF;
			r._length = _field10;
			_field14 = 0;
			_field1C += _field10;
			FETCH_CHAR;
		} else if (_currentChar == '|') {
			_field1C = 0;
			FETCH_CHAR;
		} else if (Common::isAlpha(_currentChar)) {
			if (_flag)
				break;

			int val1 = INDEXES[tolower(_currentChar) - 'a'];
			int val2 = INDEXES[tolower(_priorChar) - 'a'];
			bool flag = true;

			if (_currentChar == _priorChar) {
				r._data = _field8;
			} else if (_currentChar >= 'a' && _currentChar <= 'g') {
				val1 -= val2;
				if (val1 >= 0)
					val1 -= 12;
				r._data = _field8 + val1;
			} else if (_currentChar >= 'A' && _currentChar <= 'G') {
				val1 -= val2;
				if (val1 <= 0)
					val1 += 12;
				r._data = _field8 + val1;
			} else {
				flag = false;
			}

			if (flag) {
				r._length = _field10;
				_field1C += _field10;
				_field8 = r._data;
				_priorChar = _currentChar;
				r._data += _field14;
				_field14 = 0;
				_flag = true;
			}

			FETCH_CHAR;
		} else {
			FETCH_CHAR;
		}
	}

	if (!_flag)
		return false;

	_flag = false;
	return true;
}

void CSongParser::skipSpaces() {
	while (_currentChar && Common::isSpace(_currentChar)) {
		FETCH_CHAR;
	}
}

} // End of namespace Titanic
