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

#include "common/textconsole.h"
#include "mm/shared/utils/strings.h"

namespace MM {

Common::String capitalize(const Common::String &str) {
	Common::String result = str;
	bool capitalize = true;

	for (uint i = 0; i < str.size(); ++i) {
		if (!scumm_strnicmp(result.c_str() - 1, " i ", 3))
			// the word 'I' is always capitalized
			capitalize = true;

		if (capitalize) {
			result.setChar(toupper(result[i]), i);
			capitalize = false;
		} else {
			result.setChar(tolower(result[i]), i);
		}

		if (result[i] == '.')
			capitalize = true;
	}

	return result;
}

Common::String camelCase(const Common::String &str) {
	Common::String result = str;
	bool capitalize = true;

	for (uint i = 0; i < str.size(); ++i) {
		if (capitalize) {
			result.setChar(toupper(result[i]), i);
			capitalize = false;
		} else {
			result.setChar(tolower(result[i]), i);
		}

		if (result[i] == ' ')
			capitalize = true;
	}

	return result;
}

Common::String uppercase(const Common::String &str) {
	Common::String result;

	for (uint i = 0; i < str.size(); ++i)
		result += toupper(str[i]);

	return result;
}

Common::String searchAndReplace(const Common::String &str,
		const Common::String &find, const Common::String &replace) {
	Common::String result = str;
	const char *p = result.c_str();

	while ((p = strstr(p, find.c_str())) != nullptr) {
		uint idx = p - result.c_str();
		result = Common::String::format("%s%s%s",
			Common::String(result.c_str(), p).c_str(),
			replace.c_str(),
			Common::String(p + find.size()).c_str()
		);

		p = result.c_str() + idx + replace.size();
	}

	return result;
}

int strToInt(const Common::String &str) {
	const char *s = str.c_str();
	size_t size = strlen(s);

	if (size == 0)
		// No string at all
		return 0;

	if (toupper(s[size - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp = 0;
	int read = sscanf(s, "%xh", &tmp);
	if (read < 1)
		error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

int hexToInt(const Common::String &str) {
	uint tmp = 0;
	int read = sscanf(str.c_str(), "%xh", &tmp);
	if (read < 1)
		error("hexToInt failed on string \"%s\"", str.c_str());
	return (int)tmp;
}

Common::StringArray splitLines(const Common::String &str) {
	Common::StringArray results;
	size_t start = 0, end;

	while ((end = str.findFirstOf('\n', start)) != Common::String::npos) {
		results.push_back(Common::String(
			str.c_str() + start, str.c_str() + end));
		start = end + 1;
	}

	results.push_back(str.c_str() + start);


	return results;
}

} // namespace MM
