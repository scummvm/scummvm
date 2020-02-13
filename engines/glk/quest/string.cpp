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

#include "glk/quest/string.h"

namespace Glk {
namespace Quest {

CI_EQUAL   ci_equal_obj;
CI_LESS    ci_less_obj;
CI_LESS_EQ ci_less_eq_obj;

String operator+(const String &x, const String &y) {
	String temp(x);
	temp += y;
	return temp;
}

String operator+(const char *x, const String &y) {
	String temp(x);
	temp += y;
	return temp;
}

String operator+(const String &x, const char *y) {
	String temp(x);
	temp += y;
	return temp;
}

String operator+(char x, const String &y) {
	String temp(x);
	temp += y;
	return temp;
}

String operator+(const String &x, char y) {
	String temp(x);
	temp += y;
	return temp;
}

/*--------------------------------------------------------------------------*/

// Code for testing case insensitively by John Harrison

bool c_equal_i(char ch1, char ch2) {
	return tolower((unsigned char)ch1) == tolower((unsigned char)ch2);
}

size_t ci_find(const String &str1, const String &str2) {
	const char *pos = strstr(str1.c_str(), str2.c_str());
	return !pos ? String::npos : pos - str1.c_str();
}

static int my_stricmp(const String &s1, const String &s2) {
	return s1.compareToIgnoreCase(s2);
}

bool ci_equal(const String &str1, const String &str2) {
	return my_stricmp(str1, str2) == 0;
}
bool ci_less_eq(const String &str1, const String &str2) {
	return my_stricmp(str1, str2) <= 0;
}
bool ci_less(const String &str1, const String &str2) {
	return my_stricmp(str1, str2) < 0;
}
bool ci_notequal(const String &str1, const String &str2) {
	return !ci_equal(str1, str2);
}
bool ci_gt_eq(const String &str1, const String &str2) {
	return my_stricmp(str1, str2) >= 0;
}
bool ci_gt(const String &str1, const String &str2) {
	return my_stricmp(str1, str2) > 0;
}

} // End of namespace Quest
} // End of namespace Glk
