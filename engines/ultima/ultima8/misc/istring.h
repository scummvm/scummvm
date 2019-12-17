/*
Copyright (C) 2003-2004 The Pentagram Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// istring.h -- case insensitive stl strings

#ifndef ULTIMA8_MISC_ISTRING_H
#define ULTIMA8_MISC_ISTRING_H

#include "ultima/ultima8/std/misc.h"
#include "ultima/ultima8/std/string.h"
#include "common/array.h"

#ifdef strcasecmp
#undef strcasecmp
#endif

#ifdef strncasecmp
#undef strncasecmp
#endif

namespace Ultima {
namespace Ultima8 {
namespace Pentagram {

extern int strcasecmp(const char *s1, const char *s2);
extern int strncasecmp(const char *s1, const char *s2, uint32 length);

//! Safe strcpy with size checking
void strcpy_s(char *dest, size_t size, const char *src);
//! Safe strcpy with size checking from dest array size
template<size_t size> inline void strcpy_s(char (& dest)[size], const char *src) {
	strcpy_s(dest, size, src);
}

//! Safe strcat with size checking
inline char *strcat_s(char *dest, size_t size, const char *src) {
	size_t cur = std::strlen(dest);
	if (cur < size) strcpy_s(dest + cur, size - cur, src);
	return dest;
}
//! Safe strcat with size checking from dest array size
template<size_t size> inline char (&strcat_s(char (& dest)[size], const char *src))[size]  {
	size_t cur = std::strlen(dest);
	if (cur < size) strcpy_s(dest + cur, size - cur, src);
	return dest;
}

class istring : public std::string {
public:
	istring() : std::string() {}
	istring(const char *str) : std::string(str) {}
	istring(const char *str, uint32 len) : std::string(str, len) {}
	istring(const char *beginP, const char *endP) : std::string(beginP, endP) {}
	istring(const String &str) : std::string(str) {}
	explicit istring(char c) : std::string(c) {}
	istring(size_t n, char c) : std::string(n, c) {}
	virtual ~istring() {}

	virtual int Compare(const string &s) const override {
		return compareToIgnoreCase(s);
	}

	void split(Common::Array<istring> &arr) const ;
};

} // End of namespace Pentagram
} // End of namespace Ultima8
} // End of namespace Ultima

#endif
