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
#include "common/debug.h"
#include "common/textconsole.h"
#include <cerrno>

namespace Common {

Encoding::Encoding(const String &to, const String &from) 
	: _to(to)
	, _from(from) {
#ifdef USE_ICONV
		String toTranslit = to + "//TRANSLIT";
		_iconvHandle = iconv_open(toTranslit.c_str(), from.c_str());
#endif // USE_ICONV
}

Encoding::~Encoding() {
#ifdef USE_ICONV
	if (_iconvHandle != (iconv_t) -1)
		iconv_close(_iconvHandle);
#endif // USE_ICONV
}

char *Encoding::convert(const char *string, size_t size) {
#ifndef USE_ICONV
	_iconvHandle = 0;
#endif
	return doConversion(_iconvHandle, _to, _from, string, size);
}

char *Encoding::convert(const String &to, const String &from, const char *string, size_t size) {
#ifdef USE_ICONV
	String toTranslit = to + "//TRANSLIT";
	iconv_t iconvHandle = iconv_open(toTranslit.c_str(), from.c_str());
#else
	iconv_t iconvHandle = 0;
#endif // USE_ICONV

	char *result =  doConversion(iconvHandle, to, from, string, size);

#ifdef USE_ICONV
	if (iconvHandle != (iconv_t) -1)
		iconv_close(iconvHandle);
#endif // USE_ICONV
	return result;
}

char *Encoding::doConversion(iconv_t iconvHandle, const String &to, const String &from, const char *string, size_t length) {
	char *result = nullptr;
#ifdef USE_ICONV
	if (iconvHandle != (iconv_t) -1)
		result = convertIconv(iconvHandle, string, length);
	else
		debug("Could not convert from %s to %s using iconv", from.c_str(), to.c_str());
	if (result == nullptr)
		debug("Error while converting with iconv");
#else
	debug("Iconv is not available");
#endif // USE_ICONV

	return result;
}

char *Encoding::convertIconv(iconv_t iconvHandle, const char *string, size_t length) {
#ifdef USE_ICONV
	debug("Trying iconv...");

	size_t inSize = length;
	size_t outSize = inSize;
	size_t stringSize = inSize > 4 ? inSize : outSize;


#ifdef ICONV_USES_CONST
	const char *src = string;
#else
	char *src = new char[length];
	char *originalSrc = src;
	memcpy(src, string, length);
#endif // ICONV_USES_CONST

	char *buffer = (char *) malloc(sizeof(char) * stringSize);
	if (!buffer) {
		warning ("Cannot allocate memory for converting string");
		return nullptr;
	}
	memset(buffer, 0, stringSize);
	char *dst = buffer;
	bool error = false;

	while (inSize > 0) {
		if (iconv(iconvHandle, &src, &inSize, &dst, &outSize) == ((size_t)-1)) {
			// from SDLs implementation of SDL_iconv_string (slightly altered)
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
				debug("iconv failed");
				break;
			}
		}
	}
	// Add a zero character to the end. Hopefuly UTF32 uses the most bytes from
	// all possible encodings, so add 4 zero bytes.
	buffer = (char *) realloc(buffer, stringSize + 4);
	memset(buffer + stringSize, 0, 4);

#ifndef ICONV_USES_CONST
	delete[] originalSrc;
#endif // ICONV_USES_CONST

	if (error)
		return nullptr;
	debug("Size: %d", stringSize);

	return buffer;
#else
	debug("Iconv isn't available");
	return nullptr;
#endif //USE_ICONV
}

}
