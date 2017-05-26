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
#include "allfiles.h"
#include "newfatal.h"

#ifndef SLUDGE_STRINGY_H
#define SLUDGE_STRINGY_H

namespace Sludge {

char *copyString(const char *copyMe) {
	char *newString = new char [strlen(copyMe) + 1];
	if (! checkNew(newString)) return NULL;
	strcpy(newString, copyMe);
	return newString;
}

char *joinStrings(const char *s1, const char *s2) {
	char *newString = new char [strlen(s1) + strlen(s2) + 1];
	if (! checkNew(newString)) return NULL;
	sprintf(newString, "%s%s", s1, s2);
	return newString;
}

} // End of namespace Sludge

#endif
