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

#include "common/path.h"
#include "common/tokenizer.h"
#include "common/punycode.h"

namespace Common {

Path::Path(const Path &path) {
	_str = path.rawString();
}

Path::Path(const char *str, char separator) {
	set(str, separator);
}

Path::Path(const String &str, char separator) {
	if (separator == DIR_SEPARATOR)
		_str = str;
	else
		set(str.c_str(), separator);
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

Path Path::getParent() const {
	if (_str.size() < 2)
		return Path();
	size_t separatorPos = _str.findLastOf(DIR_SEPARATOR, _str.size() - 2);
	if (separatorPos == String::npos)
		return Path();
	return Path(_str.substr(0, separatorPos + 1), DIR_SEPARATOR);
}

Path Path::getLastComponent() const {
	if (_str.size() < 2)
		return *this;
	size_t separatorPos = _str.findLastOf(DIR_SEPARATOR, _str.size() - 2);
	if (separatorPos == String::npos)
		return *this;
	return Path(_str.substr(separatorPos + 1), DIR_SEPARATOR);
}

Path Path::appendComponent(const String &x) const {
	if (x.empty())
		return *this;
	String str = _str;
	if (!str.empty() && str.lastChar() != DIR_SEPARATOR)
		str += DIR_SEPARATOR;
	str += x;
	return Path(str, DIR_SEPARATOR);
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
	set(str);
	return *this;
}

Path &Path::operator=(const String &str) {
	set(str.c_str());
	return *this;
}

void Path::set(const char *str, char separator) {
	_str.clear();
	appendInPlace(str, separator);
}

Path &Path::appendInPlace(const Path &x) {
	_str += x.rawString();
	return *this;
}

Path &Path::appendInPlace(const String &str, char separator) {
	if (separator == DIR_SEPARATOR)
		_str += str;
	else
		appendInPlace(str.c_str(), separator);
	return *this;
}

Path &Path::appendInPlace(const char *str, char separator) {
	for (; *str; str++) {
		if (*str == separator)
			_str += DIR_SEPARATOR;
		else
			_str += *str;
	}
	return *this;
}

Path Path::append(const Path &x) const {
	Path temp(*this);
	temp.appendInPlace(x);
	return temp;
}

Path Path::append(const String &str, char separator) const {
	return append(str.c_str(), separator);
}

Path Path::append(const char *str, char separator) const {
	Path temp(*this);
	temp.appendInPlace(str, separator);
	return temp;
}

Path &Path::joinInPlace(const Path &x) {
	if (x.empty())
		return *this;

	if (!_str.empty() && _str.lastChar() != DIR_SEPARATOR && x.rawString().firstChar() != DIR_SEPARATOR)
		_str += DIR_SEPARATOR;

	_str += x.rawString();

	return *this;
}

Path &Path::joinInPlace(const String &str, char separator) {
	return joinInPlace(str.c_str(), separator);
}

Path &Path::joinInPlace(const char *str, char separator) {
	if (*str == '\0')
		return *this;

	if (!_str.empty() && _str.lastChar() != DIR_SEPARATOR && *str != separator)
		_str += DIR_SEPARATOR;

	appendInPlace(str, separator);

	return *this;
}

Path Path::join(const Path &x) const {
	Path temp(*this);
	temp.joinInPlace(x);
	return temp;
}

Path Path::join(const String &str, char separator) const {
	return join(str.c_str(), separator);
}

Path Path::join(const char *str, char separator) const {
	Path temp(*this);
	temp.joinInPlace(str, DIR_SEPARATOR);
	return temp;
}

Path Path::punycodeDecode() const {
	StringTokenizer tok(rawString(), String(DIR_SEPARATOR));
	String res;

	while (!tok.empty()) {
		res += punycode_decodefilename(tok.nextToken());
		if (!tok.empty())
			res += DIR_SEPARATOR;
	}

	return Path(res, DIR_SEPARATOR);
}

Path Path::punycodeEncode() const {
	StringTokenizer tok(rawString(), String(DIR_SEPARATOR));
	String res;

	while (!tok.empty()) {
		String part = tok.nextToken();
		if (punycode_needEncode(part))
			res += punycode_encodefilename(part);
		else
			res += part;

		if (!tok.empty())
			res += DIR_SEPARATOR;
	}

	return Path(res, DIR_SEPARATOR);
}

} // End of namespace Common
