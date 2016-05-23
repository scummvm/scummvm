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

uint32 find(const Common::String &haystack, const Common::String &needle, uint32 pos = 0) {
	if (pos >= haystack.size()) {
		return Common::String::npos;
	}

	//TODO: write something smarter
	uint32 lastIndex = haystack.size() - needle.size();
	for (uint32 i = pos; i < lastIndex; ++i) {
		bool found = true;
		for (uint32 j = 0; j < needle.size(); ++j)
			if (haystack[i + j] != needle[j]) {
				found = false;
				break;
			}

		if (found) return i;
	}

	return Common::String::npos;
}

Common::String getSubstring(const Common::String &s, uint32 beginning, uint32 ending) {
	//beginning inclusive, ending exclusive
	if (beginning == -1 || ending == -1) return ""; //bad
	Common::String result = s;
	result.erase(ending);
	result.erase(0, beginning);
	return result;
}

int parseInt(Common::String s) {
	//TODO: not sure this is not forbidden at all
	return atoi(s.c_str());
}

}

namespace Cloud {
namespace ISO8601 {

uint32 convertToTimestamp(const Common::String &iso8601Date) {		
	//2015-05-12T15:50:38Z
	uint32 firstHyphen = find(iso8601Date, "-");
	uint32 secondHyphen = find(iso8601Date, "-", firstHyphen + 1);
	uint32 tSeparator = find(iso8601Date, "T", secondHyphen + 1);
	uint32 firstColon = find(iso8601Date, ":", tSeparator + 1);
	uint32 secondColon = find(iso8601Date, ":", firstColon + 1);
	uint32 zSeparator = find(iso8601Date, "Z", secondColon + 1);
	//now note '+1' which means if there ever was '-1' result of find(), we still did a valid find() from 0th char

	Common::String year = getSubstring(iso8601Date, 0, firstHyphen);
	Common::String month = getSubstring(iso8601Date, firstHyphen + 1, secondHyphen);
	Common::String day = getSubstring(iso8601Date, secondHyphen + 1, tSeparator);
	Common::String hour = getSubstring(iso8601Date, tSeparator + 1, firstColon);
	Common::String minute = getSubstring(iso8601Date, firstColon + 1, secondColon);
	Common::String second = getSubstring(iso8601Date, secondColon + 1, zSeparator);
	//now note only 'ending' argument was not '+1' (which means I could've make that function such that -1 means 'until the end')

	int Y = parseInt(year);
	int M = parseInt(month);
	int D = parseInt(day);
	int h = parseInt(hour);
	int m = parseInt(minute);
	int s = parseInt(second);

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

} //end of namespace ISO8601
} //end of namespace Cloud
