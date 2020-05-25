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

#include "glk/hugo/stringfn.h"

namespace Glk {
namespace Hugo {

char *StringFunctions::Left(char a[], int l) {
	static char *temp;
	int i;

	temp = GetTempString();

	if (l > (int)strlen(a))
		l = strlen(a);
	for (i = 0; i<l; i++)
		temp[i] = a[i];
	temp[i] = '\0';
	return temp;
}

char *StringFunctions::Ltrim(char a[]) {
	static char *temp;
	int len = strlen(a);

	temp = GetTempString();
	strcpy(temp, a);
	while (temp[0]==' ' || temp[0]=='\t')
		memmove(temp, temp+1, len + 1);
	return temp;
}

char *StringFunctions::Mid(char a[], int pos, int n) {
	static char *temp;
	int i;

	temp = GetTempString();
	pos--;
	if (pos+n > (int)strlen(a))
		n = strlen(a)-pos;
	for (i = 0; i<n; i++)
		temp[i] = a[pos+i];
	temp[i] = '\0';
	return temp;
}

char *StringFunctions::Right(char a[], int l) {
	static char *temp;
	int i;

	temp = GetTempString();
	if (l > (int)strlen(a))
		l = strlen(a);
	for (i = 0; i<l; i++)
		temp[i] = a[strlen(a)-l+i];
	temp[i] = '\0';
	return temp;
}

char *StringFunctions::Rtrim(char a[]) {
	static char *temp;
	int len;

	temp = GetTempString();
	strcpy(temp, a);
	while (((len = strlen(temp))) && (temp[len-1]==' ' || temp[len-1]=='\t'))
		strcpy(temp, Left(temp, len-1));
	return temp;
}

char *StringFunctions::hugo_strcpy(char *s, const char *t)  {
	char *r = s;
	while ((*s++ = *t++) != 0) ;
	return r;
}

char *StringFunctions::GetTempString() {
	static char *r;

	r = &_tempString[_tempstringCount][0];
	if (++_tempstringCount >= NUM_TEMPSTRINGS)
		_tempstringCount = 0;

	return r;
}

char *StringFunctions::strlwr(char *s) {
	for (char *sp = s; *sp; ++sp)
		*sp = tolower(*sp);
	return s;
}

char *StringFunctions::strupr(char *s) {
	for (char *sp = s; *sp; ++sp)
		*sp = toupper(*sp);
	return s;
}

} // End of namespace Hugo
} // End of namespace Glk
