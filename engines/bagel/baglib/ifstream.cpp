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

#include "common/util.h"

#include "bagel/baglib/ifstream.h"
#include "bagel/boflib/stdinc.h"

namespace Bagel {

CBagIfstream::CBagIfstream(char *buffer, int length) {
	_currPtr = _buffer = buffer;
	_eof = buffer + length;
	_length = length;

	_lineNumber = 1;
	_lineString = "";
}

int CBagIfstream::getCh() {
	const int ch = get();
	return ch;
}

int CBagIfstream::getLine(char *buffer, int length) {
	return get(buffer, length, '\n');
}

void CBagIfstream::putBack() {
	if (_currPtr > _buffer)
		_currPtr--;
}

int CBagIfstream::get() {
	if (_currPtr != _eof)
		return *_currPtr++;

	return -1;
}

int CBagIfstream::get(char *buffer, int count, char delimiter) {
	if (!eof()) {
		count = MIN<int>(count, _eof - _currPtr);

		char *p = (char *)memchr(_currPtr, delimiter, count);
		if (p != nullptr) {
			count = MIN<int>(count, p - _currPtr /* + 1*/);
		}
		memcpy(buffer, _currPtr, count);
		_currPtr += count;
		assert(_currPtr <= _eof);

		return _currPtr == _eof ? -1 : 0;
	}

	return -1;
}

int CBagIfstream::peek() {
	if (_currPtr != _eof)
		return *_currPtr;

	return -1;
}

int CBagIfstream::getCh(char *ch, int count, char delimiter) {
	get(ch, count, delimiter);

	if (delimiter == '\n') {
		_lineNumber++;
		_lineString = "";
	}

	return 0;
}

bool CBagIfstream::eatWhite() {
	int ch = peek();
	while ((ch == ' ') || (ch == '\t') || (ch == '\r') || (ch == '\n')) {

		if (ch == '\n') {
			_lineNumber++;
			_lineString = "";
		}
		getCh();
		ch = peek();
	}

	return (ch != -1);
}


} // namespace Bagel
