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

#include "common/path.h"

namespace Common {

Path::Path(const Path &path) {
	_str = path.rawString();
}

Path::Path(const char *str, char separator) {
	setString(str, separator);
}

Path::Path(const String &str, char separator) {
	setString(str.c_str(), separator);
}

Path::Path(char c, char separator) {
	const char str[] = { c, '\0' };
	setString(str, separator);
}

void Path::setString(const char *str, char separator) {
	for (; *str; str++) {
		if (*str == separator)
			_str += DIR_SEPARATOR;
		else
			_str += *str;
	}
}

String Path::toString(char separator) const {
	String res;
	for (const char *ptr = _str.c_str(); *ptr; ptr++) {
		if (*ptr == DIR_SEPARATOR)
			res += separator;
		else
			res += *ptr;
	}
	return res;
}

bool Path::operator==(const Path &x) const {
	return _str == x.rawString();
}

bool Path::operator!=(const Path &x) const {
	return _str != x.rawString();
}

bool Path::empty() const {
	return _str.empty();
}

Path &Path::operator=(const Path &path) {
	_str = path.rawString();
	return *this;
}

Path &Path::operator=(const char *str) {
	setString(str);
	return *this;
}

Path &Path::operator=(const String &str) {
	setString(str.c_str());
	return *this;
}

Path &Path::operator=(char c) {
	const char str[] = { c, '\0' };
	setString(str);
	return *this;
}

Path &Path::operator+=(const Path &path) {
	_str += path.rawString();
	return *this;
}

Path &Path::operator+=(const char *str) {
	_str += Path(str).rawString();
	return *this;
}

Path &Path::operator+=(const String &str) {
	_str += Path(str).rawString();
	return *this;
}

Path &Path::operator+=(char c) {
	_str += Path(c).rawString();
	return *this;
}

Path operator+(const Path &x, const Path &y) {
	Path temp(x);
	temp += y;
	return temp;
}

Path operator+(const Path &x, const String &y) {
	Path temp(x);
	temp += y;
	return temp;
}

Path operator+(const Path &x, const char *y) {
	Path temp(x);
	temp += y;
	return temp;
}

Path operator+(const Path &x, char y) {
	Path temp(x);
	temp += y;
	return temp;
}

} // End of namespace Common
