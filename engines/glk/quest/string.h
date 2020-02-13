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

#ifndef GLK_QUEST_STRING
#define GLK_QUEST_STRING

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/str.h"

namespace Glk {
namespace Quest {

class String;

typedef Common::HashMap<String, String, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> StringMap;
typedef Common::HashMap<String, bool, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> StringBoolMap;
typedef Common::HashMap<String, Common::Array<int>, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> StringArrayIntMap;
typedef Common::HashMap<String, Common::Array<String>, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> StringArrayStringMap;

class String : public Common::String {
public:
	String() : Common::String() {}
	String(const char *str) : Common::String(str) {}
	String(const char *str, uint32 len) : Common::String(str, len) {}
	String(const char *beginP, const char *endP) : Common::String(beginP, endP) {}
	String(const String &str) : Common::String(str) {}
	explicit String(char c) : Common::String(c) {}

	char &operator[](int idx) {
		assert(_str && idx >= 0 && idx < (int)_size);
		return _str[idx];
	}

	inline uint length() const {
		return size();
	}

	String substr(size_t pos, size_t len) const {
		return String(c_str() + pos, c_str() + pos + len);
	}
	String substr(size_t pos) const {
		return String(c_str() + pos);
	}

	int find(char c, int pos = 0) const {
		const char *p = strchr(c_str() + pos, c);
		return p ? p - c_str() : -1;
	}

	int find(const Common::String &s, int pos = 0) const {
		const char *p = strstr(c_str() + pos, s.c_str());
		return p ? p - c_str() : -1;
	}

	int rfind(char c) const {
		const char *p = strrchr(c_str(), c);
		return p ? p - c_str() : -1;
	}

	String trim() const {
		String result = *this;
		static_cast<Common::String>(result).trim();
		return result;
	}
};

// Append two strings to form a new (temp) string
String operator+(const String &x, const String &y);

String operator+(const char *x, const String &y);
String operator+(const String &x, const char *y);

String operator+(const String &x, char y);
String operator+(char x, const String &y);


bool c_equal_i(char ch1, char ch2);
size_t ci_find(const String &str1, const String &str2);
bool   ci_equal(const String &str1, const String &str2);
bool   ci_less_eq(const String &str1, const String &str2);
bool   ci_less(const String &str1, const String &str2);
bool   ci_notequal(const String &str1, const String &str2);
bool   ci_gt_eq(const String &str1, const String &str2);
bool   ci_gt(const String &str1, const String &str2);

class CI_EQUAL {
public:
	bool operator()(const String &str1, const String &str2) {
		return ci_equal(str1, str2);
	}
};

class CI_LESS_EQ {
public:
	bool operator()(const String &str1, const String &str2) {
		return ci_less_eq(str1, str2);
	}
};

class CI_LESS {
public:
	bool operator()(const String &str1, const String &str2) {
		return ci_less(str1, str2);
	}
};

extern CI_EQUAL   ci_equal_obj;
extern CI_LESS    ci_less_obj;
extern CI_LESS_EQ ci_less_eq_obj;

} // End of namespace Quest
} // End of namespace Glk

#endif
