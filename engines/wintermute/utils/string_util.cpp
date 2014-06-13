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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "common/tokenizer.h"
#include "engines/wintermute/utils/string_util.h"
#include "engines/wintermute/utils/convert_utf.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
bool StringUtil::compareNoCase(const AnsiString &str1, const AnsiString &str2) {
	return (str1.compareToIgnoreCase(str2) == 0);
}

//////////////////////////////////////////////////////////////////////////
/*bool StringUtil::CompareNoCase(const WideString &str1, const WideString &str2) {
    WideString str1lc = str1;
    WideString str2lc = str2;

    ToLowerCase(str1lc);
    ToLowerCase(str2lc);

    return (str1lc == str2lc);
}*/

//////////////////////////////////////////////////////////////////////////
WideString StringUtil::utf8ToWide(const Utf8String &Utf8Str) {
	size_t wideSize = Utf8Str.size();

	uint32 *wideStringNative = new uint32[wideSize + 1];

	const UTF8 *sourceStart = reinterpret_cast<const UTF8 *>(Utf8Str.c_str());
	const UTF8 *sourceEnd = sourceStart + wideSize;

	UTF32 *targetStart = reinterpret_cast<UTF32 *>(wideStringNative);
	UTF32 *targetEnd = targetStart + wideSize;

	ConversionResult res = ConvertUTF8toUTF32(&sourceStart, sourceEnd, &targetStart, targetEnd, strictConversion);
	if (res != conversionOK) {
		delete[] wideStringNative;
		return WideString();
	}
	*targetStart = 0;
	WideString resultString(wideStringNative);
	delete[] wideStringNative;
	return resultString;
}

//////////////////////////////////////////////////////////////////////////
Utf8String StringUtil::wideToUtf8(const WideString &WideStr) {
	size_t wideSize = WideStr.size();

	size_t utf8Size = 4 * wideSize + 1;
	char *utf8StringNative = new char[utf8Size];

	const UTF32 *sourceStart = reinterpret_cast<const UTF32 *>(WideStr.c_str());
	const UTF32 *sourceEnd = sourceStart + wideSize;

	UTF8 *targetStart = reinterpret_cast<UTF8 *>(utf8StringNative);
	UTF8 *targetEnd = targetStart + utf8Size;

	ConversionResult res = ConvertUTF32toUTF8(&sourceStart, sourceEnd, &targetStart, targetEnd, strictConversion);
	if (res != conversionOK) {
		delete[] utf8StringNative;
		return Utf8String();
	}
	*targetStart = 0;
	Utf8String resultString(utf8StringNative);
	delete[] utf8StringNative;
	return resultString;
}

//////////////////////////////////////////////////////////////////////////
WideString StringUtil::ansiToWide(const AnsiString &str) {
	WideString result;
	for (AnsiString::const_iterator i = str.begin(), end = str.end(); i != end; ++i) {
		const byte c = *i;
		if (c < 0x80 || c >= 0xA0) {
			result += c;
		} else {
			uint32 utf32 = _ansiToUTF32[c - 0x80];
			if (utf32) {
				result += utf32;
			} else {
				// It's an invalid CP1252 character...
			}
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////
AnsiString StringUtil::wideToAnsi(const WideString &wstr) {
	AnsiString result;
	for (WideString::const_iterator i = wstr.begin(), end = wstr.end(); i != end; ++i) {
		const uint32 c = *i;
		if (c < 0x80 || (c >= 0xA0 && c <= 0xFF)) {
			result += c;
		} else {
			uint32 ansi = 0xFFFFFFFF;
			for (uint j = 0; j < ARRAYSIZE(_ansiToUTF32); ++j) {
				if (_ansiToUTF32[j] == c) {
					ansi = j + 0x80;
					break;
				}
			}

			if (ansi != 0xFFFFFFFF) {
				result += ansi;
			} else {
				// There's no valid CP1252 code for this character...
			}
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////
bool StringUtil::isUtf8BOM(const byte *buffer, uint32 bufferSize) {
	if (bufferSize > 3 && buffer[0] == 0xEF && buffer[1] == 0xBB && buffer[2] == 0xBF) {
		return true;
	} else {
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
int StringUtil::indexOf(const WideString &str, const WideString &toFind, size_t startFrom) {
	return str.find(toFind, startFrom);
}

Common::String StringUtil::encodeSetting(const Common::String &str) {
	for (uint32 i = 0; i < str.size(); i++) {
		if ((str[i] < 33) || (str[i] == '=') || (str[i] > 126)) {
			error("Setting contains illegal characters: %s", str.c_str());
		}
	}
	return str;
}

Common::String StringUtil::decodeSetting(const Common::String &str) {
	return str;
}

//////////////////////////////////////////////////////////////////////////
AnsiString StringUtil::toString(int val) {
	return Common::String::format("%d", val);
}

// Mapping of CP1252 characters 0x80...0x9F into UTF-32
uint32 StringUtil::_ansiToUTF32[32] = {
	0x20AC, 0x0000, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0000, 0x017D, 0x0000,
	0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x0000, 0x017E, 0x0178
};

} // End of namespace Wintermute
