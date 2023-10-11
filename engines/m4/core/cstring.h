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

#ifndef M4_CORE_CSTRING_H
#define M4_CORE_CSTRING_H

#include "common/str-array.h"
#include "m4/m4_types.h"

namespace M4 {

class StringArray : public Common::StringArray {
public:
	StringArray() : Common::StringArray() {}

	int indexOf(const Common::String &str) const {
		for (uint i = 0; i < size(); ++i) {
			if ((*this)[i] == str)
				return i;
		}
		return -1;
	}

	bool contains(const Common::String &str) const {
		return indexOf(str) != -1;
	}
	void remove(const Common::String &str) {
		int idx = indexOf(str);
		if (idx != -1)
			remove_at(idx);
	}
};

#define STR_PARSE_BUFFER_SIZE 255

extern bool charIsIn(char ch, char *str);
extern int16 char_IsIn(char ch, char *str); //new
extern int dtoi(char *string);
extern bool stringIsInt(char *str);
extern bool stringIsFloat(char *str);

extern int16 stringIsIn(char *str, char *strings[]);

extern int32 cstrlen(const char *s);
extern void cstrcpy(char *dest, const char *src);
extern void cstrncpy(char *dest, const char *src, const int16 max_len);
extern char *cstrupr(char *src);
extern char *cstr_lower(char *src);
extern int xtoi(char *string);
extern int strpos(char *key, char *target);
extern void strdel(char *inp, int indx, int count);
extern void strseg(char *work, char *work2, int indx, int count);
extern void strins(char *work, char *newStr, int indx);
extern void str_purge_trailing_spaces(char *myline);
extern void str_purge_all_spaces(char *text);
extern char *str_strip_final_lf(char *mystring);
extern void str_add_final_lf(char *mystring);
extern void str_parse_init(char *instring, char delimiter);
extern char *str_parse(char *out);

extern bool cstr_isdigit(char c);

#define strrun(a,b,c) memset(a,b,c)

} // namespace M4

#endif
