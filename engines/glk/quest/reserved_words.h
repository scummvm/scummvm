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

#ifndef GLK_QUEST_RESERVED_WORDS
#define GLK_QUEST_RESERVED_WORDS

#include "glk/quest/string.h"
#include "common/hashmap.h"
#include "common/stream.h"

namespace Glk {
namespace Quest {

class reserved_words {
private:
	StringBoolMap _data;

public:
	/**
	 * Constructor
	 */
	reserved_words(const char *c, ...) {
		va_list ap;
		va_start(ap, c);

		while (c != nullptr) {
			_data[String(c)] = true;
			c = va_arg(ap, const char *);
		}
		va_end(ap);
	}

	/**
	 * Returns true if the passed string is a reserved word
	 */
	bool operator[](const String &s) const {
		return has(s);
	}

	/**
	 * Returns true if the passed string is a reserved word
	 */
	bool has(const String &s) const {
		return _data.contains(s) && _data[s];
	}

	/**
	 * Dumps the list of reserved words to the passed output stream
	 */
	void dump(Common::WriteStream &o) const {
		o.writeString("RW {");

		for (StringBoolMap::iterator i = _data.begin(); i != _data.end(); ++i) {
			if (i != _data.begin())
				o.writeString(", ");
			o.writeString((*i)._key);
		}

		o.writeString("}");
	}
};

} // End of namespace Quest
} // End of namespace Glk

#endif
