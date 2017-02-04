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

#include "titanic/sound/music_object.h"
#include "common/util.h"

namespace Titanic {

static const char *const DATA[4] = {
	"64,^|^|^|^|^|^|^|^|^|^|^|^|^|^|^|^|8,^^^^ 5:A///|64,/|/|/|/|/|/|/"
	"|/|/|/|/|/|/|/|/|/|^|^|^|^|^|^|^|^|^|16, ^B//|64,/|/|/|/|^|16,^C/"
	"/|64,/|/|/|/|",
	"2:8,^^^^B//a|//g//B//|g///B//a|//g//A//|B//^C//b|//a//a//|BCb/b//"
	"a|//g//A//|g/+f/D//c|//b//gA/|g/^^C//C|//C//a//|BCb////a|//g//g//"
	"|g/g//B/a|/g//////|//^^B//a|//g//B//|g///B//a|//g//B//|g//^C//b|/"
	"/a//a//|BCb/b//a|//g//B//|g/+f/D//c|//b//gA/|g/^^C//C|//C//a//|BC"
	"b////a|//g//g//|g/g//B/a|/g//////|3:^^B//a//|g//A//g/|/^B//a//|g/"
	"/A//B/|b^ 3:C//b//|a//g//+f/|+fG/G/GA/|B/a/g///|B///+f//G|G/G/+f/"
	"G/|^^e//d//|c//b//gA|g/B//a//|g//g//g/|g//B/a/g|//////^^|^^Ga///G"
	"|////////|////////|////////|",
	"2:8,^^^^^^D/|/E//E//E|/d//^^d/|/E//E//E|/E//^^G/|/d//d//d|/^^^^^d"
	"/|/E//E//E|/d/^^^E/|/E//d/+F/|bD^^^^G/|/e//e//e|^^^^^^d/|/E//E//E"
	"|//d///d/|//b/////|^^^^^^D/|/E//E//E|/d//^^d/|/E//E//E|/E//^^G/|/"
	"d//d//d|/^^^^^d/|/E//E//E|/d/^^^E/|/E//d/d/|d/^^^^G/|/e//e//e|^^^"
	"^^^d/|/E//E//E|//d///d/|//b/////|3:D///c//b|//b//b//|D///c//b|//b"
	"//g//|E///d//c|//b//a//|aB/B/BC/|D/c/b///|^^^D//DE|/E/E/d/d|//E/g"
	"//g|//g//d//|^^^^g//E|//E//E//|d///d///|b///////|// 3:Db///C|///b"
	"/// 5:A|64,/|/|/|/|/|/|/|/|",
	"2:8,^^G//+f//|e//e//e/|//G//+f//|e//e//+F/|G/a//g//|+f//+f//+f/|/"
	"/G//+F//|e//e//e/|//B//a//|g//e///d|//c//b//|a//a//a/|+f/G// 2:+F"
	"//|e//e//C/|//b/g/+f/|//G/////|^^G//+f//|e//e//e/|//G//+f//|e//e/"
	"/e/|//a//g//|+f//+f//+f/|//G//+F//|e//e//e/|//B//a//|g//e///d|/  "
	"2:dC//b//|a//a//a/|+f/G//+F//|e//e//C/|//b/g/+f/|//G/////|d//d//d"
	"/|/E//E//d|d//d//E/|/+F//G//b|a//a//a/|/D//D// 3:D|//g/g//D|/d/G/"
	"///|^^b//b//|b//ba/B/|c//B//a/|/g//+f//+f|G//+F//e/|/c//C///|b/g/"
	"+f///|G///////|G///////|C///////|////////|////////|"
};

/*------------------------------------------------------------------------*/

CMusicObject::CMusicObject(int index) {
	assert(index >= 0 && index <= 3);
	CMusicParser parser(DATA[index]);

	// Count how many encoded values there are
	CValuePair r;
	int count = 0;
	while (parser.parse(r))
		++count;
	assert(count > 0);

	_data.resize(count);
	parser.reset();
	for (int idx = 0; idx < count; ++idx)
		parser.parse(_data[idx]);

	_field8 = 0x7FFFFFFF;
	uint val = 0x80000000;

	for (int idx = 0; idx < count; ++idx) {
		CValuePair &vp = _data[idx];
		if (vp._field0 != 0x7FFFFFFF) {
			if (vp._field0 < _field8)
				_field8 = vp._field0;
			if (vp._field0 > val)
				val = vp._field0;
		}
	}

	val -= _field8;
	_fieldC = val;
}

CMusicObject::~CMusicObject() {
	_data.clear();
}

/*------------------------------------------------------------------------*/

#define FETCH_CHAR _currentChar = _str[_strIndex++]

CMusicParser::CMusicParser(const char *str) : _str(str), _strIndex(0),
		_field8(0), _priorChar('A'), _field10(32), _field14(0), _flag(false),
		_field1C(0), _currentChar(' '), _numValue(1) {
}

void CMusicParser::reset() {
	_strIndex = 0;
	_field8 = 0;
	_field10 = 0;
	_field14 = 0;
	_currentChar = ' ';
	_priorChar = 'A';
	_numValue = 1;
	_field1C = 0;
}

bool CMusicParser::parse(CValuePair &r) {
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
			r._field4 += _field10;
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
			r._field0 = 0x7FFFFFFF;
			r._field4 = _field10;
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
				r._field0 = _field8;
			} else if (_currentChar >= 'a' && _currentChar <= 'g') {
				val1 -= val2;
				if (val1 < 0)
					val1 = 12;
				r._field0 = _field8 + val1;
			} else if (_currentChar >= 'A' && _currentChar <= 'G') {
				val1 -= val2;
				if (val1 <= 0)
					val1 = 12;
				r._field0 = _field8 + val1;
			} else {
				flag = false;
			}

			if (flag) {
				r._field4 = _field10;
				_field1C += _field10;
				_field8 = r._field0;
				_priorChar = _currentChar;
				r._field0 += _field14;
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

void CMusicParser::skipSpaces() {
	while (_currentChar && Common::isSpace(_currentChar)) {
		FETCH_CHAR;
	}
}

} // End of namespace Titanic
