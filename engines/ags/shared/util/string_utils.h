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

#ifndef AGS_SHARED_UTIL_STRING_UTILS_H
#define AGS_SHARED_UTIL_STRING_UTILS_H

#include "util/string.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

//=============================================================================

// Converts char* to string and frees original malloc-ed array;
// This is used when we get a malloc'd char array from some utility function.
Common::String cbuf_to_string_and_free(char *char_buf);

namespace AGS {
namespace Shared {
namespace StrUtil {
enum ConversionError {
	kNoError,   // conversion successful
	kFailed,    // conversion failed (e.g. wrong format)
	kOutOfRange // the resulting value is out of range
};

// Convert integer to string, by printing its value
String          IntToString(int val);
// Tries to convert whole string into integer value;
// returns def_val on failure
int             StringToInt(const String &s, int def_val = 0);
// Tries to convert whole string into integer value;
// Returns error code if any non-digit character was met or if value is out
// of range; the 'val' variable will be set with resulting integer, or
// def_val on failure
ConversionError StringToInt(const String &s, int &val, int def_val);

// Serialize and unserialize unterminated string prefixed with 32-bit length;
// length is presented as 32-bit integer integer
String          ReadString(Stream *in);
void            ReadString(char *cstr, Stream *in, size_t buf_limit);
void            ReadString(char **cstr, Stream *in);
void            ReadString(String &s, Stream *in);
void            SkipString(Stream *in);
void            WriteString(const String &s, Stream *out);
void            WriteString(const char *cstr, Stream *out);

// Serialize and unserialize string as c-string (null-terminated sequence)
void            ReadCStr(char *buf, Stream *in, size_t buf_limit);
void            SkipCStr(Stream *in);
void            WriteCStr(const char *cstr, Stream *out);
void            WriteCStr(const String &s, Stream *out);
}
} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
