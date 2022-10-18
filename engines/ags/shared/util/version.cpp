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

#include "common/util.h"
#include "ags/shared/util/version.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

Version::Version()
	: Major(0)
	, Minor(0)
	, Release(0)
	, Revision(0) {
	MakeString();
}

Version::Version(int32_t major, int32_t minor, int32_t release)
	: Major(major)
	, Minor(minor)
	, Release(release)
	, Revision(0) {
	MakeString();
}

Version::Version(int32_t major, int32_t minor, int32_t release, int32_t revision)
	: Major(major)
	, Minor(minor)
	, Release(release)
	, Revision(revision) {
	MakeString();
}

Version::Version(int32_t major, int32_t minor, int32_t release, int32_t revision, const String &special)
	: Major(major)
	, Minor(minor)
	, Release(release)
	, Revision(revision)
	, Special(special) {
	MakeString();
}

Version::Version(int32_t major, int32_t minor, int32_t release, int32_t revision, const String &special, const String &build_info)
	: Major(major)
	, Minor(minor)
	, Release(release)
	, Revision(revision)
	, Special(special)
	, BuildInfo(build_info) {
	MakeString();
}

Version::Version(const String &version_string)
	: Major(0)
	, Minor(0)
	, Release(0)
	, Revision(0) {
	SetFromString(version_string);
}

void Version::SetFromString(const String &version_string) {
	Major = version_string.LeftSection('.').ToInt();
	String second_section = version_string.Section('.', 1, 1);
	Minor = second_section.ToInt();
	String third_section = version_string.Section('.', 2, 2);
	String fourth_section = version_string.Section('.', 3, 3);
	String revision_section;

	bool old_version_format = Major < 3 || fourth_section.IsEmpty();
	if (old_version_format) {
		if (second_section.GetLength() > 1) {
			Release = Minor % 10;
			Minor /= 10;
		} else {
			Release = 0;
		}
		revision_section = third_section;
	} else {
		Release = third_section.ToInt();
		revision_section = fourth_section;
	}

	int revision_length = 0;
	if (!revision_section.IsEmpty()) {
		const char *seek_ptr = revision_section.GetCStr();
		const char *end_ptr = revision_section.GetCStr() + revision_section.GetLength();
		while (seek_ptr != end_ptr) {
			if (!Common::isDigit(*seek_ptr)) {
				break;
			}
			revision_length++;
			seek_ptr++;
		}
	}

	Revision = revision_section.Left(revision_length).ToInt();
	// In old version format a special tag was added right after revision digits.
	// In new version format a special tag is separated from revision digits with single space char.
	Special = revision_section.Mid(revision_length + (old_version_format ? 0 : 1));

	MakeString();
}

void Version::MakeString() {
	if (Special.IsEmpty()) {
		LongString.Format("%d.%d.%d.%d", Major, Minor, Release, Revision);
	} else {
		LongString.Format("%d.%d.%d.%d %s", Major, Minor, Release, Revision, Special.GetCStr());
	}
	BackwardCompatibleString.Format("%d.%02d.%d%s", Major, Minor * 10 + Release, Revision, Special.GetCStr());
	ShortString.Format("%d.%d", Major, Minor);
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
