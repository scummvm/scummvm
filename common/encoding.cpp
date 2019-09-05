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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/encoding.h"
#include "common/textconsole.h"
#include "common/system.h"
#include "common/translation.h"
#include "common/endian.h"
#include <errno.h>

#ifdef USE_ICONV

#include <iconv.h>

#endif // USE_ICONV

namespace Common {

String addUtfEndianness(const String &str) {
	if (str.equalsIgnoreCase("utf-16") || str.equalsIgnoreCase("utf-32")) {
#ifdef SCUMM_BIG_ENDIAN
		return str + "BE";
#else
		return str + "LE";
#endif
	} else
		return String(str);
}

Encoding::Encoding(const String &to, const String &from) 
	: _to(to)
	, _from(from) {
}

char *Encoding::switchEndian(const char *string, int length, int bitCount) {
	assert(bitCount % 8 == 0);
	assert(length % (bitCount / 8) == 0);
	char *newString = (char *) malloc(length);
	if (!newString) {
		warning("Could not allocate memory for string conversion");
		return nullptr;
	}
	if (bitCount == 16) {
		int characterCount = length / 2;
		for(int i = 0; i < characterCount ; i++)
			((uint16 *) newString)[i] = SWAP_BYTES_16(((const uint16 *) string)[i]);
		return newString;
	} else if (bitCount == 32) {
		int characterCount = length / 4;
		for(int i = 0; i < characterCount ; i++)
			((uint32 *) newString)[i] = SWAP_BYTES_32(((const uint32 *) string)[i]);
		return newString;
	} else {
		return nullptr;
	}
}

char *Encoding::convert(const char *string, size_t size) {
	return convertWithTransliteration(_to, _from, string, size);
}

char *Encoding::convert(const String &to, const String &from, const char *string, size_t size) {
	return convertWithTransliteration(to, from, string, size);
}

char *Encoding::convertWithTransliteration(const String &to, const String &from, const char *string, size_t length) {
	if (from.equalsIgnoreCase(to)) {
		// don't convert, just copy the string and return it
		char *result = (char *) calloc(sizeof(char), length + 4);
		if (!result) {
			warning("Could not allocate memory for string conversion");
			return nullptr;
		}
		memcpy(result, string, length);
		return result;
	}
	if ((addUtfEndianness(to).equalsIgnoreCase("utf-16be") &&
		addUtfEndianness(from).equalsIgnoreCase("utf-16le")) ||
		(addUtfEndianness(to).equalsIgnoreCase("utf-16le") &&
		addUtfEndianness(from).equalsIgnoreCase("utf-16be")) ||
		(addUtfEndianness(to).equalsIgnoreCase("utf-32be") &&
		addUtfEndianness(from).equalsIgnoreCase("utf-32le")) ||
		(addUtfEndianness(to).equalsIgnoreCase("utf-32le") &&
		addUtfEndianness(from).equalsIgnoreCase("utf-32be")))
	{
		// The encoding is the same, we just need to switch the endianness
		if (to.hasPrefixIgnoreCase("utf-16"))
			return switchEndian(string, length, 16);
		else
			return switchEndian(string, length, 32);
	}
	char *newString = nullptr;
	String newFrom = from;
	size_t newLength = length;
	if (from.equalsIgnoreCase("iso-8859-5") &&
			!to.hasPrefixIgnoreCase("utf")) {
		// There might be some cyrillic characters, which need to be transliterated.
		newString = transliterateCyrillic(string);
		if (!newString)
			return nullptr;
		newFrom = "ASCII";
	}
	if (from.hasPrefixIgnoreCase("utf") &&
			!to.hasPrefixIgnoreCase("utf") &&
			!to.equalsIgnoreCase("iso-8859-5")) {
		// There might be some cyrillic characters, which need to be transliterated.
		char *tmpString;
		if (from.hasPrefixIgnoreCase("utf-32"))
			tmpString = nullptr;
		else {
			tmpString = conversion("UTF-32", from, string, length);
			if (!tmpString)
				return nullptr;
			// find out the length in bytes of the tmpString
			int i;
			for (i = 0; ((const uint32 *)tmpString)[i]; i++) {}
			newLength = i * 4;
			newFrom = "UTF-32";
		}
		if (tmpString != nullptr) {
			newString = (char *) transliterateUTF32((const uint32 *) tmpString, newLength);
			free(tmpString);
		} else
			newString = (char *) transliterateUTF32((const uint32 *) string, newLength);
		if (!newString)
			return nullptr;
	}
	char *result;
	if (newString != nullptr) {
		result = conversion(to, newFrom, newString, newLength);
		free(newString);
	} else
		result = conversion(to, newFrom, string, newLength);
	return result;
}

char *Encoding::conversion(const String &to, const String &from, const char *string, size_t length) {
	char *result = nullptr;
#ifdef USE_ICONV
	result = convertIconv(addUtfEndianness(to).c_str(), addUtfEndianness(from).c_str(), string, length);
#endif // USE_ICONV
	if (result == nullptr)
		result = g_system->convertEncoding(addUtfEndianness(to).c_str(),
				addUtfEndianness(from).c_str(), string, length);

	if (result == nullptr) {
		result = convertTransManMapping(addUtfEndianness(to).c_str(), addUtfEndianness(from).c_str(), string, length);
	}

	if (result == nullptr) {
		result = convertConversionTable(addUtfEndianness(to).c_str(), addUtfEndianness(from).c_str(), string, length);
	}

	return result;
}

char *Encoding::convertIconv(const char *to, const char *from, const char *string, size_t length) {
#ifdef USE_ICONV

	String toTranslit = String(to) + "//TRANSLIT";
	iconv_t iconvHandle = iconv_open(toTranslit.c_str(), from);
	if (iconvHandle == (iconv_t) -1)
		return nullptr;

	size_t inSize = length;
	size_t outSize = inSize;
	size_t stringSize = inSize > 4 ? inSize : 4;


#ifdef ICONV_USES_CONST
	const char *src = string;
#else
	char *src = new char[length];
	char *originalSrc = src;
	memcpy(src, string, length);
#endif // ICONV_USES_CONST

	char *buffer = (char *) calloc(sizeof(char), stringSize);
	if (!buffer) {
		warning ("Cannot allocate memory for converting string");
		return nullptr;
	}
	char *dst = buffer;
	bool error = false;

	while (inSize > 0) {
		if (iconv(iconvHandle, &src, &inSize, &dst, &outSize) == ((size_t)-1)) {
			// from SDL's implementation of SDL_iconv_string (slightly altered)
			if (errno == E2BIG) {
				char *oldString = buffer;
				stringSize *= 2;
				buffer = (char *) realloc(buffer, stringSize);
				if (!buffer) {
					warning ("Cannot allocate memory for converting string");
					error = true;
					break;
				}
				dst = buffer + (dst - oldString);
				outSize = stringSize - (dst - buffer);
				memset(dst, 0, stringSize / 2);
			} else {
				error = true;
				break;
			}
		}
	}
	iconv(iconvHandle, NULL, NULL, &dst, &outSize);
	// Add a zero character to the end. Hopefuly UTF32 uses the most bytes from
	// all possible encodings, so add 4 zero bytes.
	buffer = (char *) realloc(buffer, stringSize + 4);
	memset(buffer + stringSize, 0, 4);

#ifndef ICONV_USES_CONST
	delete[] originalSrc;
#endif // ICONV_USES_CONST

	iconv_close(iconvHandle);
	if (error) {
		if (buffer)
			free(buffer);
		return nullptr;
	}
	return buffer;
#else
	return nullptr;
#endif //USE_ICONV
}

// This algorithm is able to convert only between the current TransMan charset
// and UTF-32, but if it fails, it tries to at least convert from the current
// TransMan encoding to UTF-32 and then it calls convert() again with that.
char *Encoding::convertTransManMapping(const char *to, const char *from, const char *string, size_t length) {
#ifdef USE_TRANSLATION
	String currentCharset = TransMan.getCurrentCharset();
	if (currentCharset.equalsIgnoreCase(from)) {
		// We can use the transMan mapping directly
		uint32 *partialResult = (uint32 *) calloc(sizeof(uint32), (length + 1));
		if (!partialResult) {
			warning("Couldn't allocate memory for encoding conversion");
			return nullptr;
		}
		const uint32 *mapping = TransMan.getCharsetMapping();
		if (mapping == 0) {
			for(unsigned i = 0; i < length; i++) {
				partialResult[i] = string[i];
			}
		} else {
			for(unsigned i = 0; i < length; i++) {
				partialResult[i] = mapping[(unsigned char) string[i]] & 0x7FFFFFFF;
			}
		}
		char *finalResult = convert(to, "UTF-32", (char *) partialResult, length * 4);
		free(partialResult);
		return finalResult;
	} else if (currentCharset.equalsIgnoreCase(to) && String(from).hasPrefixIgnoreCase("utf-32")) {
		bool swapEndian = false;
		char *newString = nullptr;

#ifdef SCUMM_BIG_ENDIAN
		if (String(from).hasSuffixIgnoreCase("LE"))
			swapEndian = true;
#else
		if (String(from).hasSuffixIgnoreCase("BE"))
			swapEndian = true;
#endif
		if (swapEndian) {
			if (String(from).hasPrefixIgnoreCase("utf-16"))
				newString = switchEndian(string, length, 16);
			if (String(from).hasPrefixIgnoreCase("utf-32"))
				newString = switchEndian(string, length, 32);
			if (newString != nullptr)
				string = newString;
			else
				return nullptr;
		}
		// We can do reverse mapping
		const uint32 *mapping = TransMan.getCharsetMapping();
		const uint32 *src = (const uint32 *) string;
		char *result = (char *) calloc(sizeof(char), (length + 4));
		if (!result) {
			warning("Couldn't allocate memory for encoding conversion");
			if (newString != nullptr)
				free(newString);
			return nullptr;
		}
		for (unsigned i = 0; i < length; i++) {
			for (int j = 0; j < 256; j++) {
				if ((mapping[j] & 0x7FFFFFFF) == src[i]) {
					result[i] = j;
					break;
				}
			}
		}
		if (newString != nullptr)
			free(newString);
		return result;
	} else
		return nullptr;
#else
	return nullptr;
#endif // USE_TRANSLATION
}

static uint32 g_cp850ConversionTable[] = {
	0x0000, 0x263A, 0x263B, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022,
	0x25d8, 0x25CB, 0x25D9, 0x2642, 0x2640, 0x266A, 0x266B, 0x263C,
	0x25BA, 0x25C4, 0x2195, 0x203C, 0x00B6, 0x00A7, 0x25AC, 0x21A8,
	0x2191, 0x2193, 0x2192, 0x2190, 0x221F, 0x2194, 0x25B2, 0x25BC,
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
	0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
	0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
	0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
	0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
	0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
	0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x2302,

	0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
	0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
	0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,
	0x00FF, 0x00D6, 0x00DC, 0x00F8, 0x00A3, 0x00D8, 0x00D7, 0x0192,
	0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,
	0x00BF, 0x00AE, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
	0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x00C1, 0x00C2, 0x00C0,
	0x00A9, 0x2563, 0x2551, 0x2557, 0x255D, 0x00A2, 0x00A5, 0x2510,
	0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x00E3, 0x00C3,
	0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x00A4,
	0x00F0, 0x00D0, 0x00CA, 0x00CB, 0x00C8, 0x0131, 0x00CD, 0x00CE,
	0x00CF, 0x2518, 0x250C, 0x2588, 0x2584, 0x00A6, 0x00CC, 0x2580,
	0x00D3, 0x00DF, 0x00D4, 0x00D2, 0x00F5, 0x00D5, 0x00B5, 0x00FE,
	0x00DE, 0x00DA, 0x00DB, 0x00D9, 0x00FD, 0x00DD, 0x00AF, 0x00B4,
	0x00AD, 0x00B1, 0x2017, 0x00BE, 0x00B6, 0x00A7, 0x00F7, 0x00B8,
	0x00B0, 0x00A8, 0x00B7, 0x00B9, 0x00B3, 0x00B2, 0x25A0, 0x00A0
};

static uint32 g_cp437ConversionTable[] = {
	0x0000, 0x263A, 0x263B, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022,
	0x25d8, 0x25CB, 0x25D9, 0x2642, 0x2640, 0x266A, 0x266B, 0x263C,
	0x25BA, 0x25C4, 0x2195, 0x203C, 0x00B6, 0x00A7, 0x25AC, 0x21A8,
	0x2191, 0x2193, 0x2192, 0x2190, 0x221F, 0x2194, 0x25B2, 0x25BC,
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
	0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
	0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
	0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
	0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
	0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
	0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x2302,

	0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
	0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
	0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,
	0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x20A7, 0x0192,
	0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,
	0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
	0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
	0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
	0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
	0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
	0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
	0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
	0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x00B5, 0x03C4,
	0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
	0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248,
	0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0
};

struct ConversionTable {
	const char *name;
	uint32 *table;
};

const ConversionTable g_encodingConversionTables[] = {
	{"cp850", g_cp850ConversionTable},
	{"cp437", g_cp437ConversionTable},
	{nullptr, nullptr}
};

char *Encoding::convertConversionTable(const char *to, const char *from, const char *string, size_t length) {
	uint32 *table = nullptr;
	for (const ConversionTable *i = g_encodingConversionTables; i->name != nullptr; i++) {
		if (String(from).equalsIgnoreCase(i->name)) {
			table = i->table;
		}
	}
	if (table != nullptr) {
		uint32 *utf32Result = (uint32 *) calloc(sizeof(uint32), length + 1);
		if (!utf32Result) {
			warning("Could not allocate memory for encoding conversion");
			return nullptr;
		}
		for (unsigned i = 0; i < length; i++) {
			utf32Result[i] = table[(unsigned char) string[i]];
		}
		char *finalResult = convert(to, "utf-32", (char *)utf32Result, length * 4);
		free(utf32Result);
		return finalResult;
	}

	for (const ConversionTable *i = g_encodingConversionTables; i->name != nullptr; i++) {
		if (String(to).equalsIgnoreCase(i->name)) {
			table = i->table;
		}
	}
	if (table != nullptr) {
		uint32 *utf32Result = (uint32 *) convert("utf-32", from, string, length);
		if (String(from).hasPrefixIgnoreCase("utf-16"))
			length /= 2;
		if (String(from).hasPrefixIgnoreCase("utf-32"))
			length /= 4;
		char *finalResult = (char *) calloc(sizeof(char), length +1);
		if (!finalResult) {
			warning("Could not allocate memory for encoding conversion");
			return nullptr;
		}
		for (unsigned i = 0; i < length; i++) {
			for (unsigned j = 0; j < 257; j++) {
				if (j == 256) {
					// We have some character, that isn't a part of cp850, so
					// we replace it with '?' to remain consistent with iconv
					// and SDL
					finalResult[i] = '?';
				} else if (utf32Result[i] == table[j]){
					finalResult[i] = j;
					break;
				}
			}
		}
		free(utf32Result);
		return finalResult;
	}
	return nullptr;
}

static char g_cyrillicTransliterationTable[] = {
	' ', 'E', 'D', 'G', 'E', 'Z', 'I', 'I', 'J', 'L', 'N', 'C', 'K', '-', 'U', 'D',
	'A', 'B', 'V', 'G', 'D', 'E', 'Z', 'Z', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'R', 'S', 'T', 'U', 'F', 'H', 'C', 'C', 'S', 'S', '\"', 'Y', '\'', 'E', 'U', 'A',
	'a', 'b', 'v', 'g', 'd', 'e', 'z', 'z', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
	'r', 's', 't', 'u', 'f', 'h', 'c', 'c', 's', 's', '\"', 'y', '\'', 'e', 'u', 'a',
	'N', 'e', 'd', 'g', 'e', 'z', 'i', 'i', 'j', 'l', 'n', 'c', 'k', '?', 'u', 'd',
};

char *Encoding::transliterateCyrillic(const char *string) {
	char *result = (char *) malloc(strlen(string) + 1);
	if (!result) {
		warning("Could not allocate memory for encoding conversion");
		return nullptr;
	}
	for(unsigned i = 0; i <= strlen(string); i++) {
		if ((unsigned char) string[i] >= 160)
			result[i] = g_cyrillicTransliterationTable[(unsigned char) string[i] - 160];
		else
			result[i] = string[i];
	}
	return result;
}

uint32 *Encoding::transliterateUTF32(const uint32 *string, size_t length) {
	uint32 *result = (uint32 *) malloc(length + 4);
	if (!result) {
		warning("Could not allocate memory for encoding conversion");
		return nullptr;
	}
	for(unsigned i = 0; i <= length / 4; i++) {
		if (string[i] >= 0x410 && string[i] <= 0x450)
			result[i] = g_cyrillicTransliterationTable[string[i] - 160 - 864];
		else
			result[i] = string[i];
	}
	return result;
}

size_t Encoding::stringLength(const char *string, const String &encoding) {
	if (encoding.hasPrefixIgnoreCase("UTF-16")) {
		const uint16 *i = (const uint16 *) string;
		for (;*i != 0; i++) {}
		return (const char *) i - string;
	} else if (encoding.hasPrefixIgnoreCase("UTF-32")) {
		const uint32 *i = (const uint32 *) string;
		for (;*i != 0; i++) {}
		return (const char *) i - string;
	} else {
		const char *i = string;
		for (;*i != 0; i++) {}
		return i - string;
	}
}

}
