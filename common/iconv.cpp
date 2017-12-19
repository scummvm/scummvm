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

#include <iconv.h>

#include "common/iconv.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "common/ustr.h"

namespace Common {

static const char **getIconvCodesForEncoding(Encoding encoding) {
	static const char *encodingCP932Codes[]  = { "cp932",  nullptr };
	static const char *encodingCP1250Codes[] = { "cp1250", nullptr };
	static const char *encodingCP1251Codes[] = { "cp1251", nullptr };
	static const char *encodingCP1252Codes[] = { "cp1252", nullptr };
	static const char *encodingCP1255Codes[] = { "cp1255", nullptr };

	// libiconv and glibc-iconv don't use the same name for that encoding, we have to try both
	static const char *encodingMacCentralEuropeCodes[] = { "maccentraleurope", "mac-centraleurope", nullptr };

	switch (encoding) {
		case kEncodingCP932:  return encodingCP932Codes;
		case kEncodingCP1250: return encodingCP1250Codes;
		case kEncodingCP1251: return encodingCP1251Codes;
		case kEncodingCP1252: return encodingCP1252Codes;
		case kEncodingCP1255: return encodingCP1255Codes;
		case kEncodingMacCentralEurope: return encodingMacCentralEuropeCodes;
	}

	error("Failed look up iconv codes for encoding '%d'", encoding);
}

U32String convertToU32String(Encoding fromEncoding, const String &string) {
	const char **fromCodes = getIconvCodesForEncoding(fromEncoding);

	// Apparently UTF-32 isn't native endian. Also, UCS−4−INTERNAL fails
	// for me.
	static const char *toCode =
#ifdef SCUMM_BIG_ENDIAN
		"UTF-32BE";
#else
		"UTF-32LE";
#endif

	iconv_t handle;
	do {
		handle = iconv_open(toCode, *fromCodes);

		if (handle == (iconv_t)-1) {
			fromCodes++;
		}
	} while (handle == ((iconv_t)-1) && *fromCodes);

	if (handle == ((iconv_t)-1))
		error("Failed to initialize UTF-32 conversion from %d", fromEncoding);

	size_t inSize = string.size();
	size_t outSize = inSize * 4; // Approximation
	size_t originalOutSize = outSize;

	char *buffer = new char[outSize];
	char *dst = buffer;

#ifdef ICONV_USES_CONST
	const char *src = string.c_str();
#else
	char *src = const_cast<char *>(string.c_str());
#endif

	if (iconv(handle, &src, &inSize, &dst, &outSize) == ((size_t)-1))
		error("Failed to convert %s to UTF-32 string", *fromCodes);

	// The conversion descriptor may still contain some state. Write it to the output buffer.
	if (iconv(handle, nullptr, nullptr, &dst, &outSize) == ((size_t)-1))
		error("Failed to convert the remaining state of %s to UTF-32 string", *fromCodes);

	U32String output((const U32String::value_type *)buffer, (originalOutSize - outSize) / 4);

	delete[] buffer;
	iconv_close(handle);

	return output;
}

} // End of namespace Common
