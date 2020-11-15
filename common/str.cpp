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

#include "common/hash-str.h"
#include "common/list.h"
#include "common/memorypool.h"
#include "common/str.h"
#include "common/util.h"
#include "common/mutex.h"

namespace Common {

String::String(char c)
	: BaseString<char>() {

	_storage[0] = c;
	_storage[1] = 0;

	_size = (c == 0) ? 0 : 1;
}

#ifndef SCUMMVM_UTIL
String::String(const U32String &str, Common::CodePage page)
	: BaseString<char>() {
	_storage[0] = 0;
	*this = String(str.encode(page));
}
#endif

String &String::operator=(const char *str) {
	assign(str);
	return *this;
}

String &String::operator=(const String &str) {
	assign(str);
	return *this;
}

String &String::operator=(char c) {
	assign(c);
	return *this;
}

String &String::operator+=(const char *str) {
	assignAppend(str);
	return *this;
}

String &String::operator+=(const String &str) {
	assignAppend(str);
	return *this;
}

String &String::operator+=(char c) {
	assignAppend(c);
	return *this;
}

bool String::hasPrefix(const String &x) const {
	return hasPrefix(x.c_str());
}

bool String::hasPrefix(const char *x) const {
	assert(x != nullptr);
	// Compare x with the start of _str.
	const char *y = c_str();
	while (*x && *x == *y) {
		++x;
		++y;
	}
	// It's a prefix, if and only if all letters in x are 'used up' before
	// _str ends.
	return *x == 0;
}

bool String::hasPrefixIgnoreCase(const String &x) const {
	return hasPrefixIgnoreCase(x.c_str());
}

bool String::hasPrefixIgnoreCase(const char *x) const {
	assert(x != nullptr);
	// Compare x with the start of _str.
	const char *y = c_str();
	while (*x && tolower(*x) == tolower(*y)) {
		++x;
		++y;
	}
	// It's a prefix, if and only if all letters in x are 'used up' before
	// _str ends.
	return *x == 0;
}

bool String::hasSuffix(const String &x) const {
	return hasSuffix(x.c_str());
}

bool String::hasSuffix(const char *x) const {
	assert(x != nullptr);
	// Compare x with the end of _str.
	const uint32 x_size = strlen(x);
	if (x_size > _size)
		return false;
	const char *y = c_str() + _size - x_size;
	while (*x && *x == *y) {
		++x;
		++y;
	}
	// It's a suffix, if and only if all letters in x are 'used up' before
	// _str ends.
	return *x == 0;
}

bool String::hasSuffixIgnoreCase(const String &x) const {
	return hasSuffixIgnoreCase(x.c_str());
}

bool String::hasSuffixIgnoreCase(const char *x) const {
	assert(x != nullptr);
	// Compare x with the end of _str.
	const uint32 x_size = strlen(x);
	if (x_size > _size)
		return false;
	const char *y = c_str() + _size - x_size;
	while (*x && tolower(*x) == tolower(*y)) {
		++x;
		++y;
	}
	// It's a suffix, if and only if all letters in x are 'used up' before
	// _str ends.
	return *x == 0;
}

bool String::contains(const String &x) const {
	return strstr(c_str(), x.c_str()) != nullptr;
}

bool String::contains(const char *x) const {
	assert(x != nullptr);
	return strstr(c_str(), x) != nullptr;
}

bool String::contains(char x) const {
	return strchr(c_str(), x) != nullptr;
}

bool String::contains(uint32 x) const {
	for (String::const_iterator itr = begin(); itr != end(); itr++) {
		if (uint32(*itr) == x) {
			return true;
		}
	}
	return false;
}

#ifdef USE_CXX11
bool String::contains(char32_t x) const {
	return contains((uint32)x);
}
#endif

#ifndef SCUMMVM_UTIL

bool String::matchString(const char *pat, bool ignoreCase, bool pathMode) const {
	return Common::matchString(c_str(), pat, ignoreCase, pathMode);
}

bool String::matchString(const String &pat, bool ignoreCase, bool pathMode) const {
	return Common::matchString(c_str(), pat.c_str(), ignoreCase, pathMode);
}

#endif

void String::replace(uint32 pos, uint32 count, const String &str) {
	replace(pos, count, str, 0, str._size);
}

void String::replace(uint32 pos, uint32 count, const char *str) {
	replace(pos, count, str, 0, strlen(str));
}

void String::replace(iterator begin_, iterator end_, const String &str) {
	replace(begin_ - _str, end_ - begin_, str._str, 0, str._size);
}

void String::replace(iterator begin_, iterator end_, const char *str) {
	replace(begin_ - _str, end_ - begin_, str, 0, strlen(str));
}

void String::replace(uint32 posOri, uint32 countOri, const String &str,
					 uint32 posDest, uint32 countDest) {
	replace(posOri, countOri, str._str, posDest, countDest);
}

void String::replace(uint32 posOri, uint32 countOri, const char *str,
					 uint32 posDest, uint32 countDest) {

	// Prepare string for the replaced text.
	if (countOri < countDest) {
		uint32 offset = countDest - countOri; ///< Offset to copy the characters
		uint32 newSize = _size + offset;

		ensureCapacity(newSize, true);

		_size = newSize;

		// Push the old characters to the end of the string
		for (uint32 i = _size; i >= posOri + countDest; i--)
			_str[i] = _str[i - offset];

	} else if (countOri > countDest){
		uint32 offset = countOri - countDest; ///< Number of positions that we have to pull back

		makeUnique();

		// Pull the remainder string back
		for (uint32 i = posOri + countDest; i + offset <= _size; i++)
			_str[i] = _str[i + offset];

		_size -= offset;
	} else {
		makeUnique();
	}

	// Copy the replaced part of the string
	for (uint32 i = 0; i < countDest; i++)
		_str[posOri + i] = str[posDest + i];

}

// static
String String::format(const char *fmt, ...) {
	String output;

	va_list va;
	va_start(va, fmt);
	output = String::vformat(fmt, va);
	va_end(va);

	return output;
}

// static
String String::vformat(const char *fmt, va_list args) {
	String output;
	assert(output.isStorageIntern());

	va_list va;
	scumm_va_copy(va, args);
	int len = vsnprintf(output._str, _builtinCapacity, fmt, va);
	va_end(va);

	if (len == -1 || len == _builtinCapacity - 1) {
		// MSVC and IRIX don't return the size the full string would take up.
		// MSVC returns -1, IRIX returns the number of characters actually written,
		// which is at the most the size of the buffer minus one, as the string is
		// truncated to fit.

		// We assume MSVC failed to output the correct, null-terminated string
		// if the return value is either -1 or size.
		// For IRIX, because we lack a better mechanism, we assume failure
		// if the return value equals size - 1.
		// The downside to this is that whenever we try to format a string where the
		// size is 1 below the built-in capacity, the size is needlessly increased.

		// Try increasing the size of the string until it fits.
		int size = _builtinCapacity;
		do {
			size *= 2;
			output.ensureCapacity(size - 1, false);
			assert(!output.isStorageIntern());
			size = output._extern._capacity;

			scumm_va_copy(va, args);
			len = vsnprintf(output._str, size, fmt, va);
			va_end(va);
		} while (len == -1 || len >= size - 1);
		output._size = len;
	} else if (len < (int)_builtinCapacity) {
		// vsnprintf succeeded
		output._size = len;
	} else {
		// vsnprintf didn't have enough space, so grow buffer
		output.ensureCapacity(len, false);
		scumm_va_copy(va, args);
		int len2 = vsnprintf(output._str, len + 1, fmt, va);
		va_end(va);
		assert(len == len2);
		output._size = len2;
	}

	return output;
}

size_t String::rfind(const char *s) const {
	int sLen = strlen(s);

	for (int idx = (int)_size - sLen; idx >= 0; --idx) {
		if (!strncmp(_str + idx, s, sLen))
			return idx;
	}

	return npos;
}

size_t String::rfind(char c, size_t pos) const {
	for (int idx = MIN((int)_size - 1, (int)pos); idx >= 0; --idx) {
		if ((*this)[idx] == c)
			return idx;
	}

	return npos;
}

size_t String::findFirstOf(char c, size_t pos) const {
	const char *strP = (pos >= _size) ? 0 : strchr(_str + pos, c);
	return strP ? strP - _str : npos;
}

size_t String::findFirstOf(const char *chars, size_t pos) const {
	for (uint idx = pos; idx < _size; ++idx) {
		if (strchr(chars, (*this)[idx]))
			return idx;
	}

	return npos;
}

size_t String::findLastOf(char c, size_t pos) const {
	int start = (pos == npos) ? (int)_size - 1 : MIN((int)_size - 1, (int)pos);
	for (int idx = start; idx >= 0; --idx) {
		if ((*this)[idx] == c)
			return idx;
	}

	return npos;
}

size_t String::findLastOf(const char *chars, size_t pos) const {
	int start = (pos == npos) ? (int)_size - 1 : MIN((int)_size - 1, (int)pos);
	for (int idx = start; idx >= 0; --idx) {
		if (strchr(chars, (*this)[idx]))
			return idx;
	}

	return npos;
}

size_t String::findFirstNotOf(char c, size_t pos) const {
	for (uint idx = pos; idx < _size; ++idx) {
		if ((*this)[idx] != c)
			return idx;
	}

	return npos;
}

size_t String::findFirstNotOf(const char *chars, size_t pos) const {
	for (uint idx = pos; idx < _size; ++idx) {
		if (!strchr(chars, (*this)[idx]))
			return idx;
	}

	return npos;
}

size_t String::findLastNotOf(char c) const {
	for (int idx = (int)_size - 1; idx >= 0; --idx) {
		if ((*this)[idx] != c)
			return idx;
	}

	return npos;
}

size_t String::findLastNotOf(const char *chars) const {
	for (int idx = (int)_size - 1; idx >= 0; --idx) {
		if (!strchr(chars, (*this)[idx]))
			return idx;
	}

	return npos;
}

String String::substr(size_t pos, size_t len) const {
	if (pos >= _size)
		return String();
	else if (len == npos)
		return String(_str + pos);
	else
		return String(_str + pos, MIN((size_t)_size - pos, len));
}

#pragma mark -

bool operator==(const char* y, const String &x) {
	return (x == y);
}

bool operator!=(const char* y, const String &x) {
	return x != y;
}

#pragma mark -

bool String::equalsIgnoreCase(const String &x) const {
	return (0 == compareToIgnoreCase(x));
}

bool String::equalsIgnoreCase(const char *x) const {
	assert(x != nullptr);
	return (0 == compareToIgnoreCase(x));
}

int String::compareToIgnoreCase(const String &x) const {
	return compareToIgnoreCase(x.c_str());
}

int String::compareToIgnoreCase(const char *x) const {
	assert(x != nullptr);
	return scumm_stricmp(c_str(), x);
}

int String::compareDictionary(const String &x) const {
	return compareDictionary(x.c_str());
}

int String::compareDictionary(const char *x) const {
	assert(x != nullptr);
	return scumm_compareDictionary(c_str(), x);
}

#pragma mark -

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

#ifndef SCUMMVM_UTIL

char *ltrim(char *t) {
	while (isSpace(*t))
		t++;
	return t;
}

char *rtrim(char *t) {
	int l = strlen(t) - 1;
	while (l >= 0 && isSpace(t[l]))
		t[l--] = 0;
	return t;
}

char *trim(char *t) {
	return rtrim(ltrim(t));
}

#endif

String lastPathComponent(const String &path, const char sep) {
	const char *str = path.c_str();
	const char *last = str + path.size();

	// Skip over trailing slashes
	while (last > str && *(last - 1) == sep)
		--last;

	// Path consisted of only slashes -> return empty string
	if (last == str)
		return String();

	// Now scan the whole component
	const char *first = last - 1;
	while (first > str && *first != sep)
		--first;

	if (*first == sep)
		first++;

	return String(first, last);
}

String normalizePath(const String &path, const char sep) {
	if (path.empty())
		return path;

	const char *cur = path.c_str();
	String result;

	// If there is a leading slash, preserve that:
	if (*cur == sep) {
		result += sep;
		// Skip over multiple leading slashes, so "//" equals "/"
		while (*cur == sep)
			++cur;
	}

	// Scan for path components till the end of the String
	List<String> comps;
	while (*cur != 0) {
		const char *start = cur;

		// Scan till the next path separator resp. the end of the string
		while (*cur != sep && *cur != 0)
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
		while (*cur == sep)
			cur++;
	}

	// Finally, assemble all components back into a path
	while (!comps.empty()) {
		result += comps.front();
		comps.pop_front();
		if (!comps.empty())
			result += sep;
	}

	return result;
}

#ifndef SCUMMVM_UTIL

bool matchString(const char *str, const char *pat, bool ignoreCase, bool pathMode) {
	assert(str);
	assert(pat);

	const char *p = nullptr;
	const char *q = nullptr;
	bool escaped = false;

	for (;;) {
		if (pathMode && *str == '/') {
			p = nullptr;
			q = nullptr;
			if (*pat == '?')
				return false;
		}

		const char curPat = *pat;
		switch (*pat) {
		case '*':
			if (*str) {
				// Record pattern / string position for backtracking
				p = ++pat;
				q = str;
			} else {
				// If we've reached the end of str, we can't backtrack further
				// NB: We can't simply check if pat also ended here, because
				// the pattern might end with any number of *s.
				++pat;
				p = nullptr;
				q = nullptr;
			}
			// If pattern ended with * -> match
			if (!*pat)
				return true;
			break;

		case '\\':
			if (!escaped) {
				pat++;
				break;
			}
			// fallthrough

		case '#':
			// treat # as a wildcard for digits unless escaped
			if (!escaped) {
				if (!isDigit(*str))
					return false;
				pat++;
				str++;
				break;
			}
			// fallthrough

		default:
			if ((!ignoreCase && *pat != *str) ||
				(ignoreCase && tolower(*pat) != tolower(*str))) {
				if (p) {
					// No match, oops -> try to backtrack
					pat = p;
					str = ++q;
					if (!*str)
						return !*pat;
					break;
				}
				else
					return false;
			}
			// fallthrough
		case '?':
			if (!*str)
				return !*pat;
			pat++;
			str++;
		}

		escaped = !escaped && (curPat == '\\');
	}
}

void replace(Common::String &source, const Common::String &what, const Common::String &with) {
	const char *cstr = source.c_str();
	const char *position = strstr(cstr, what.c_str());
	if (position) {
		uint32 index = position - cstr;
		source.replace(index, what.size(), with);
	}
}

String tag2string(uint32 tag) {
	char str[5];
	str[0] = (char)(tag >> 24);
	str[1] = (char)(tag >> 16);
	str[2] = (char)(tag >> 8);
	str[3] = (char)tag;
	str[4] = '\0';
	// Replace non-printable chars by dot
	for (int i = 0; i < 4; ++i) {
		if (!Common::isPrint(str[i]))
			str[i] = '.';
	}
	return String(str);
}

#endif

size_t strlcpy(char *dst, const char *src, size_t size) {
	// Our backup of the source's start, we need this
	// to calculate the source's length.
	const char * const srcStart = src;

	// In case a non-empty size was specified we
	// copy over (size - 1) bytes at max.
	if (size != 0) {
		// Copy over (size - 1) bytes at max.
		while (--size != 0) {
			if ((*dst++ = *src) == 0)
				break;
			++src;
		}

		// In case the source string was longer than the
		// destination, we need to add a terminating
		// zero.
		if (size == 0)
			*dst = 0;
	}

	// Move to the terminating zero of the source
	// string, we need this to determine the length
	// of the source string.
	while (*src)
		++src;

	// Return the source string's length.
	return src - srcStart;
}

size_t strlcat(char *dst, const char *src, size_t size) {
	// In case the destination buffer does not contain
	// space for at least 1 character, we will just
	// return the source string's length.
	if (size == 0)
		return strlen(src);

	// Our backup of the source's start, we need this
	// to calculate the source's length.
	const char * const srcStart = src;

	// Our backup of the destination's start, we need
	// this to calculate the destination's length.
	const char * const dstStart = dst;

	// Search the end of the destination, but do not
	// move past the terminating zero.
	while (size-- != 0 && *dst != 0)
		++dst;

	// Calculate the destination's length;
	const size_t dstLength = dst - dstStart;

	// In case we reached the end of the destination
	// buffer before we had a chance to append any
	// characters we will just return the destination
	// length plus the source string's length.
	if (size == 0)
		return dstLength + strlen(srcStart);

	// Copy over all of the source that fits
	// the destination buffer. We also need
	// to take the terminating zero we will
	// add into consideration.
	while (size-- != 0 && *src != 0)
		*dst++ = *src++;
	*dst = 0;

	// Move to the terminating zero of the source
	// string, we need this to determine the length
	// of the source string.
	while (*src)
		++src;

	// Return the total length of the result string
	return dstLength + (src - srcStart);
}

size_t strnlen(const char *src, size_t maxSize) {
	size_t counter = 0;
	while (counter != maxSize && *src++)
		++counter;
	return counter;
}

String toPrintable(const String &in, bool keepNewLines) {
	Common::String res;

	const char *tr = "\x01\x01\x02\x03\x04\x05\x06" "a"
				  //"\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f";
					   "b" "t" "n" "v" "f" "r\x0e\x0f"
					"\x10\x11\x12\x13\x14\x15\x16\x17"
					"\x18\x19\x1a" "e\x1c\x1d\x1e\x1f";

	for (const byte *p = (const byte *)in.c_str(); *p; p++) {
		if (*p == '\n') {
			if (keepNewLines)
				res += *p;
			else
				res += "\\n";

			continue;
		}

		if (*p < 0x20 || *p == '\'' || *p == '\"' || *p == '\\') {
			res += '\\';

			if (*p < 0x20) {
				if (tr[*p] < 0x20)
					res += Common::String::format("x%02x", *p);
				else
					res += tr[*p];
			} else {
				res += *p;	// We will escape it
			}
		} else if (*p > 0x7e) {
			res += Common::String::format("\\x%02x", *p);
		} else
			res += *p;
	}

	return res;
}

} // End of namespace Common

// Portable implementation of stricmp / strcasecmp / strcmpi.
// TODO: Rename this to Common::strcasecmp
int scumm_stricmp(const char *s1, const char *s2) {
	byte l1, l2;
	do {
		// Don't use ++ inside tolower, in case the macro uses its
		// arguments more than once.
		l1 = (byte)*s1++;
		l1 = tolower(l1);
		l2 = (byte)*s2++;
		l2 = tolower(l2);
	} while (l1 == l2 && l1 != 0);
	return l1 - l2;
}

// Portable implementation of strnicmp / strncasecmp / strncmpi.
// TODO: Rename this to Common::strncasecmp
int scumm_strnicmp(const char *s1, const char *s2, uint n) {
	byte l1, l2;
	do {
		if (n-- == 0)
			return 0; // no difference found so far -> signal equality

		// Don't use ++ inside tolower, in case the macro uses its
		// arguments more than once.
		l1 = (byte)*s1++;
		l1 = tolower(l1);
		l2 = (byte)*s2++;
		l2 = tolower(l2);
	} while (l1 == l2 && l1 != 0);
	return l1 - l2;
}

const char *scumm_skipArticle(const char *s1) {
	int o1 = 0;
	if (!scumm_strnicmp(s1, "the ", 4))
		o1 = 4;
	else if (!scumm_strnicmp(s1, "a ", 2))
		o1 = 2;
	else if (!scumm_strnicmp(s1, "an ", 3))
		o1 = 3;

	return &s1[o1];
}

int scumm_compareDictionary(const char *s1, const char *s2) {
	return scumm_stricmp(scumm_skipArticle(s1), scumm_skipArticle(s2));
}

//  Portable implementation of strdup.
char *scumm_strdup(const char *in) {
	const size_t len = strlen(in) + 1;
	char *out = (char *)malloc(len);
	if (out) {
		strcpy(out, in);
	}
	return out;
}

//  Portable implementation of strcasestr.
const char *scumm_strcasestr(const char *s, const char *find) {
	char c, sc;
	size_t len;

	if ((c = *find++) != 0) {
		c = (char)tolower((unsigned char)c);
		len = strlen(find);
		do {
			do {
				if ((sc = *s++) == 0)
					return (NULL);
			} while ((char)tolower((unsigned char)sc) != c);
		} while (scumm_strnicmp(s, find, len) != 0);
		s--;
	}
	return s;
}
