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

namespace Common {

#ifndef RELEASE_BUILD
bool Path::_shownSeparatorCollisionWarning = false;
#endif

inline void Path::escape(String &dst, char srcSeparator, const char *begin, const char *end) {
	if (!end) {
		end = begin + strlen(begin);
	}

	const char *nextEsc = strchr(begin, ESCAPE);
	const char *nextEscSep, *nextSrcSep;
	if (srcSeparator == SEPARATOR) {
		// The source directory separator is ours, no need to escape it
		nextEscSep = nullptr;
	} else {
		nextEscSep = strchr(begin, SEPARATOR);
	}
	if (!srcSeparator || srcSeparator == SEPARATOR) {
		// There is no source directory separator, every SEPARATOR will get escaped
		// Source directory separator is already a SEPARATOR, nothing to do
		nextSrcSep = nullptr;
	} else {
		nextSrcSep = strchr(begin, srcSeparator);
	}

	while (true) {
		const char *next = nullptr;
		if (nextEsc && nextEsc < end && (!next || nextEsc < next)) {
			next = nextEsc;
		}
		if (nextEscSep && nextEscSep < end && (!next || nextEscSep < next)) {
			next = nextEscSep;
		}
		if (nextSrcSep && nextSrcSep < end && (!next || nextSrcSep < next)) {
			next = nextSrcSep;
		}
		if (!next) {
			break;
		}

		// Append all content up to what we found first, excluded
		dst.append(begin, next);

		// We will look after the current found character
		begin = next + 1;

		// Do the escaping
		if (next == nextEsc) {
			// We found an ESCAPE
			dst += ESCAPE;
			dst += ESCAPED_ESCAPE;
			nextEsc = strchr(begin, ESCAPE);
		} else if (next == nextEscSep) {
			// We found a SEPARATOR
			dst += ESCAPE;
			dst += ESCAPED_SEPARATOR;
			nextEscSep = strchr(begin, SEPARATOR);
		} else {
			// We found a srcSeparator
			dst += SEPARATOR;
			nextSrcSep = strchr(begin, srcSeparator);
		}
	}

	// Append anything after the last escape up to end
	dst.append(begin, end);
}

inline String Path::unescape(char dstSeparator, const char *begin, const char *end) {
	String dst;

	if (!end) {
		end = begin + strlen(begin);
	}

#ifndef RELEASE_BUILD
	if (!_shownSeparatorCollisionWarning &&
	        dstSeparator &&
	        dstSeparator != ESCAPE && dstSeparator != SEPARATOR) {
		// ESCAPE and SEPARATOR cases are handled when unescaping
		const char *dstSep = strchr(begin, dstSeparator);
		if (dstSep && dstSep < end) {
			warning("Collision in path \"%s\" with separator %c", begin, dstSeparator);
			_shownSeparatorCollisionWarning = true;
		}
	}
#endif

	const char *nextEsc = strchr(begin, ESCAPE);
	const char *nextSep;
	if (dstSeparator == SEPARATOR) {
		// dstSeparator is SEPARATOR so there is no substitution to do
		nextSep = nullptr;
	} else {
		nextSep = strchr(begin, SEPARATOR);
	}

	while (true) {
		const char *next = nullptr;
		if (nextEsc && nextEsc < end && (!next || nextEsc < next)) {
			next = nextEsc;
		}
		if (nextSep && nextSep < end && (!next || nextSep < next)) {
			next = nextSep;
		}
		if (!next) {
			break;
		}

		// Append all content up to character excluded
		dst.append(begin, next);

		if (next == nextSep) {
			// We found a SEPARATOR, replace with dstSeparator
			// Make sure dstSeparator is not 0
			assert(dstSeparator);
			dst += dstSeparator;
			begin = next + 1;
			nextSep = strchr(begin, SEPARATOR);

			continue;
		}

		// We found an ESCAPE
		char follower = *(next + 1);
		switch (follower) {
		case ESCAPED_ESCAPE:
			dst += ESCAPE;
#ifndef RELEASE_BUILD
			if (!_shownSeparatorCollisionWarning && dstSeparator == ESCAPE) {
				warning("Collision 1 while unescaping in path part \"%s\" with separator %c", begin, dstSeparator);
				_shownSeparatorCollisionWarning = true;
			}
#endif
			break;
		case ESCAPED_SEPARATOR:
			dst += SEPARATOR;
#ifndef RELEASE_BUILD
			if (!_shownSeparatorCollisionWarning && dstSeparator == SEPARATOR) {
				warning("Collision 2 while unescaping in path part \"%s\" with separator %c", begin, dstSeparator);
				_shownSeparatorCollisionWarning = true;
			}
#endif
			break;
		default:
			error("Invalid escape character '%c' in path part \"%s\"", follower, begin);
		}
		begin = next + 2;
		// Look for the next one
		nextEsc = strchr(begin, ESCAPE);
	}

	// Append anything after the last ESCAPE up to end
	dst.append(begin, end);

	return dst;
}

bool Path::canUnescape(bool willBeAtBegin, bool wasAtBegin,
                       const char *begin, const char *end) {
	if (begin == end || (end == nullptr && *begin == '\x00')) {
		// It's empty
		return true;
	}

	if (*begin == ESCAPE) {
		if (willBeAtBegin) {
			// begin will be the start and contains an ESCAPE marker
			// This means it escapes either a SEPARATOR or an ESCAPE
			return false;
		}

		// We won't end up at begin of future path so we must check further what ESCAPE we have
	} else {
		if (wasAtBegin && end == nullptr) {
			// begin is the start of an escaped path and doesn't begin with an ESCAPE
			// and we go till the end of string
			// This means we must have ESCAPED_SEPARATOR somewhere
			return false;
		}
	}

	const char *esc = strchr(begin, ESCAPE);
	while (esc && (!end || esc < end)) {
		if (*(esc + 1) == ESCAPED_SEPARATOR) {
			return false;
		}
		esc = strchr(esc + 2, ESCAPE);
	}
	return true;
}

String Path::encode(const char *str, char separator) {
	if (*str != ESCAPE && !strchr(str, SEPARATOR)) {
		// If we start with ESCAPE we will need to escape
		// No SEPARATOR, no clash
		Common::String ret(str);
		if (separator) {
			// If separator is 0 there is only one path component
			ret.replace(separator, SEPARATOR);
		}
		return ret;
	}

	// Prepend ESCAPE as we are escaping
	Common::String ret(ESCAPE);

	// Do the escape and replace separators by SEPARATOR
	escape(ret, separator, str);

	return ret;
}

Path Path::extract(const char *begin, const char *end) const {
	if (_str.empty() || begin == end) {
		// We are empty, we can't extract anything
		return Path();
	}

	if (!end) {
		end = _str.c_str() + _str.size();
	}

	if (!isEscaped()) {
		Path ret;
		if (*begin == ESCAPE) {
			// We were not escaped but we will need to because we are creating a path starting with ESCAPE
			ret._str += ESCAPE;
			escape(ret._str, SEPARATOR, begin, end);
		} else {
			// Not escaped and no escape character at begin: just do the extraction
			ret._str = String(begin, end);
		}
		return ret;
	}

	// We are escaped

	// Make sure begin points on real start
	if (begin == _str.c_str()) {
		begin++;
	}

	if (canUnescape(true, false, begin, end)) {
		// No escaped separator was found and no ESCAPE was at begin, unescape
		Path ret;
		ret._str = unescape(SEPARATOR, begin, end);

		return ret;
	}

	// We can't unescape add escape marker and the escaped range
	Path ret;
	ret._str += ESCAPE;
	ret._str.append(begin, end);
	return ret;
}

String Path::toString(char separator) const {
	// toString should never be called with \x00 separator
	assert(separator != kNoSeparator);

	if (_str.empty() ||
	        (separator == SEPARATOR && !isEscaped())) {
		return _str;
	}

	if (!isEscaped()) {
		// Path was not escaped, replace all SEPARATOR by the real separator
#ifndef RELEASE_BUILD
		if (!_shownSeparatorCollisionWarning && strchr(_str.c_str(), separator)) {
			warning("Collision 3 while unescaping path \"%s\" with separator %c", _str.c_str(), separator);
			_shownSeparatorCollisionWarning = true;
		}
#endif

		Common::String ret(_str);
		ret.replace(SEPARATOR, separator);
		return ret;
	}

	// Remove leading escape indicator and do our stuff
	const char *str = _str.c_str();
	str++;

	// Unescape and replace SEPARATORs by separator
	return unescape(separator, str, _str.c_str() + _str.size());
}

size_t Path::findLastSeparator(size_t last) const {
	if (last == String::npos || last > _str.size()) {
		// Easy case
		const char *sep = strrchr(_str.c_str(), SEPARATOR);
		if (!sep) {
			return String::npos;
		} else {
			return sep - _str.c_str();
		}
	}

	// Let's jump from separator to separator
	const char *begin = _str.c_str();
	const char *end = begin + last;

	const char *str = begin;
	const char *sep = strchr(str, SEPARATOR);
	while (sep && sep < end) {
		str = sep + 1;
		sep = strchr(str, SEPARATOR);
	}

	if (str == begin) {
		// Nothing was found in the range specified
		return String::npos;
	} else {
		// str was pointing just after the matching separator
		return (str - 1) - begin;
	}
}

Path Path::getParent() const {
	if (_str.empty()) {
		return Path();
	}

	// ignore potential trailing separator
	size_t separatorPos = findLastSeparator(_str.size() - 1);
	if (separatorPos == String::npos) {
		return Path();
	}

	const char *begin = _str.c_str();
	const char *end = begin + separatorPos + 1;
	return extract(begin, end);
}

Path Path::getLastComponent() const {
	if (_str.empty()) {
		return Path();
	}

	// ignore potential trailing separator
	size_t separatorPos = findLastSeparator(_str.size() - 1);
	if (separatorPos == String::npos) {
		return *this;
	}

	const char *begin = _str.c_str() + separatorPos + 1;
	const char *end = _str.c_str() + _str.size();
	return extract(begin, end);
}

String Path::baseName() const {
	if (_str.empty()) {
		return String();
	}

	size_t last = _str.size();
	if (isSeparatorTerminated()) {
		last--;
	}

	const char *begin = _str.c_str();
	const char *end = _str.c_str();

	size_t separatorPos = findLastSeparator(last);

	if (separatorPos != String::npos) {
		begin += separatorPos + 1;
	} else if (isEscaped()) {
		// unescape uses the real start, not the escape marker
		begin++;
	}
	end += last;

	return unescape(kNoSeparator, begin, end);
}

int Path::numComponents() const {
	if (_str.empty())
		return 0;

	const char *str = _str.c_str();

	if (isEscaped())
		str++;

	int num = 1;

	const char *sep = strchr(str, SEPARATOR);
	while (sep) {
		str = sep + 1;
		sep = strchr(str, SEPARATOR);
		num++;
	}

	return num;
}

Path &Path::appendInPlace(const Path &x) {
	if (x._str.empty()) {
		return *this;
	}

	if (_str.empty()) {
		_str = x._str;
		return *this;
	}
	// From here both paths have data

	if (isEscaped() == x.isEscaped()) {
		if (isEscaped()) {
			// Both are escaped: no chance we could unescape
			// Append the other without its escape mark
			_str.append(x._str.begin() + 1, x._str.end());
		} else {
			// No escape: append as we will don't need to escape
			_str += x._str;
		}
		return *this;
	}

	// If we are here, one of us is escaped and the other one is not
	if (isEscaped()) {
		// This one is escaped not the other one
		// There is no chance we could unescape this one
		escape(_str, SEPARATOR, x._str.c_str(), x._str.c_str() + x._str.size());
		return *this;
	}

	// This one is not escaped the other one is
	// There is a small chance the other one is escaped because it started with ESCAPED
	// In this case (and only this one) we could unescape
	const char *other = x._str.c_str() + 1; // Remove leading ESCAPE

	if (canUnescape(false, true, other)) {
		// This one stays unescaped and the other one becomes unescaped
		_str += unescape(SEPARATOR, other, x._str.c_str() + x._str.size());
	} else {
		// This one gets escaped and the other one stays escaped
		String str(ESCAPE);
		escape(str, SEPARATOR, _str.c_str(), _str.c_str() + _str.size());
		str.append(other, x._str.c_str() + x._str.size());
		// Replace ourselves
		_str = str;
	}
	return *this;
}

Path &Path::appendInPlace(const char *str, char separator) {
	if (!*str) {
		return *this;
	}
	if (_str.empty()) {
		set(str, separator);
		return *this;
	}
	// From here both paths have data

	if (isEscaped()) {
		// This one is escaped
		// There is no chance we could unescape it
		escape(_str, separator, str);
		return *this;
	}

	// We are not escaped, let's hope str won't need escaping
	if (!needsEncoding(str, separator)) {
		// No need to encode means it's perfect
		_str += str;
		return *this;
	}

	if (!strchr(str, SEPARATOR)) {
		// No SEPARATOR, no clash
		// Even if str starts with ESCAPE, no need to escape as we append
		Common::String tmp(str);
		if (separator) {
			// If separator is 0 there is only one path component
			tmp.replace(separator, SEPARATOR);
		}
		_str += tmp;
		return *this;
	}

	// Too bad, we need to escape
	String tmp(ESCAPE);
	escape(tmp, SEPARATOR, _str.c_str(), _str.c_str() + _str.size());
	escape(tmp, separator, str);
	// Replace ourselves
	_str = tmp;

	return *this;
}

Path Path::appendComponent(const char *str) const {
	if (!*str) {
		return *this;
	}

	if (_str.empty()) {
		return Path(str, kNoSeparator);
	}

	bool addSeparator = (*(_str.end() - 1) != SEPARATOR);

	if (isEscaped()) {
		// We are escaped, escape str as well
		Path ret(*this);
		if (addSeparator) {
			ret._str += SEPARATOR;
		}
		// It's a component, there is no directory separator in source
		escape(ret._str, kNoSeparator, str);
		return ret;
	} else if (strchr(str, SEPARATOR)) {
		// We are not escaped but str will need escape: escape both parts
		Path ret;
		ret._str += ESCAPE;
		escape(ret._str, SEPARATOR, _str.c_str(), _str.c_str() + _str.size());
		if (addSeparator) {
			ret._str += SEPARATOR;
		}
		// It's a component, there is no directory separator in source
		escape(ret._str, kNoSeparator, str);
		return ret;
	} else {
		// No need to escape anything
		Path ret(*this);
		if (addSeparator) {
			ret._str += SEPARATOR;
		}
		ret._str += str;
		return ret;
	}
}

Path &Path::joinInPlace(const Path &x) {
	if (x.empty()) {
		return *this;
	}
	if (_str.empty()) {
		_str = x._str;
		return *this;
	}

	const char *str = x._str.c_str();
	// Append SEPARATOR if:
	// - we don't finish with SEPARATOR already
	// - other Path is escaped and doesn't begin by SEPARATOR
	// - or other Path isn't escaped and doesn't begin by SEPARATOR
	if (*(_str.end() - 1) != SEPARATOR &&
	        ((x.isEscaped() && str[1] != SEPARATOR) ||
	         *str != SEPARATOR)) {
		_str += SEPARATOR;
	}

	return appendInPlace(x);
}

Path &Path::joinInPlace(const char *str, char separator) {
	if (*str == '\0') {
		return *this;
	}
	if (_str.empty()) {
		set(str, separator);
		return *this;
	}

	if (*(_str.end() - 1) != SEPARATOR && *str != separator) {
		_str += SEPARATOR;
	}

	return appendInPlace(str, separator);
}

Path &Path::removeTrailingSeparators() {
	while (_str.size() > 1 && _str.lastChar() == SEPARATOR) {
		_str.deleteLastChar();
	}
	return *this;
}

const char *Path::getSuffix(const Common::Path &other) const {
	if (other.empty()) {
		// Other is empty, return full string
		const char *suffix = _str.c_str();
		if (isEscaped()) {
			suffix++;
		}
		return suffix;
	}

	if (isEscaped() == other.isEscaped()) {
		// Easy one both have same escapism
		if (_str.hasPrefix(other._str)) {
			const char *suffix = _str.c_str() + other._str.size();
			if (!other.isSeparatorTerminated()) {
				if (*suffix == SEPARATOR) {
					// Skip the separator
					return suffix + 1;
				} else if (*suffix == '\x00') {
					// Both paths are equal: return end of string
					return suffix;
				} else {
					// We are in the middle of some path component: this is not relative
					return nullptr;
				}
			} else {
				// Other already had a separator: this is relative and starts with next component
				return suffix;
			}
		} else {
			return nullptr;
		}
	}

	// One is escaped and not the other
	if (other.isEscaped()) {
		// If the other is escaped it can't be a substring of this one
		// as it must contain something which needed escape and we obviously didn't need it
		return nullptr;
	}

	// We may be escaped because something needs escape after the end of the prefix
	// We need to iterate over both paths and checking escape in ours
	const char *str = _str.c_str() + 1;
	const char *strOther = other._str.c_str();
	while (*strOther) {
		char c = *str;
		if (c == ESCAPE) {
			str++;
			switch (*str) {
			case ESCAPED_ESCAPE:
				c = ESCAPE;
				break;
			case ESCAPED_SEPARATOR:
				/* We are not supposed to have an escaped separator as
				 * the other one would need one too.
				 * We would then have the same escapism.
				 */
				c = SEPARATOR;
				break;
			default:
				error("Invalid escape character '%c' in path \"%s\"", *str, _str.c_str());
			}
		}
		if (*strOther != c) {
			break;
		}
		++strOther;
		++str;
	}
	// It's a prefix, if and only if all letters in other are 'used up' before
	// we end.
	if (*strOther != '\0') {
		return nullptr;
	}

	if (!other.isSeparatorTerminated()) {
		// Make sure we didn't end up in the middle of some path component
		if (*str != SEPARATOR) {
			return nullptr;
		}
		str++;
	}

	return str;
}

Path Path::relativeTo(const Common::Path &other) const {
	const char *suffix = getSuffix(other);
	if (!suffix) {
		return *this;
	}

	// Remove all spurious separators
	while (*suffix == SEPARATOR) {
		suffix++;
	}

	return extract(suffix);
}

Path Path::normalize() const {
	if (_str.empty()) {
		return Path();
	}

	const char *cur = _str.c_str();

	bool hasLeadingSeparator = false;

	if (isEscaped()) {
		// We are an escaped path
		cur++;
	}

	// If there is a leading separator, preserve that:
	if (*cur == SEPARATOR) {
		hasLeadingSeparator = true;
		cur++;
		// Skip over multiple leading separators, so "//" equals "/"
		while (*cur == SEPARATOR) {
			cur++;
		}
	}

	// Scan for path components till the end of the String
	StringArray comps;
	while (*cur) {
		const char *start = cur;

		// Scan till the next path separator resp. the end of the string
		cur = strchr(cur, SEPARATOR);
		if (!cur) {
			cur = _str.c_str() + _str.size();
		}

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
		while (*cur == SEPARATOR) {
			cur++;
		}
	}

	Common::Path result;

	if (comps.empty()) {
		// No components, add the leading separator if there was any
		if (hasLeadingSeparator) {
			result._str += SEPARATOR;
		}
		return result;
	}

	// Before making the new path, we need to determine if it will be escaped
	bool needEscape = false;
	bool needUnescape = false;
	if (isEscaped()) {
		// We are an escaped path
		// Maybe we can get unescaped because problematic component got shifted
		if (hasLeadingSeparator || *comps.front().c_str() != ESCAPE) {
			// Well, we don't start with ESCAPE so there is still a chance we get unescaped
			needUnescape = true;

			StringArray::const_iterator it;
			for (it = comps.begin(); it != comps.end(); it++) {
				if (!canUnescape(false, false, it->c_str())) {
					// Nope we can't get unescaped
					needUnescape = false;
					break;
				}
			}
		}

		// We will have an escaped result
		if (!needUnescape) {
			result._str += ESCAPE;
		}
	} else {
		// We are not an escaped path
		// The only reason we would need to escape is that we begin with an ESCAPE character
		if (!hasLeadingSeparator && *comps.front().c_str() == ESCAPE) {
			// Uh oh: we need to escape everything
			needEscape = true;
			result._str += ESCAPE;
		}
	}

	// Finally, assemble all components back into a path
	bool addSep = hasLeadingSeparator;
	for (StringArray::const_iterator it = comps.begin(); it != comps.end(); it++) {
		if (addSep) {
			result._str += SEPARATOR;
		}
		addSep = true;

		if (needEscape) {
			escape(result._str, kNoSeparator, it->c_str(), it->c_str() + it->size());
		} else if (needUnescape) {
			result._str += unescape(kNoSeparator, it->c_str(), it->c_str() + it->size());
		} else {
			result._str += *it;
		}
	}

	return result;
}

StringArray Path::splitComponents() const {
	StringArray res;

	if (_str.empty()) {
		// We always return at least 1 component
		res.push_back("");
		return res;
	}

	const char *str = _str.c_str();
	const char *end = str + _str.size();

	bool escaped = isEscaped();
	if (escaped) {
		str++;
	}

	const char *sep = strchr(str, SEPARATOR);
	while (sep) {
		if (!escaped) {
			res.push_back(String(str, sep));
		} else {
			res.push_back(unescape(kNoSeparator, str, sep));
		}
		str = sep + 1;
		sep = strchr(str, SEPARATOR);
	}

	if (!escaped) {
		res.push_back(String(str, end));
	} else {
		res.push_back(unescape(kNoSeparator, str, end));
	}

	return res;
}

Path Path::joinComponents(StringArray::const_iterator begin, StringArray::const_iterator end) {
	if (begin == end) {
		return Path();
	}

	Path ret(*begin, kNoSeparator);

	for (StringArray::const_iterator it = begin + 1; it != end; it++) {
		ret._str += SEPARATOR;
		ret.appendInPlace(*it, kNoSeparator);
	}
	return ret;
}

template<typename T>
T Path::reduceComponents(T(*reducer)(T value, const String &element, bool last), T value) const {
	if (_str.empty()) {
		return value;
	}

	const char *str = _str.c_str();
	const char *end = str + _str.size();

	bool escaped = isEscaped();
	if (escaped) {
		str++;
	}

	const char *sep = strchr(str, SEPARATOR);
	while (sep) {
		String item;
		if (escaped) {
			item = unescape(kNoSeparator, str, sep);
		} else {
			item = String(str, sep);
		}

		value = reducer(value, item, false);

		str = sep + 1;
		sep = strchr(str, SEPARATOR);
	}

	String item;
	if (escaped) {
		item = unescape(kNoSeparator, str, end);
	} else {
		item = String(str, end);
	}

	value = reducer(value, item, true);

	return value;
}

bool Path::compareComponents(bool (*comparator)(const String &x, const String &y),
                             const Path &other) const {
	if (_str.empty() != other._str.empty()) {
		// One is empty and the other is not
		return false;
	}
	if (_str.empty()) {
		// Both are empty
		return true;
	}

	const char *str = _str.c_str();
	const char *end = str + _str.size();
	bool escaped = isEscaped();
	if (escaped) {
		str++;
	}

	const char *strOther = other._str.c_str();
	const char *endOther = strOther + other._str.size();
	bool escapedOther = other.isEscaped();
	if (escapedOther) {
		strOther++;
	}

	const char *sep = strchr(str, SEPARATOR);
	const char *sepOther = strchr(strOther, SEPARATOR);
	while (sep && sepOther) {
		String item;
		if (escaped) {
			item = unescape(kNoSeparator, str, sep);
		} else {
			item = String(str, sep);
		}

		String itemOther;
		if (escapedOther) {
			itemOther = unescape(kNoSeparator, strOther, sepOther);
		} else {
			itemOther = String(strOther, sepOther);
		}

		if (!comparator(item, itemOther)) {
			return false;
		}

		str = sep + 1;
		strOther = sepOther + 1;
		sep = strchr(str, SEPARATOR);
		sepOther = strchr(strOther, SEPARATOR);
	}

	if (sep != sepOther) {
		// When one is nullptr and the other is not
		// This means we don't have an equal number of path components
		return false;
	}

	String item;
	if (escaped) {
		item = unescape(kNoSeparator, str, end);
	} else {
		item = String(str);
	}

	String itemOther;
	if (escapedOther) {
		itemOther = unescape(kNoSeparator, strOther, endOther);
	} else {
		itemOther = String(strOther);
	}

	return comparator(item, itemOther);
}

Path Path::punycodeDecode() const {
	Path tmp;
	return reduceComponents<Path &>(
		[](Path &path, const String &in, bool last) -> Path & {
			// We encode the result as UTF-8
			String out = punycode_hasprefix(in) ?
				     punycode_decodefilename(in).encode() :
				     in;
			path.appendInPlace(out, kNoSeparator);
			if (!last) {
				path._str += SEPARATOR;
			}
			return path;
		}, tmp);
}

Path Path::punycodeEncode() const {
	Path tmp;
	return reduceComponents<Path &>(
		[](Path &path, const String &in, bool last) -> Path & {
			// We decode the result as UTF-8
			Common::String out = punycode_encodefilename(in.decode());
			path.appendInPlace(out, kNoSeparator);
			if (!last) {
				path._str += SEPARATOR;
			}
			return path;
		}, tmp);
}

// For a path component creates a string with following property:
// if 2 files have the same case-insensitive
// identifier string then and only then we treat them as
// effectively the same file. For this there are 2
// transformations we need to do:
// * decode punycode
// * Replace / with : in path components so a path from
// HFS(+) image will end up with : independently of how
// it was dumped or copied from
static String getIdentifierComponent(const String &in) {
	String part = punycode_hasprefix(in) ?
	              punycode_decodefilename(in).encode() :
	              in;
	part.replace('/', ':');
	return part;
}

uint Path::hash() const {
	return hashit(_str.c_str());
}

uint Path::hashIgnoreCase() const {
	return hashit_lower(_str);
}

// This hash algorithm is inspired by a Python proposal to hash for tuples
// https://bugs.python.org/issue942952#msg20602
// As we don't have the length, it's not added in but
// it doesn't change collisions that much on their stress test
struct hasher {
	uint result;
	uint mult;
};

uint Path::hashIgnoreCaseAndMac() const {
	hasher v = { 0x345678, 1000003 };
	reduceComponents<hasher &>(
		[](hasher &value, const String &in, bool last) -> hasher & {
			uint hash = hashit_lower(getIdentifierComponent(in));

			value.result = (value.result + hash) * value.mult;
			value.mult = (value.mult * 69069);
			return value;
		}, v);
	return v.result;
}

bool Path::matchPattern(const Path &pattern) const {
	return compareComponents(
		[](const String &x, const String &y) {
			return getIdentifierComponent(x).matchString(getIdentifierComponent(y), true);
		}, pattern);
}

bool Path::equalsIgnoreCase(const Path &other) const {
	return _str.equalsIgnoreCase(other._str);
}

bool Path::equalsIgnoreCaseAndMac(const Path &other) const {
	return compareComponents(
		[](const String &x, const String &y) {
			return getIdentifierComponent(x).equalsIgnoreCase(getIdentifierComponent(y));
		}, other);
}

bool Path::operator<(const Path &x) const {
	// Here order matters: if we other is empty it is less or equal than us
	// We can only be greater or equal
	if (x.empty()) {
		return false;
	}
	// Other is not empty, we are: we are less
	if (empty()) {
		return true;
	}

	// Same escapism, easy to compare
	if (isEscaped() == x.isEscaped()) {
		return _str < x._str;
	}

	// Different escapism: fall back on escaped
	const Common::String &ref = isEscaped() ? _str : x._str;
	Common::String val(ESCAPE);
	if (isEscaped()) {
		escape(val, SEPARATOR, x._str.c_str());
		return ref < val;
	} else {
		escape(val, SEPARATOR, _str.c_str());
		return val < ref;
	}
}

Path Path::fromConfig(const String &value) {
	if (value.empty()) {
		return Path();
	}

	// Paths saved under WIN32 used \ separator
	// We now punyencode and store using /
#if defined(WIN32)
	if (strchr(value.c_str(), Path::kNativeSeparator)) {
		String value_ = value;
		// We may have mixed \ and / in our paths before
		// As / is forbidden in paths under WIN32, this will never be a collision
		value_.replace(Path::kNativeSeparator, '/');
		// As we have \, we are sure we didn't punyencode this path
		return Path(value_, '/');
	}
#endif

	// If the path is not punyencoded this will be a no-op
	return Path(value, '/').punycodeDecode();
}

Path Path::fromCommandLine(const String &value) {
	if (value.empty()) {
		return Path();
	}

	// WIN32 accepts / and \ as separators
#if defined(WIN32)
	if (strchr(value.c_str(), Path::kNativeSeparator)) {
		String value_ = value;
		// User may have mixed \ and /
		// As / is forbidden in paths under WIN32, this will never be a collision
		value_.replace(Path::kNativeSeparator, '/');
		return Path(value_, '/');
	}
#endif
	// Unlike for options the paths provided by the user are not punyencoded
	return Path(value, '/');
}

} // End of namespace Common
