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

#include "engines/wintermute/dcgf.h"
#include "common/tokenizer.h"
#include "StringUtil.h"
#include "ConvertUTF.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
void StringUtil::ToLowerCase(AnsiString &str) {
	return str.toUppercase();
}

//////////////////////////////////////////////////////////////////////////
/*void StringUtil::ToLowerCase(WideString &str) {
    std::transform(str.begin(), str.end(), str.begin(), ::towlower);
}*/

//////////////////////////////////////////////////////////////////////////
void StringUtil::ToUpperCase(AnsiString &str) {
	return str.toUppercase();
}

//////////////////////////////////////////////////////////////////////////
/*void StringUtil::ToUpperCase(WideString &str) {
    std::transform(str.begin(), str.end(), str.begin(), ::towupper);
}*/

//////////////////////////////////////////////////////////////////////////
bool StringUtil::CompareNoCase(const AnsiString &str1, const AnsiString &str2) {
	AnsiString str1lc = str1;
	AnsiString str2lc = str2;

	ToLowerCase(str1lc);
	ToLowerCase(str2lc);

	return (str1lc == str2lc);
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
WideString StringUtil::Utf8ToWide(const Utf8String &Utf8Str) {
	error("StringUtil::Utf8ToWide - WideString not supported yet");
	/*  size_t WideSize = Utf8Str.size();

	    if (sizeof(wchar_t) == 2) {
	        wchar_t *WideStringNative = new wchar_t[WideSize + 1];

	        const UTF8 *SourceStart = reinterpret_cast<const UTF8 *>(Utf8Str.c_str());
	        const UTF8 *SourceEnd = SourceStart + WideSize;

	        UTF16 *TargetStart = reinterpret_cast<UTF16 *>(WideStringNative);
	        UTF16 *TargetEnd = TargetStart + WideSize + 1;

	        ConversionResult res = ConvertUTF8toUTF16(&SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
	        if (res != conversionOK) {
	            delete [] WideStringNative;
	            return L"";
	        }
	        *TargetStart = 0;
	        WideString ResultString(WideStringNative);
	        delete [] WideStringNative;

	        return ResultString;
	    } else if (sizeof(wchar_t) == 4) {
	        wchar_t *WideStringNative = new wchar_t[WideSize + 1];

	        const UTF8 *SourceStart = reinterpret_cast<const UTF8 *>(Utf8Str.c_str());
	        const UTF8 *SourceEnd = SourceStart + WideSize;

	        UTF32 *TargetStart = reinterpret_cast<UTF32 *>(WideStringNative);
	        UTF32 *TargetEnd = TargetStart + WideSize;

	        ConversionResult res = ConvertUTF8toUTF32(&SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
	        if (res != conversionOK) {
	            delete [] WideStringNative;
	            return L"";
	        }
	        *TargetStart = 0;
	        WideString ResultString(WideStringNative);
	        delete [] WideStringNative;

	        return ResultString;
	    } else {
	        return L"";
	    }*/
	return "";
}

//////////////////////////////////////////////////////////////////////////
Utf8String StringUtil::WideToUtf8(const WideString &WideStr) {
	error("StringUtil::WideToUtf8 - Widestring not supported yet");
	/*  size_t WideSize = WideStr.length();

	    if (sizeof(wchar_t) == 2) {
	        size_t Utf8Size = 3 * WideSize + 1;
	        char *Utf8StringNative = new char[Utf8Size];

	        const UTF16 *SourceStart = reinterpret_cast<const UTF16 *>(WideStr.c_str());
	        const UTF16 *SourceEnd = SourceStart + WideSize;

	        UTF8 *TargetStart = reinterpret_cast<UTF8 *>(Utf8StringNative);
	        UTF8 *TargetEnd = TargetStart + Utf8Size;

	        ConversionResult res = ConvertUTF16toUTF8(&SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
	        if (res != conversionOK) {
	            delete [] Utf8StringNative;
	            return (Utf8String)"";
	        }
	        *TargetStart = 0;
	        Utf8String ResultString(Utf8StringNative);
	        delete [] Utf8StringNative;
	        return ResultString;
	    } else if (sizeof(wchar_t) == 4) {
	        size_t Utf8Size = 4 * WideSize + 1;
	        char *Utf8StringNative = new char[Utf8Size];

	        const UTF32 *SourceStart = reinterpret_cast<const UTF32 *>(WideStr.c_str());
	        const UTF32 *SourceEnd = SourceStart + WideSize;

	        UTF8 *TargetStart = reinterpret_cast<UTF8 *>(Utf8StringNative);
	        UTF8 *TargetEnd = TargetStart + Utf8Size;

	        ConversionResult res = ConvertUTF32toUTF8(&SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
	        if (res != conversionOK) {
	            delete [] Utf8StringNative;
	            return (Utf8String)"";
	        }
	        *TargetStart = 0;
	        Utf8String ResultString(Utf8StringNative);
	        delete [] Utf8StringNative;
	        return ResultString;
	    } else {
	        return (Utf8String)"";
	    }*/
	return "";
}

// Currently this only does Ansi->ISO 8859, and only for carets.
char simpleAnsiToWide(const AnsiString &str, uint32 &offset) {
	char c = str[offset];

	if (c == 92) {
		offset++;
		return '\'';
	} else {
		offset++;
		return c;
	}
}

//////////////////////////////////////////////////////////////////////////
WideString StringUtil::AnsiToWide(const AnsiString &str) {
	// TODO: This function gets called a lot, so warnings like these drown out the usefull information
	static bool hasWarned = false;
	if (!hasWarned) {
		hasWarned = true;
		warning("StringUtil::AnsiToWide - WideString not supported yet");
	}
	Common::String converted = "";
	uint32 index = 0;
	while (index != str.size()) {
		converted += simpleAnsiToWide(str, index);
	}
	// using default os locale!

	/*  setlocale(LC_CTYPE, "");
	    size_t WideSize = mbstowcs(NULL, str.c_str(), 0) + 1;
	    wchar_t *wstr = new wchar_t[WideSize];
	    mbstowcs(wstr, str.c_str(), WideSize);
	    WideString ResultString(wstr);
	    delete [] wstr;
	    return ResultString;*/
	return WideString(converted);
}

//////////////////////////////////////////////////////////////////////////
AnsiString StringUtil::WideToAnsi(const WideString &wstr) {
	// using default os locale!
	// TODO: This function gets called a lot, so warnings like these drown out the usefull information
	static bool hasWarned = false;
	if (!hasWarned) {
		hasWarned = true;
		warning("StringUtil::WideToAnsi - WideString not supported yet");
	}
	/*  setlocale(LC_CTYPE, "");
	    size_t WideSize = wcstombs(NULL, wstr.c_str(), 0) + 1;
	    char *str = new char[WideSize];
	    wcstombs(str, wstr.c_str(), WideSize);
	    AnsiString ResultString(str);
	    delete [] str;
	    return ResultString;*/
	return AnsiString(wstr);
}

//////////////////////////////////////////////////////////////////////////
bool StringUtil::StartsWith(const AnsiString &str, const AnsiString &pattern, bool ignoreCase) {
	/*  size_t strLength = str.size();
	    size_t patternLength = pattern.size();

	    if (strLength < patternLength || patternLength == 0)
	        return false;

	    AnsiString startPart = str.substr(0, patternLength);

	    if (ignoreCase) return CompareNoCase(startPart, pattern);
	    else return (startPart == pattern);*/
	if (!ignoreCase)
		return str.hasPrefix(pattern);
	else {
		size_t strLength = str.size();
		size_t patternLength = pattern.size();

		if (strLength < patternLength || patternLength == 0)
			return false;

		AnsiString startPart(str.c_str(), patternLength);
		uint32 likeness = startPart.compareToIgnoreCase(pattern.c_str());
		return (likeness == 0);
	}
}

//////////////////////////////////////////////////////////////////////////
bool StringUtil::EndsWith(const AnsiString &str, const AnsiString &pattern, bool ignoreCase) {
	/*  size_t strLength = str.size(); // TODO: Remove
	    size_t patternLength = pattern.size();

	    if (strLength < patternLength || patternLength == 0)
	        return false;

	    AnsiString endPart = str.substr(strLength - patternLength, patternLength);

	    if (ignoreCase) return CompareNoCase(endPart, pattern);
	    else return (endPart == pattern);*/
	if (!ignoreCase) {
		return str.hasSuffix(pattern);
	} else {
		size_t strLength = str.size();
		size_t patternLength = pattern.size();

		if (strLength < patternLength || patternLength == 0)
			return false;

		Common::String endPart(str.c_str() + (strLength - patternLength), patternLength);
		uint32 likeness = str.compareToIgnoreCase(pattern.c_str());
		return (likeness != 0);
	}
}

//////////////////////////////////////////////////////////////////////////
bool StringUtil::IsUtf8BOM(const byte *Buffer, uint32 BufferSize) {
	if (BufferSize > 3 && Buffer[0] == 0xEF && Buffer[1] == 0xBB && Buffer[2] == 0xBF) return true;
	else return false;
}

//////////////////////////////////////////////////////////////////////////
AnsiString StringUtil::Replace(const AnsiString &str, const AnsiString &from, const AnsiString &to) {
	if (from.empty() || from == to) return str;

	AnsiString result = str;
	/*size_t pos = 0;*/

	while (result.contains(from)) {
		const char *startPtr = strstr(result.c_str(), from.c_str());
		uint32 index = startPtr - result.c_str();

		Common::String tail(result.c_str() + index + to.size());
		result = Common::String(result.c_str(), index);
		result += to;
		result += tail;

		/*      pos = result.find(from, pos);
		        if (pos == result.npos) break;

		        result.replace(pos, from.size(), to);
		        pos += to.size();*/
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////
AnsiString StringUtil::Trim(const AnsiString &str, bool fromLeft, bool fromRight, const AnsiString &chars) {
	AnsiString trimmedStr = str;

	if (fromRight) {
		//trimmedStr.erase(trimmedStr.find_last_not_of(chars) + 1); // TODO
		warning("fromRight-trim not implemented yet, %s", chars.c_str());
	}
	if (fromLeft) {
		uint32 lastOf = LastIndexOf(str, chars, 0);
		trimmedStr = Common::String(trimmedStr.c_str() + lastOf);
		//trimmedStr.erase(0, trimmedStr.find_first_not_of(chars));
	}
	return trimmedStr;
}

//////////////////////////////////////////////////////////////////////////
int StringUtil::IndexOf(const WideString &str, const WideString &toFind, size_t startFrom) {
	/*size_t pos = str.find(toFind, startFrom);
	if (pos == str.npos) return -1;
	else return pos;*/
	const char *index = strstr(str.c_str(), toFind.c_str());
	if (index == NULL)
		return -1;
	else
		return index - str.c_str();
}

//////////////////////////////////////////////////////////////////////////
int StringUtil::LastIndexOf(const WideString &str, const WideString &toFind, size_t startFrom) {
	/*size_t pos = str.rfind(toFind, startFrom);
	if (pos == str.npos) return -1;
	else return pos;*/
	int32 lastIndex = -1;
	bool found = false;
	for (size_t i = startFrom; i < str.size(); i++) {
		found = false;
		for (size_t j = 0; j < toFind.size(); j++) {
			if (str[i + j] != toFind[j]) {
				found = false;
				break;
			} else {
				found = true;
			}
		}
		if (found)
			lastIndex = i;
	}
	return lastIndex;
}

//////////////////////////////////////////////////////////////////////////
AnsiString StringUtil::ToString(size_t val) {
	/*  std::ostringstream str;
	    str << val;
	    return str.str();*/
	return Common::String::format("%u", (uint32)val);
}

//////////////////////////////////////////////////////////////////////////
AnsiString StringUtil::ToString(int val) {
	/*  std::ostringstream str;
	    str << val;
	    return str.str();*/
	return Common::String::format("%d", val);
}

//////////////////////////////////////////////////////////////////////////
AnsiString StringUtil::ToString(float val) {
	/*  std::ostringstream str;
	    str << val;
	    return str.str();*/
	return Common::String::format("%f", val);
}

//////////////////////////////////////////////////////////////////////////
AnsiString StringUtil::ToString(double val) {
	/*  std::ostringstream str;
	    str << val;
	    return str.str();*/
	return Common::String::format("%f", val);
}


//////////////////////////////////////////////////////////////////////////
void StringUtil::Split(const AnsiString &list, const AnsiString &delimiters, AnsiStringArray &result, bool keepEmptyItems) {
	result.clear();
//TODO: Verify this, wrt keepEmptyItems.
	Common::StringTokenizer tokenizer(list.c_str(), delimiters.c_str());
	//typedef boost::char_separator<char> separator_t;
	//typedef boost::tokenizer<separator_t, AnsiString::const_iterator, AnsiString> tokenizer_t;

	//separator_t del(delimiters.c_str(), "", keepEmptyItems ? boost::keep_empty_tokens : boost::drop_empty_tokens);
	//tokenizer_t tokens(list, del);
	while (!tokenizer.empty()) {
		Common::String copy(tokenizer.nextToken().c_str());
		result.push_back(copy);
	}
}

} // end of namespace WinterMute
