//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
//
//
//=============================================================================
#ifndef __AGS_CN_UTIL__STRINGUTILS_H
#define __AGS_CN_UTIL__STRINGUTILS_H

#include "util/string.h"

namespace AGS { namespace Common { class Stream; } }
using namespace AGS; // FIXME later

//=============================================================================

// Converts char* to string and frees original malloc-ed array;
// This is used when we get a malloc'd char array from some utility function.
Common::String cbuf_to_string_and_free(char *char_buf);

namespace AGS
{
namespace Common
{
namespace StrUtil
{
    enum ConversionError
    {
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
} // namespace Common
} // namespace AGS


#endif // __AGS_CN_UTIL__STRINGUTILS_H
