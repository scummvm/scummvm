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

#include "ultima/ultima4/core/utils.h"

namespace Ultima {
namespace Ultima4 {

void assertMsg(bool exp, const char *desc, ...) {
	if (!exp) {
		va_list args;
		va_start(args, desc);
		Common::String msg = Common::String::vformat(desc, args);
		va_end(args);

		error("Assertion failed: %s", msg.c_str());
	}
}

void xu4_srandom() {
//    srand((uint)time(nullptr));
}

int xu4_random(int upperRange) {
	if (upperRange == 0) {
		warning("No upper range specified");
		return 0;
	}

	return g_ultima->getRandomNumber(upperRange - 1);
}

Common::String &trim(Common::String &val, const Common::String &chars_to_trim) {
	Common::String::iterator i;
	if (val.size()) {
		size_t pos;
		for (i = val.begin(); (i != val.end()) && (pos = chars_to_trim.find(*i)) != Common::String::npos;)
			i = val.erase(i);
		for (i = val.end() - 1; (i != val.begin()) && (pos = chars_to_trim.find(*i)) != Common::String::npos;)
			i = val.erase(i) - 1;
	}
	return val;
}

Common::String &lowercase(Common::String &val) {
	Common::String::iterator i;
	for (i = val.begin(); i != val.end(); i++)
		*i = tolower(*i);
	return val;
}

Common::String &uppercase(Common::String &val) {
	Common::String::iterator i;
	for (i = val.begin(); i != val.end(); i++)
		*i = toupper(*i);
	return val;
}

Common::String xu4_to_string(int val) {
	char buffer[16];
	sprintf(buffer, "%d", val);
	return buffer;
}

Std::vector<Common::String> split(const Common::String &s, const Common::String &separators) {
	Std::vector<Common::String> result;
	Common::String current;

	for (unsigned i = 0; i < s.size(); i++) {
		if (separators.find(s[i]) != Common::String::npos) {
			if (current.size() > 0)
				result.push_back(current);
			current.clear();
		} else
			current += s[i];
	}

	if (current.size() > 0)
		result.push_back(current);

	return result;
}

} // End of namespace Ultima4
} // End of namespace Ultima
