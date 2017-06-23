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

#ifndef TITANIC_MUSIC_SONG_H
#define TITANIC_MUSIC_SONG_H

#include "common/scummsys.h"
#include "common/array.h"

namespace Titanic {

struct CValuePair {
	int _data;
	int _length;
	CValuePair() : _data(0), _length(0) {}
};

class CMusicSong {
public:
	Common::Array<CValuePair> _data;
	int _minVal;
	int _range;
public:
	CMusicSong(int index);
	~CMusicSong();

	int size() const { return _data.size(); }

	const CValuePair &operator[](int index) const { return _data[index]; }
};

class CSongParser {
private:
	const char *_str;
	uint _strIndex;
	int _field8;
	char _priorChar;
	int _field10;
	int _field14;
	bool _flag;
	int _field1C;
	char _currentChar;
	int _numValue;
private:
	void skipSpaces();
public:
	CSongParser(const char *str);

	/**
	 * Resets the parser
	 */
	void reset();

	/**
	 * Parses a value from the string
	 */
	bool parse(CValuePair &r);
};

} // End of namespace Titanic

#endif /* TITANIC_MUSIC_SONG_H */
