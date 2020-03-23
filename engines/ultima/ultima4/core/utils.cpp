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

/**
 * Seed the random number generator.
 */
void xu4_srandom() {
//    srand((unsigned int)time(NULL));
}

/**
 * Generate a random number between 0 and (upperRange - 1).  This
 * routine uses the upper bits of the random number provided by rand()
 * to compensate for older generators that have low entropy in the
 * lower bits (e.g. MacOS X).
 */
int xu4_random(int upperRange) {
	return g_ultima->getRandomNumber(0x7fffffff);
}

/**
 * Trims whitespace from a Common::String
 * @param val The Common::String you are trimming
 * @param chars_to_trim A list of characters that will be trimmed
 */
Common::String &trim(Common::String &val, const Common::String &chars_to_trim) {
	using namespace std;
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

/**
 * Converts the Common::String to lowercase
 */
Common::String &lowercase(Common::String &val) {
	using namespace std;
	Common::String::iterator i;
	for (i = val.begin(); i != val.end(); i++)
		*i = tolower(*i);
	return val;
}

/**
 * Converts the Common::String to uppercase
 */
Common::String &uppercase(Common::String &val) {
	using namespace std;
	Common::String::iterator i;
	for (i = val.begin(); i != val.end(); i++)
		*i = toupper(*i);
	return val;
}

/**
 * Converts an integer value to a Common::String
 */
Common::String xu4_to_string(int val) {
	char buffer[16];
	sprintf(buffer, "%d", val);
	return buffer;
}

/**
 * Splits a Common::String into substrings, divided by the charactars in
 * separators.  Multiple adjacent seperators are treated as one.
 */
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
