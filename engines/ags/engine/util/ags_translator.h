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

#ifndef AGS_ENGINE_UTIL_AGS_TRANSLATOR_H
#define AGS_ENGINE_UTIL_AGS_TRANSLATOR_H

#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/str.h"

namespace AGS3 {

class AgsTranslator {
public:
	static AgsTranslator &getInstance();

	// Loads translations from a UTF-8 JSON file that contains a flat object: { "src": "dst", ... }
	// This allows users to provide fan-translations or string replacements without modifying game files.
	// The file is optional; if missing, returns false.
	bool loadFromJsonIfPresent(const Common::String &filename);

	// Returns true and sets out if translation exists; otherwise false.
	bool translate(const Common::String &src, Common::String &out) const;

	// Returns the translated c-string, or the original if not found.
	const char *getTranslation(const char *src);

private:
	Common::HashMap<Common::String, Common::String> _entries;
	Common::HashMap<Common::String, bool> _translatedValues;
	Common::HashMap<Common::String, bool> _missingStrings;
	bool _loaded = false;

	static Common::String normalizeKey(const Common::String &s);

	void logMissingString(const Common::String &src);
};

} // namespace AGS3

#endif
