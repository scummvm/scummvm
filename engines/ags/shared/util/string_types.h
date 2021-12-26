/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AGS_SHARED_UTIL_STRING_TYPES_H
#define AGS_SHARED_UTIL_STRING_TYPES_H

#include "ags/lib/std/map.h"
#include "ags/lib/std/vector.h"
#include "ags/shared/util/string.h"
#include "common/hash-str.h"

namespace AGS3 {
namespace FNV {

const uint32_t PRIME_NUMBER = 2166136261U;
const uint32_t SECONDARY_NUMBER = 16777619U;

inline size_t Hash(const char *data, const size_t len) {
	uint32_t hash = PRIME_NUMBER;
	for (size_t i = 0; i < len; ++i)
		hash = (SECONDARY_NUMBER * hash) ^ (uint8_t)(data[i]);
	return hash;
}

inline size_t Hash_LowerCase(const char *data, const size_t len) {
	uint32_t hash = PRIME_NUMBER;
	for (size_t i = 0; i < len; ++i)
		hash = (SECONDARY_NUMBER * hash) ^ (uint8_t)(tolower(data[i]));
	return hash;
}

} // namespace FNV
} // namespace AGS3

namespace AGS3 {

struct CaseSensitiveString_EqualTo {
	bool operator()(const ::AGS3::AGS::Shared::String &x, const ::AGS3::AGS::Shared::String &y) const {
		return x.Compare(y) == 0;
	}
};

struct CaseSensitiveString_Hash {
	uint operator()(const ::AGS3::AGS::Shared::String &x) const {
		return Common::hashit(x.GetCStr());
	}
};

struct IgnoreCase_EqualTo {
	bool operator()(const ::AGS3::AGS::Shared::String &x, const ::AGS3::AGS::Shared::String &y) const {
		return x.CompareNoCase(y) == 0;
	}
};

struct IgnoreCase_Hash {
	uint operator()(const ::AGS3::AGS::Shared::String &x) const {
		return Common::hashit_lower(x.GetCStr());
	}
};

struct IgnoreCase_LessThan {
	bool operator()(const ::AGS3::AGS::Shared::String &x, const ::AGS3::AGS::Shared::String &y) const {
		return x.CompareNoCase(y) < 0;
	}
};

} // namespace AGS3

namespace Common {

// Specalization of the Hash functor for String objects.
// We do case sensitve hashing here, because that is what
// the default EqualTo is compatible with. If one wants to use
// case insensitve hashing, then only because one wants to use
// IgnoreCase_EqualTo, and then one has to specify a custom
// hash anyway.
template<>
struct Hash<AGS3::AGS::Shared::String> {
	uint operator()(const AGS3::AGS::Shared::String &s) const {
		return Common::hashit(s.GetCStr());
	}
};

} // namespace Common

namespace AGS3 {
namespace AGS {
namespace Shared {

typedef std::vector<String> StringV;
typedef std::unordered_map<String, String> StringMap;
typedef std::unordered_map<String, String, IgnoreCase_Hash, IgnoreCase_EqualTo> StringIMap;

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
