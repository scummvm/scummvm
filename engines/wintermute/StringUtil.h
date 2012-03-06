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
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
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

#ifndef WINTERMUTE_STRINGUTIL_H
#define WINTERMUTE_STRINGUTIL_H

#include "PlatformSDL.h"

namespace WinterMute {

class StringUtil {
public:
	static void ToLowerCase(AnsiString &str);
	static void ToLowerCase(WideString &str);
	static void ToUpperCase(AnsiString &str);
	static void ToUpperCase(WideString &str);
	static bool CompareNoCase(const AnsiString &str1, const AnsiString &str2);
	static bool CompareNoCase(const WideString &str1, const WideString &str2);
	static WideString Utf8ToWide(const Utf8String &Utf8Str);
	static Utf8String WideToUtf8(const WideString &WideStr);
	static WideString AnsiToWide(const AnsiString &str);
	static AnsiString WideToAnsi(const WideString &str);

	static bool StartsWith(const AnsiString &str, const AnsiString &pattern, bool ignoreCase = false);
	static bool EndsWith(const AnsiString &str, const AnsiString &pattern, bool ignoreCase = false);

	static bool IsUtf8BOM(const byte *buffer, uint32 bufferSize);

	static AnsiString Replace(const AnsiString &str, const AnsiString &from, const AnsiString &to);
	static AnsiString Trim(const AnsiString &str, bool fromLeft = true, bool fromRight = true, const AnsiString &chars = " \n\r\t");

	static int IndexOf(const WideString &str, const WideString &toFind, size_t startFrom);
	static int LastIndexOf(const WideString &str, const WideString &toFind, size_t startFrom);

	static AnsiString ToString(size_t val);
	static AnsiString ToString(int val);
	static AnsiString ToString(float val);
	static AnsiString ToString(double val);

	static void Split(const AnsiString &list, const AnsiString &delimiters, AnsiStringArray &result, bool keepEmptyItems = false);
};

} // end of namespace WinterMute

#endif // __WmeStringUtil_H__
