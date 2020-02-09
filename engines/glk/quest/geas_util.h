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

#ifndef GLK_QUEST_GEAS_UTIL
#define GLK_QUEST_GEAS_UTIL

#include "glk/quest/read_file.h"
#include "glk/quest/streams.h"

namespace Glk {
namespace Quest {

typedef Common::Array<String> vstring;

inline int parse_int(const String &s) {
	return atoi(s.c_str());
}

vstring split_param(String s);
vstring split_f_args(String s);

bool is_param(String s);
String param_contents(String s);

String nonparam(String, String);

String string_geas_block(const GeasBlock &);

bool starts_with(String, String);
bool ends_with(String, String);

String string_int(int i);

String trim_braces(String s);

int eval_int(String s);

String pcase(String s);
String ucase(String s);
String lcase(String s);


template<class T> Common::WriteStream &operator<<(Common::WriteStream &o, Common::Array<T> v) {
	o << "{ '";
	for (uint i = 0; i < v.size(); i ++) {
		o << v[i];
		if (i + 1 < v.size())
			o << "', '";
	}
	o << "' }";
	return o;
}

template <class KEYTYPE, class VALTYPE>
bool has(Common::HashMap<KEYTYPE, VALTYPE, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> m, KEYTYPE key) {
	return m.contains(key);
}

class Logger {
public:
	Logger();
	~Logger();

private:
	class Nullstreambuf : public Common::WriteStream {
		uint32 write(const void *dataPtr, uint32 dataSize) override { return dataSize; }
		int32 pos() const override { return 0; }
	};

	Common::WriteStream *logfilestr_;
//	std::streambuf *cerrbuf_;
	static Nullstreambuf cnull;
};

} // End of namespace Quest
} // End of namespace Glk

#endif
