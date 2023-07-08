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

#include "common/hash-str.h"
#include "common/list.h"
#include "common/punycode.h"

const char ESCAPER = '/';
const char ESCAPE_SLASH = '+';
const char ESCAPE_SEPARATOR = '/';
const char *DIR_SEPARATOR = "//";
const char *SLASH_ESCAPED = "/+";

namespace Common {

Path::Path(const Path &path) {
	_str = path._str;
}

Path::Path(const char *str, char separator) {
	set(str, separator);
}

Path::Path(const String &str, char separator) {
	set(str.c_str(), separator);
}

String Path::toString(char separator) const {
	String res;
	for (uint i = 0; i < _str.size(); i++) {
		if (_str[i] == ESCAPER) {
			i++;
			if (_str[i] == ESCAPE_SLASH)
				res += '/';
			else if (_str[i] == ESCAPE_SEPARATOR)
				res += separator;
			else
				error("Path::toString(): Malformed Common::Path. '%c' unexpected after '/'", _str[i]);
		} else {
			res += _str[i];
		}
	}
	return res;
}

size_t Path::findLastSeparator(size_t last) const {
	if (_str.size() < 2)
		return String::npos;

	size_t res = String::npos;
	if (last == String::npos || last > _str.size())
		last = _str.size();

	for (uint i = 0; i < last - 1; i++) {
		if (_str[i] == ESCAPER && _str[i + 1] == ESCAPE_SEPARATOR) {
			res = i;
		}
	}

	return res;
}

Path Path::getParent() const {
	if (_str.size() < 2)
		return Path();
	// ignore potential trailing separator
	size_t separatorPos = findLastSeparator(_str.size() - 1);
	if (separatorPos == String::npos)
		return Path();
	Path ret;
	ret._str = _str.substr(0, separatorPos + 2);
	return ret;
}

Path Path::getLastComponent() const {
	if (_str.size() < 2)
		return *this;
	// ignore potential trailing separator
	size_t separatorPos = findLastSeparator(_str.size() - 1);
	if (separatorPos == String::npos)
		return *this;
	Path ret;
	ret._str = _str.substr(separatorPos + 2);
	return ret;
}

static String escapePath(const String& in) {
	String ret;
	for (uint i = 0; i < in.size(); i++) {
		if (in[i] == '/')
			ret += SLASH_ESCAPED;
		else
			ret += in[i];
	}
	return ret;
}

Path Path::appendComponent(const String &x) const {
	if (x.empty())
		return *this;
	String str = _str;
	size_t lastSep = findLastSeparator();
	if (!str.empty() && (lastSep == String::npos || lastSep != str.size() - 2))
		str += DIR_SEPARATOR;

	str += escapePath(x);

	Path ret;
	ret._str = str;
	return ret;
}

bool Path::operator==(const Path &x) const {
	return _str == x._str;
}

bool Path::operator!=(const Path &x) const {
	return _str != x._str;
}

bool Path::empty() const {
	return _str.empty();
}

Path &Path::operator=(const Path &path) {
	_str = path._str;
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
	_str += x._str;
	return *this;
}

Path &Path::appendInPlace(const String &str, char separator) {
	appendInPlace(str.c_str(), separator);
	return *this;
}

Path &Path::appendInPlace(const char *str, char separator) {
	for (; *str; str++) {
		if (*str == separator)
			_str += DIR_SEPARATOR;
		else if (*str == '/') // Order matters as / may be the separator and often is.
			_str += SLASH_ESCAPED;
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

	size_t lastSep = findLastSeparator();
	if (!_str.empty() && (lastSep == String::npos || lastSep != _str.size() - 2) && !x._str.hasPrefix(DIR_SEPARATOR))
		_str += DIR_SEPARATOR;

	_str += x._str;

	return *this;
}

Path &Path::joinInPlace(const String &str, char separator) {
	return joinInPlace(str.c_str(), separator);
}

Path &Path::joinInPlace(const char *str, char separator) {
	if (*str == '\0')
		return *this;

	size_t lastSep = findLastSeparator();
	if (!_str.empty() && (lastSep == String::npos || lastSep != _str.size() - 2) && *str != separator)
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
	temp.joinInPlace(str, separator);
	return temp;
}

Path Path::normalize() const {
	if (empty()) {
		return Path();
	}

	Common::String::const_iterator cur = _str.begin();
	const Common::String::const_iterator end = _str.end();

	Common::Path result;

	// If there is a leading slash, preserve that:
	if (cur != end &&
	    *cur == ESCAPER &&
	    *(cur + 1) == ESCAPE_SEPARATOR) {
		result._str += DIR_SEPARATOR;
		cur += 2;
		// Skip over multiple leading slashes, so "//" equals "/"
		while (cur != end && *cur == ESCAPER && *(cur + 1) == ESCAPE_SEPARATOR)
			cur += 2;
	}

	// Scan for path components till the end of the String
	List<String> comps;
	while (cur != end) {
		Common::String::const_iterator start = cur;

		// Scan till the next path separator resp. the end of the string
		while (!(*cur == ESCAPER && *(cur + 1) == ESCAPE_SEPARATOR) && cur != end)
			cur++;

		const String component(start, cur);

		if (component.empty() || component == ".") {
			// Skip empty components and dot components
		} else if (!comps.empty() && component == ".." && comps.back() != "..") {
			// If stack is non-empty and top is not "..", remove top
			comps.pop_back();
		} else {
			// Add the component to the stack
			comps.push_back(component);
		}

		// Skip over separator chars
		while (cur != end && *cur == ESCAPER && *(cur + 1) == ESCAPE_SEPARATOR)
			cur += 2;
	}

	// Finally, assemble all components back into a path
	while (!comps.empty()) {
		result._str += comps.front();
		comps.pop_front();
		if (!comps.empty())
			result._str += DIR_SEPARATOR;
	}

	return result;
}

StringArray Path::splitComponents() const {
	StringArray res;
	String cur;
	for (uint i = 0; i < _str.size(); i++) {
		if (_str[i] == ESCAPER) {
			i++;
			if (_str[i] == ESCAPE_SLASH)
				cur += '/';
			else if (_str[i] == ESCAPE_SEPARATOR) {
				res.push_back(cur);
				cur = "";
			} else {
				error("Path::splitComponents(): Malformed Common::Path. '%c' unexpected after '/'", _str[i]);
			}
		} else
			cur += _str[i];
	}

	res.push_back(cur);

	return res;
}

Path Path::punycodeDecode() const {
	StringArray c = splitComponents();
	String res;

	for (uint i = 0; i < c.size(); i++) {
		res += escapePath(punycode_decodefilename(c[i]));
		if (i + 1 < c.size())
			res += DIR_SEPARATOR;
	}

	Path ret;
	ret._str = res;
	return ret;
}

Path Path::joinComponents(const StringArray& c) {
	String res;

	for (uint i = 0; i < c.size(); i++) {
		res += escapePath(c[i]);
		if (i + 1 < c.size())
			res += DIR_SEPARATOR;
	}

	Path ret;
	ret._str = res;
	return ret;
}

// See getIdentifierString() for more details.
// This does the same but for a single path component and is used by
// getIdentifierString().
static String getIdentifierComponent(const String& in) {
	String part = punycode_decodefilename(in);
	String res = "";
	for (uint j = 0; j < part.size(); j++)
		if (part[j] == '/')
			res += ':';
		else
			res += part[j];
	return res;
}

// For a path creates a string with following property:
// if 2 files have the same case-insensitive
// identifier string then and only then we treat them as
// effectively the same file. For this there are 2
// transformations we need to do:
// * decode punycode
// * Replace / with : in path components so a path from
// HFS(+) image will end up with : independently of how
// it was dumped or copied from
String Path::getIdentifierString() const {
	StringArray c = splitComponents();
	String res;

	for (uint i = 0; i < c.size(); i++) {
		res += getIdentifierComponent(c[i]);
		if (i + 1 < c.size())
			res += DIR_SEPARATOR;
	}

	return res;
}

Path Path::punycodeEncode() const {
	StringArray c = splitComponents();
	String res;

	for (uint i = 0; i < c.size(); i++) {
		res += escapePath(punycode_encodefilename(c[i]));
		if (i + 1 < c.size())
			res += DIR_SEPARATOR;
	}

	Path ret;
	ret._str = res;
	return ret;
}

bool Path::matchPattern(const Path& pattern) const {
	StringArray c = splitComponents();
	StringArray cpat = pattern.splitComponents();

	// Prevent wildcards from matching the directory separator.
	if (c.size() != cpat.size())
		return false;

	for (uint i = 0; i < c.size(); i++) {
		if (!getIdentifierComponent(c[i]).matchString(getIdentifierComponent(cpat[i]), true))
			return false;
	}

	return true;
}

bool Path::IgnoreCaseAndMac_EqualsTo::operator()(const Path& x, const Path& y) const {
	return x.getIdentifierString().equalsIgnoreCase(y.getIdentifierString());
}

uint Path::IgnoreCaseAndMac_Hash::operator()(const Path& x) const {
	return hashit_lower(x.getIdentifierString().c_str());
}

} // End of namespace Common
