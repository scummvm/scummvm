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

#ifndef TITANIC_TT_QUOTES_H
#define TITANIC_TT_QUOTES_H

#include "common/scummsys.h"
#include "common/stream.h"
#include "titanic/support/string.h"

namespace Titanic {

class TTquotes {
	struct TTquotesEntry {
		byte _tagIndex, _maxSize;
		const char *_strP;
		TTquotesEntry() : _tagIndex(0), _maxSize(0), _strP(nullptr) {}
	};
	struct TTquotesLetter {
		Common::Array<TTquotesEntry> _entries;
		int _field4;
		int _field8;

		TTquotesLetter() : _field4(0), _field8(0) {}
	};
private:
	TTquotesLetter _alphabet[26];
	uint _tags[256];
	char *_dataP;
	size_t _dataSize;
	int _field544;
private:
	/**
	 * Test whether a substring contains one of the quotes,
	 * and if so, returns the 4-character tag Id associated with it
	 */
	int find(const char *startP, const char *endP) const;
public:
	bool _loaded;
public:
	TTquotes();
	~TTquotes();

	/**
	 * Load quotes data resource
	 */
	void load();

	/**
	 * Test whether a passed string contains one of the quotes,
	 * and if so, returns the 4-character tag Id associated with it
	 */
	int find(const char *str) const;
};

} // End of namespace Titanic

#endif /* TITANIC_TT_QUOTES_H */
