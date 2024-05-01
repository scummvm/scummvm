
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

#ifndef BAGEL_BAGLIB_IFSTREAM_H
#define BAGEL_BAGLIB_IFSTREAM_H

#include "common/util.h"

#include "bagel/boflib/object.h"
#include "bagel/boflib/string.h"
#include "bagel/boflib/stdinc.h"

namespace Bagel {

class CBagIfstream : public CBofObject {
private:
	int _lineNumber;
	CBofString _lineString;

	char *_buffer;
	char *_curr;
	char *_eof;
	int _length;

public:
	CBagIfstream(char *buffer, int length) {
		_curr = _buffer = buffer;
		_eof = buffer + length;
		_length = length;

		_lineNumber = 1;
		_lineString = "";
	}

	int getCh() {
		int ch = get();
		return ch;
	}

	int getLine(char *buffer, int length) {
		return get(buffer, length, '\n');
	}

	int putBack(char /*ch*/) {
		if (_curr > _buffer)
			_curr--;

		return 0;
	}

	bool eof() const {
		if (_curr == _eof) {
			return true;

		}

		return false;
	}

	int get() {
		if (_curr != _eof)
			return *_curr++;

		return -1;
	}

	int get(char *buffer, int count, char delimiter = '\n') {
		if (!eof()) {
			count = MIN<int>(count, _eof - _curr);

			char *p = (char *)memchr(_curr, delimiter, count);
			if (p != nullptr) {
				count = MIN<int>(count, p - _curr /* + 1*/);
			}
			memcpy(buffer, _curr, count);
			_curr += count;
			Assert(_curr <= _eof);

			return _curr == _eof ? -1 : 0;
		} 

		return -1;
	}

	int peek() {
		if (_curr != _eof)
			return *_curr;

		return -1;
	}

	int getCh(char *ch, int count, char delimiter = '\n') {
		get(ch, count, delimiter);

		if (delimiter == '\n') {
			_lineNumber++;
			_lineString = "";
		}

		return 0;
	}

	int eatWhite() {
		int ch = peek();
		while ((ch == ' ') || (ch == '\t') || (ch == '\r') || (ch == '\n')) {

			if (ch == '\n') {
				_lineNumber++;
				_lineString = "";
			}
			getCh();
			ch = peek();
		}
		if (ch == -1)
			return ch;

		return 0;
	}

	int getLineNumber() const {
		return _lineNumber;
	}
	int getSize() const {
		return _length;
	}
	const CBofString &getLineString() {
		return _lineString;
	}
};


} // namespace Bagel

#endif
