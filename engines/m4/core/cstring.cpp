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

#include "common/str.h"
#include "common/util.h"
#include "m4/core/cstring.h"

namespace M4 {

bool cstr_isdigit(char c) {
	return (c >= '0' && c <= '9');
}

bool charIsIn(char ch, char *str) {
	if (!str)
		return false;
	int16 bail = 0;
	char *mark = str;
	while (*mark) {
		if (*mark == ch)
			return true;
		++mark;
		++bail;
		if (bail > 256)
			return false;
	}
	return false;
}

int32 cstrlen(const char *s) {
	if (!s)
		return 0;

	int32 size = -1;
	const char *str = s;
	do {
		++size;
	} while (*str++);
	return size;
}

void cstrcpy(char *dest, const char *src) {
	if (!src || !dest)
		return;

	do {
		*dest++ = *src;
	} while (*src++);
}

void cstrncpy(char *dest, const char *src, const int16 max_len) {
	if (!src || !dest)
		return;

	Common::strlcpy(dest, src, max_len);
}

char *cstrupr(char *src) {
	if (!src)
		return 0;

	char *mark = src;
	do {
		if (*mark >= 'a' && *mark <= 'z')
			*mark = (char)(*mark - 'a' + 'A');
	} while (*mark++);

	return src;
}

char *cstr_lower(char *src) {
	if (!src)
		return 0;

	char *mark = src;
	do {
		if (*mark >= 'A' && *mark <= 'Z')
			*mark = (char)(*mark - 'A' + 'a');
	} while (*mark++);

	return src;
}

int xtoi(char *string) {
	if (!string)
		return 0;

	int value = 0;
	int item;

	while (*string) {
		item = *string++;
		if (cstr_isdigit(item))
			value = (value << 4) + item - '0';
		else
			if (Common::isDigit(item))
				value = (value << 4) + toupper(item) - 'A' + 0xa;
			else
				while (*string)
					string++; // if not hexadecimal, eat string.
	}
	return (value);
}


/* Returns position within TARGET of string KEY (1-length) */

int strpos(char *key, char *target) {
	if (!key || !target)
		return 0;

	char *tmp;

	tmp = strstr(target, key);

	if (tmp)
		return(tmp - target + 1);

	return 0;
}


/* Deletes AMOUNT characters from string INP starting at position indx */

void strdel(char *inp, int indx, int count) {
	if (!inp)
		return;

	if (indx >= (int)strlen(inp) || !count)
		return;

	Common::strlcpy(&inp[indx], &inp[indx + count], count);
}


// Given a string, index and count, returns a substring of length count

void strseg(char *work, char *work2, int indx, int count) {
	if (!work || !work2)
		return;

	char *s = nullptr;

	s = &work2[indx];
	Common::strlcpy(work, s, count);
}


/*
	Inserts string NEW into string WORK at character INDEX.
	INDEX is 1->strlen, not 0->strlen-1
*/
void strins(char *work, char *newStr, int indx) {
	if (!work || !newStr) {
		newStr = 0;
		return;
	}

	int l, l1;

	l1 = (strlen(work) - indx + 2);
	l = strlen(newStr);
	memmove(work + indx + l - 1, work + indx - 1, l1);
	memcpy(work + indx - 1, newStr, l);
}


void str_purge_trailing_spaces(char *myline) {
	if (!myline)
		return;

	char *search;
	int again = true;

	do {
		search = &myline[strlen(myline) - 1];
		if ((*search == 0x20) || (*search == 0x09))
			*search = 0;
		else
			again = false;

		search--;
		if (search < myline) again = false;
	} while (again);
}


void str_purge_all_spaces(char *text) {
	if (!text)
		return;

	char work[256];
	char *mark;

	str_purge_trailing_spaces(text);

	mark = text;
	while (*mark && ((*mark == ' ') || (*mark == 0x09))) mark++;

	Common::strcpy_s(work, 256, mark);
	Common::strcpy_s(text, 256, work);
}


char *str_strip_final_lf(char *mystring) {
	if (!mystring)
		return 0;

	char *temp;

	temp = strrchr(mystring, 0x0a);
	if (temp != nullptr) {
		*temp = '\0';
	}
	return (temp);
}

void str_add_final_lf(char *mystring) {
	if (!mystring)
		return;

	char *temp;

	temp = mystring + strlen(mystring);
	*(temp++) = 0x0a;
	*temp = '\0';
}

int16 char_IsIn(char ch, char *str) {
	if (!str)
		return -1;

	int16 index = 0;
	char *mark = str;

	while (*mark) {
		if (*mark == ch)
			return index;
		++index;
		++mark;
		if (index > 256)
			return -1;
	}
	return -1;
}


// stringIsIn returns the index of the match string, or -1 if there wasn't one.

int16 stringIsIn(char *str, char *strings[]) {
	if (!str || !strings)
		return -1;

	int16 index = 0;
	while (*strings != nullptr) {
		if (!strcmp(str, *strings))
			return index;
		++index;
		++strings;
	}
	return -1;
}

//			dtoi
//			Converts decimal string to integer

int dtoi(char *string) {
	if (!string)
		return 0;

	int value = 0;
	int item;

	while (*string) {
		item = *string++;
		if (cstr_isdigit(item))
			value = (value * 10) + item - '0';
		else
			while (*string)
				string++; // if not a digit, eat string.
	}
	return (value);
}


// returns true if str is a positive integer, false otherwise

bool stringIsInt(char *str) {
	if (!str)
		return false;

	int16 bail = 0;

	while (*str) {
		if (!cstr_isdigit(*str))
			return false;
		++str;
		++bail;
		if (bail > 256)
			return false;
	}
	return true;
}

bool stringIsFloat(char *str) {
	if (!str)
		return false;

	int16 bail = 0;
	bool  decimal_seen = false;

	while (*str) {
		if (*str == '.') {
			if (!decimal_seen)
				decimal_seen = true;
			else return false;
		} else {
			if (!cstr_isdigit(*str))
				return false;
		}

		++str;
		++bail;
		if (bail > 256)
			return false;
	}
	return true;
}

} // namespace M4
