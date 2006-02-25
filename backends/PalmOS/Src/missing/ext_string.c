/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include <string.h>

void *memchr(const void *s, int c, UInt32 n) {
	UInt32 chr;
	for(chr = 0; chr < n;chr++,((UInt8 *)s)++)
		if ( *((UInt8 *)s) == c)
			return (void *)s;

	return NULL;
}

UInt32 strspn(const char *s1, const char *s2) {
	UInt32 chr = 0;

	while (	chr < strlen(s1) &&
			strchr(s2, s1[chr]) )
		chr++;

	return chr;
}

static Char *StrTokNext = NULL;

Char *strtok(Char *str, const Char *sep) {
	Char	*position = NULL,
			*found,
			*end;

	UInt16	loop = 0,
			chars= StrLen(sep);

	str			= (str)?(str):(StrTokNext);
	StrTokNext	= NULL;

	if (!str)
		return NULL;

	end = str+StrLen(str);

	while (loop<chars)
	{
		found = StrChr(str,sep[loop]);
		loop++;

		if (found == str)
		{
			str++;
			loop = 0;
		}
		else if (position == NULL || position > found)
			position = found;
	}

	if (position == NULL)
		if (str==end)
			return NULL;
		else
			return str;

	position[0] = 0;
	StrTokNext	= position+1;

	return str;
}

Char *strpbrk(const Char *s1, const Char *s2) {
	Char *found;
	UInt32 n;

	for (n=0; n <= StrLen(s2); n++) {
		found = StrChr(s1, s2[n]);
		if (found)
			return found;
	}

	return NULL;
}

Char *strrchr(const Char *s, int c) {
	UInt32 chr;
	UInt32 n = StrLen(s);

	for(chr = n; chr >= 0; chr--)
		if ( *((UInt8 *)s+chr) == c)
			return (Char *)(s+chr);

	return NULL;
}

Char *strdup(const Char *s1) {
	Char* buf = (Char *)MemPtrNew(StrLen(s1)+1);

	if(buf)
		StrCopy(buf, s1);

	return buf;
}