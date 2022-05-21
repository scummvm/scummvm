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

#include "common/array.h"
#include "common/file.h"
#include "mm/utils/strings_data.h"

namespace MM {

bool StringsData::load(const Common::String &filename) {
	Common::File f;
	Common::Array<Common::String> prefixKeys;
	Common::String key, value, fullKey;
	size_t p;

	if (!f.open(filename))
		return false;

	while (!f.eos()) {
		// Get the next line
		Common::String line = f.readLine();

		// Check for blank or commen tlines
		Common::String lineTrimmed = line;
		lineTrimmed.trim();
		if (lineTrimmed.empty() || lineTrimmed.hasPrefix("#"))
			continue;

		// Count number of tabs for identation amount
		size_t numTabs = 0;
		while (line.hasPrefix("\t")) {
			line.deleteChar(0);
			++numTabs;
		}

		// Split key and value if present
		p = line.findFirstOf(":");
		if (p == Common::String::npos)
			error("Line encountered without colon");

		key = Common::String(line.c_str(), line.c_str() + p);
		value = Common::String(line.c_str() + p + 1);
		key.trim();
		value.trim();

		// Handle quoted string values
		if (value.hasPrefix("\"") && value.hasSuffix("\"")) {
			value.deleteChar(0);
			value.deleteLastChar();
		}

		// Handle the entries
		if (numTabs == prefixKeys.size()) {
			// Do nothing
		} else if (numTabs < prefixKeys.size()) {
			// Drop off prefixes to the desired indentation
			prefixKeys.resize(numTabs);	
		} else {
			error("Incorrect indentation");
		}

		if (value.empty()) {
			prefixKeys.push_back(key);
		} else {
			// Form a key from the prefix and current key
			fullKey = "";
			for (size_t i = 0; i < prefixKeys.size(); ++i) {
				fullKey += prefixKeys[i];
				fullKey += ".";
			}

			fullKey = fullKey + key;
			(*this)[fullKey] = value;
		}
	}

	return true;
}

} // namespace MM
