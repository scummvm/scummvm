/*
This file is part of WME Lite.
http://dead-code.org/redir.php?target=wmelite

Copyright (c) 2011 Jan Nedoma

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef __WmeStringUtil_H__
#define __WmeStringUtil_H__

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
