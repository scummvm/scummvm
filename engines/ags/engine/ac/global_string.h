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

#ifndef __AGS_EE_AC__GLOBALSTRING_H
#define __AGS_EE_AC__GLOBALSTRING_H

int StrGetCharAt (const char *strin, int posn);
void StrSetCharAt (char *strin, int posn, int nchar);
void _sc_strcat(char*s1, const char*s2);
void _sc_strlower (char *desbuf);
void _sc_strupper (char *desbuf);
void _sc_strcpy(char*destt, const char*text);

#endif // __AGS_EE_AC__GLOBALSTRING_H
