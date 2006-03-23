/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

// Code is based on:

/* 
 * Copyright (c) 1998-2003 Massachusetts Institute of Technology. 
 * This code was developed as part of the Haystack research project 
 * (http://haystack.lcs.mit.edu/). Permission is hereby granted, 
 * free of charge, to any person obtaining a copy of this software 
 * and associated documentation files (the "Software"), to deal in 
 * the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, 
 * sublicense, and/or sell copies of the Software, and to permit 
 * persons to whom the Software is furnished to do so, subject to 
 * the following conditions: 
 * 
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software. 
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
 * OTHER DEALINGS IN THE SOFTWARE. 
 */

/*************************************************

  assocarray.h - Associative arrays

  Andrew Y. Ng, 1996

**************************************************/

#include "common/assocarray.h"

namespace Common {

// int:
int hashit(int x, int hashsize) {
	return x % hashsize;
}

int data_eq(int x, int y) {
	return x == y;
}

#if 0
// double:
int hashit(double d, int hashsize) {
	int hash, dex;
	byte *p = (byte *)&d;

	hash = 0;

	for (dex = 0; dex < sizeof(double); dex++)
		hash = ((hash << 8) + p[dex]) % hashsize;

	return hash;
}
#endif

int data_eq(double d1, double d2) {
	return (d1 == d2);
}

// const char *:
int hashit(const char *str, int hashsize) {
	const byte *p = (const byte *)str;
	int hash, dex;

	hash = 0;

	for (dex = 0; p[dex] != 0; dex++)
		hash = ((hash << 8) + p[dex]) % hashsize;

	return hash;
}

int data_eq(const char *str1, const char *str2) {
	return !strcmp(str1, str2);
}

// String:
int hashit(const Common::String &str, int hashsize) {
	return hashit(str.c_str(), hashsize);
}

int data_eq(const Common::String &str1, const String &str2) {
	return (str1 == str2);
}

}	// End of namespace Common
