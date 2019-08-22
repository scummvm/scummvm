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

#include "common/encoding.h"
#include "common/textconsole.h"
#include "common/system.h"
#include "common/translation.h"
#include "common/endian.h"
#include <errno.h>

namespace Common {

Encoding::Encoding(const String &to, const String &from) 
	: _to(to)
	, _from(from) {
		_iconvHandle = initIconv(to, from);
}

Encoding::~Encoding() {
	deinitIconv(_iconvHandle);
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

String Encoding::addUtfEndianness(const String &str) {
	if (str.equalsIgnoreCase("utf-16") || str.equalsIgnoreCase("utf-32")) {
#ifdef SCUMM_BIG_ENDIAN
		return str + "BE";
#else
		return str + "LE";
#endif
	} else
		return String(str);
}

iconv_t Encoding::initIconv(const String &to, const String &from) {
#ifdef USE_ICONV
		String toTranslit = addUtfEndianness(to) + "//TRANSLIT";
		return iconv_open(toTranslit.c_str(),
							addUtfEndianness(from).c_str());
#else
		return 0;
#endif // USE_ICONV
}

void Encoding::deinitIconv(iconv_t iconvHandle) {
#ifdef USE_ICONV
	if (iconvHandle != (iconv_t) -1)
		iconv_close(iconvHandle);
#endif // USE_ICONV
}

void Encoding::setFrom(const String &from) {
	deinitIconv(_iconvHandle);
	_from = from;
	_iconvHandle = initIconv(_to, _from);
}

void Encoding::setTo(const String &to) {
	deinitIconv(_iconvHandle);
	_to = to;
	_iconvHandle = initIconv(_to, _from);
}

char *Encoding::convert(const char *string, size_t size) {
	return convertWithTransliteration(_iconvHandle, _to, _from, string, size);
}

char *Encoding::convert(const String &to, const String &from, const char *string, size_t size) {
	iconv_t iconvHandle = initIconv(to, from);

	char *result = convertWithTransliteration(iconvHandle, to, from, string, size);

	deinitIconv(iconvHandle);
	return result;
}

char *Encoding::convertWithTransliteration(iconv_t iconvHandle, const String &to, const String &from, const char *string, size_t length) {
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
			iconv_t tmpHandle = initIconv("UTF-32", from);
			tmpString = conversion(tmpHandle, "UTF-32", from, string, length);
			deinitIconv(tmpHandle);
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
	iconv_t newHandle = iconvHandle;
	if (newFrom != from)
		newHandle = initIconv(to, newFrom);
	char *result;
	if (newString != nullptr) {
		result = conversion(newHandle, to, newFrom, newString, newLength);
		free(newString);
	} else
		result = conversion(newHandle, to, newFrom, string, newLength);

	if (newFrom != from)
		deinitIconv(newHandle);
	return result;
}

char *Encoding::conversion(iconv_t iconvHandle, const String &to, const String &from, const char *string, size_t length) {
	char *result = nullptr;
#ifdef USE_ICONV
	if (iconvHandle != (iconv_t) -1)
		result = convertIconv(iconvHandle, string, length);
#endif // USE_ICONV
	if (result == nullptr)
		result = g_system->convertEncoding(addUtfEndianness(to).c_str(),
				addUtfEndianness(from).c_str(), string, length);

	if (result == nullptr) {
		result = convertTransManMapping(addUtfEndianness(to).c_str(), addUtfEndianness(from).c_str(), string, length);
	}

	return result;
}

char *Encoding::convertIconv(iconv_t iconvHandle, const char *string, size_t length) {
#ifdef USE_ICONV

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

}
