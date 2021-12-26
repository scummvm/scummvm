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

//=============================================================================
//
// Class, depicting version of the AGS engine
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_VERSION_H
#define AGS_SHARED_UTIL_VERSION_H

#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

using Shared::String;

struct Version {
	int32_t Major;
	int32_t Minor;
	int32_t Release;
	int32_t Revision;
	String  Special;
	String  BuildInfo;

	String  LongString;
	String  ShortString;
	String  BackwardCompatibleString;

	// Last engine version, using different version format than AGS Editor (3.22.1120 / 3.2.2.1120)
	static const Version LastOldFormatVersion;

	Version();
	Version(int32_t major, int32_t minor, int32_t release);
	Version(int32_t major, int32_t minor, int32_t release, int32_t revision);
	Version(int32_t major, int32_t minor, int32_t release, int32_t revision, const String &special);
	Version(int32_t major, int32_t minor, int32_t release, int32_t revision, const String &special, const String &build_info);
	Version(const String &version_string);

	inline int32_t AsNumber() const {
		return Major * 10000 + Minor * 100 + Release;
	}

	inline int64_t AsLongNumber() const {
		return (int64_t)Major * 100000000L + (int64_t)Minor * 1000000L + (int64_t)Release * 10000L + Revision;
	}

	inline int32_t AsSmallNumber() const {
		return Major * 100 + Minor;
	}

	void SetFromString(const String &version_string);

	inline bool operator < (const Version &other) const {
		return AsLongNumber() < other.AsLongNumber();
	}

	inline bool operator <= (const Version &other) const {
		return AsLongNumber() <= other.AsLongNumber();
	}

	inline bool operator > (const Version &other) const {
		return AsLongNumber() > other.AsLongNumber();
	}

	inline bool operator >= (const Version &other) const {
		return AsLongNumber() >= other.AsLongNumber();
	}

	inline bool operator == (const Version &other) const {
		return AsLongNumber() == other.AsLongNumber();
	}

	inline bool operator != (const Version &other) const {
		return AsLongNumber() != other.AsLongNumber();
	}

private:
	void MakeString();
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
