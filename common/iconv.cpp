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

#include "common/str.h"
#include "common/textconsole.h"
#include "common/ustr.h"

namespace Common {

U32String convertToU32String(const char *fromCode, const String &string) {
	// Apparently UTF-32 isn't native endian. Also, UCS−4−INTERNAL fails
	// for me.
	iconv_t handle = iconv_open(
#ifdef SCUMM_BIG_ENDIAN
		"UTF-32BE",
#else
		"UTF-32LE",
#endif
		fromCode);
	if (handle == ((iconv_t)-1))
		error("Failed to initialize UTF-32 conversion from %s", fromCode);

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
		error("Failed to convert %s to UTF-32 string", fromCode);

	U32String output((const U32String::value_type *)buffer, (originalOutSize - outSize) / 4);

	delete[] buffer;
	iconv_close(handle);

	return output;
}

} // End of namespace Common
