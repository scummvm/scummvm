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

#ifndef COMMON_STRING_ENCODING_H
#define COMMON_STRING_ENCODING_H

namespace Common {

class String;
class U32String;

enum CodePage {
	kCodePageInvalid = -1,
	kUtf8 = 0,
	kWindows1250,
	kWindows1251,
	kWindows1252,
	kWindows1253,
	kWindows1254,
	kWindows1255,
	kWindows1256,
	kWindows1257,
	kWindows932,
	kWindows949,
	kWindows950,
	kISO8859_1,
	kISO8859_2,
	kISO8859_5,
	kMacCentralEurope,
	kDos850,
	kDos866,
	kASCII,

	kLatin1 = kISO8859_1,
	kBig5 = kWindows950,
	kLastEncoding = kASCII
};

U32String convertUtf8ToUtf32(const String &str);
String convertUtf32ToUtf8(const U32String &str);

U32String convertToU32String(const char *str, CodePage page = kUtf8);
String convertFromU32String(const U32String &str, CodePage page = kUtf8);
uint16 convertUHCToUCS(uint8 high, uint8 low);
} // End of namespace Common

#endif
