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

#ifndef TESORO_CORE_STDIOTEXT_H
#define TESORO_CORE_STDIOTEXT_H

#include "common/endian.h"
#include "common/scummsys.h"

namespace Tesoro {

/** Text reader with the parts of C stdio the game's text files are read with. */
class StdioText {
public:
	StdioText(const byte *data, uint32 size) : _data(data), _size(size) {}

	/** Next byte, or -1 at the end (which also raises the end flag). */
	int get() {
		if (_pushed >= 0) {
			const int c = _pushed;
			_pushed = -1;
			return c;
		}
		if (_pos >= _size) {
			_eof = true;
			return -1;
		}
		return _data[_pos++];
	}
	void unget(int c) {
		if (c == -1)
			return;
		_pushed = c & 0xff;
		_eof = false;
	}
	bool eof() const { return _eof; }
	/** Bytes read so far. */
	uint32 consumed() const { return _pos - (_pushed >= 0 ? 1 : 0); }
	/** Goes back to a position counted from the start of the text. */
	void seek(uint32 pos) {
		_pos = pos;
		_pushed = -1;
		_eof = false;
	}

	/** The subset of scanf the texts use: %s, %i and %[set] conversions, white space and literals. */
	void scan(const char *fmt, void *a0 = nullptr, void *a1 = nullptr, void *a2 = nullptr);

private:
	static bool isSpace(int c) { return c == ' ' || (c >= 9 && c <= 13); }
	void skipSpace();
	bool readInt(int32 &value);
	uint readSet(const char *&fmt, char *dst);

	const byte *_data;
	uint32 _size;
	uint32 _pos = 0;
	int _pushed = -1;
	bool _eof = false;
};

} // End of namespace Tesoro

#endif
