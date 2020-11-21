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

#ifndef AGS_SHARED_UTIL_STRINGTYPES_H
#define AGS_SHARED_UTIL_STRINGTYPES_H

#include <cctype>
#include <functional>
#include <unordered_map>

#include <vector>
#include "util/string.h"

namespace FNV
{

const uint32_t PRIME_NUMBER = 2166136261U;
const uint32_t SECONDARY_NUMBER = 16777619U;

inline size_t Hash(const char *data, const size_t len)
{
    uint32_t hash = PRIME_NUMBER;
    for (size_t i = 0; i < len; ++i)
        hash = (SECONDARY_NUMBER * hash) ^ (uint8_t)(data[i]);
    return hash;
}

inline size_t Hash_LowerCase(const char *data, const size_t len)
{
    uint32_t hash = PRIME_NUMBER;
    for (size_t i = 0; i < len; ++i)
        hash = (SECONDARY_NUMBER * hash) ^ (uint8_t)(tolower(data[i]));
    return hash;
}

} // namespace FNV


// A std::hash specialization for AGS String
namespace std
{
#ifdef AGS_NEEDS_TR1
namespace tr1
{
#endif
// std::hash for String object
template<>
struct hash<AGS::Common::String> : public unary_function<AGS::Common::String, size_t>
{
    size_t operator ()(const AGS::Common::String &key) const
    {
        return FNV::Hash(key.GetCStr(), key.GetLength());
    }
};
#ifdef AGS_NEEDS_TR1
}
#endif
}


namespace AGS
{
namespace Common
{

//
// Various comparison functors
//

// Test case-insensitive String equality
struct StrEqNoCase : public std::binary_function<String, String, bool>
{
    bool operator()(const String &s1, const String &s2) const
    {
        return s1.CompareNoCase(s2) == 0;
    }
};

// Case-insensitive String less
struct StrLessNoCase : public std::binary_function<String, String, bool>
{
    bool operator()(const String &s1, const String &s2) const
    {
        return s1.CompareNoCase(s2) < 0;
    }
};

// Compute case-insensitive hash for a String object
struct HashStrNoCase : public std::unary_function<String, size_t>
{
    size_t operator ()(const String &key) const
    {
        return FNV::Hash_LowerCase(key.GetCStr(), key.GetLength());
    }
};

typedef std::vector<String> StringV;
typedef std::unordered_map<String, String> StringMap;
typedef std::unordered_map<String, String, HashStrNoCase, StrEqNoCase> StringIMap;

} // namespace Common
} // namespace AGS

#endif
