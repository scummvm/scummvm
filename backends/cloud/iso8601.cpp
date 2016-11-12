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

#include "backends/cloud/iso8601.h"
#include "common/str.h"

namespace {

Common::String getSubstring(const Common::String &s, uint32 beginning, uint32 ending) {
	//beginning inclusive, ending exclusive
	Common::String result = s;
	result.erase(ending);
	result.erase(0, beginning);
	return result;
}

int find(const char *cstr, uint32 startPosition, char needle) {
	const char *res = strchr(cstr + startPosition, needle);
	if (res == nullptr)
		return -1;
	return res - cstr;
}

}

namespace Cloud {
namespace ISO8601 {

uint32 convertToTimestamp(const Common::String &iso8601Date) {
	//2015-05-12T15:50:38Z
	const char *cstr = iso8601Date.c_str();
	int firstHyphen = find(cstr, 0, '-');
	int secondHyphen = find(cstr, firstHyphen + 1, '-');
	int tSeparator = find(cstr, secondHyphen + 1, 'T');
	int firstColon = find(cstr, tSeparator + 1, ':');
	int secondColon = find(cstr, firstColon + 1, ':');
	int zSeparator = find(cstr, secondColon + 1, 'Z');
	if (zSeparator == -1)
		zSeparator = find(cstr, secondColon + 1, '-'); // Box's RFC 3339
	//now note '+1' which means if there ever was '-1' result of find(), we still did a valid find() from 0th char

	Common::String year = getSubstring(iso8601Date, 0, firstHyphen);
	Common::String month = getSubstring(iso8601Date, firstHyphen + 1, secondHyphen);
	Common::String day = getSubstring(iso8601Date, secondHyphen + 1, tSeparator);
	Common::String hour = getSubstring(iso8601Date, tSeparator + 1, firstColon);
	Common::String minute = getSubstring(iso8601Date, firstColon + 1, secondColon);
	Common::String second = getSubstring(iso8601Date, secondColon + 1, zSeparator);
	//now note only 'ending' argument was not '+1' (which means I could've make that function such that -1 means 'until the end')

	int Y = atoi(year.c_str());
	int M = atoi(month.c_str());
	int D = atoi(day.c_str());
	int h = atoi(hour.c_str());
	int m = atoi(minute.c_str());
	int s = atoi(second.c_str());

	//ok, now I compose a timestamp based on my basic perception of time/date
	//yeah, I know about leap years and leap seconds and all, but still we don't care there

	uint32 days = D - 1;
	for (int i = 1970; i < Y; ++i)
		if ((i % 4 == 0 && i % 100 != 0) || (i % 400 == 0))
			days += 366;
		else
			days += 365;

	int mdays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	for (int i = 1; i < M; ++i) {
		days += mdays[i - 1];
		if (i == 2)
			if ((Y % 4 == 0 && Y % 100 != 0) || (Y % 400 == 0))
				days += 1;
	}

	uint32 hours = days * 24 + h;
	uint32 minutes = hours * 60 + m;
	return minutes * 60 + s;
}

} // End of namespace ISO8601
} // End of namespace Cloud
