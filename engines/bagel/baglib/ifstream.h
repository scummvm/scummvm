
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

#include "bagel/boflib/object.h"
#include "bagel/boflib/string.h"

namespace Bagel {

class CBagIfstream : public CBofObject {
private:
	int _lineNumber;
	CBofString _lineString;

	char *_buffer;
	char *_currPtr;
	char *_eof;
	int _length;

public:
	CBagIfstream(char *buffer, int length);

	int getCh();
	int getLine(char *buffer, int length);
	void putBack();
	int get();
	int get(char *buffer, int count, char delimiter = '\n');
	int peek();
	int getCh(char *ch, int count, char delimiter = '\n');
	bool eatWhite();

	bool eof() const { return (_currPtr == _eof); }
	int getLineNumber() const { return _lineNumber; }
	int getSize() const { return _length; }
	const CBofString &getLineString() const { return _lineString; }
};


} // namespace Bagel

#endif
