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

#ifndef GLK_HUGO_STRINGFN
#define GLK_HUGO_STRINGFN

#include "common/algorithm.h"

namespace Glk {
namespace Hugo {

#define NUM_TEMPSTRINGS 2

/**
 * The following string-manipulation functions closely mimic BASIC-language string functionality.
 * They do not alter the provided string; instead, they return a pointer to a static (modified) copy.
 */
class StringFunctions {
private:
	char _tempString[NUM_TEMPSTRINGS][1025];
	int _tempstringCount;

	char *GetTempString();
public:
	StringFunctions() : _tempstringCount(0) {
		Common::fill(&_tempString[0][0], &_tempString[NUM_TEMPSTRINGS][1025], '\0');
	}

	char *Left(char a[], int l);

	char *Ltrim(char a[]);

	char *Mid(char a[], int pos, int n);

	char *Right(char a[], int l);

	char *Rtrim(char a[]);

	char *hugo_strcpy(char *s, const char *t);

	char *strlwr(char *s);

	char *strupr(char *s);
};

} // End of namespace Hugo
} // End of namespace Glk

#endif
