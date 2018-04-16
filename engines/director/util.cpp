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

#include "common/str.h"
#include "common/textconsole.h"

#include "director/util.h"

namespace Director {

int castNumToNum(const char *str) {
	if (strlen(str) != 3)
		return -1;

	if (tolower(str[0]) >= 'a' && tolower(str[0]) <= 'h' &&
		str[1] >= '1' && str[1] <= '8' &&
		str[2] >= '1' && str[2] <= '8') {

		return (tolower(str[0]) - 'a') * 64 + (str[1] - '1') * 8 + (str[2] - '1') + 1;
	}

	return -1;
}

char *numToCastNum(int num) {
	static char res[4];

	res[0] = res[1] = res[2] = '?';
	res[3] = '\0';
	num--;

	if (num >= 0 && num < 512) {
		int c = num / 64;
		res[0] = 'A' + c;
		num -= 64 * c;

		c =  num / 8;
		res[1] = '1' + c;
		num -= 8 * c;

		res[2] = '1' + num;
	}

	return res;
}

// This is table for built-in Macintosh font lowercasing.
// '.' means that the symbol should be not changed, rest
// of the symbols are stripping the diacritics
// The table starts from 0x80
//
// TODO: Check it for correctness.
static char lowerCaseConvert[] =
"aacenoua" // 80
"aaaaacee" // 88
"eeiiiino" // 90
"oooouuuu" // 98
"........" // a0
".......o" // a8
"........" // b0
".......o" // b8
"........" // c0
".. aao.." // c8
"--.....y";// d0-d8

Common::String *toLowercaseMac(Common::String *s) {
	Common::String *res = new Common::String;
	const unsigned char *p = (const unsigned char *)s->c_str();

	while (*p) {
		if (*p >= 0x80 && *p <= 0xd8) {
			if (lowerCaseConvert[*p - 0x80] != '.')
				*res += lowerCaseConvert[*p - 0x80];
			else
				*res += *p;
		} else if (*p < 0x80) {
			*res += tolower(*p);
		} else {
			warning("Unacceptable symbol in toLowercaseMac: %c", *p);

			*res += *p;
		}
		p++;
	}

	return res;
}

Common::String convertPath(Common::String &path) {
	if (path.empty())
		return path;

	if (!path.contains(':')) {
		return path;
	}

	if (path[0] != ':') {
		warning("convertPath: unsupported absolute path '%s'", path.c_str());

		return path;
	}

	Common::String res;
	uint32 idx = 0;

	if (path.hasPrefix(":::")) {
		res = "../";
		idx = 3;
	} else {
		res = "./";
		idx = 1;
	}

	while (idx != path.size()) {
		if (path[idx] == ':')
			res += '/';
		else
			res += path[idx];

		idx++;
	}

	return res;
}

} // End of namespace Director
