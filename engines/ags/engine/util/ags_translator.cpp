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

#include "ags/engine/util/ags_translator.h"

#include "common/archive.h"
#include "common/debug.h"
#include "common/formats/json.h"
#include "common/memstream.h"
#include "common/path.h"
#include "common/savefile.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"

namespace AGS3 {

AgsTranslator &AgsTranslator::getInstance() {
	static AgsTranslator t;
	return t;
}

void AgsTranslator::logMissingString(const Common::String &src) {
	// Don't log the same missing string over and over
	if (_missingStrings.contains(src))
		return;
	_missingStrings[src] = true;

	Common::String hexStr;
	for (uint i = 0; i < src.size(); ++i) {
		hexStr += Common::String::format("%02X ", (unsigned char)src[i]);
	}

	debug(0, "AGS Translation Missing: '%s' (Hex: %s)", src.c_str(), hexStr.c_str());
}

const char *AgsTranslator::getTranslation(const char *src) {
	loadFromJsonIfPresent("translation_override.json");

	if (!src || !*src)
		return src;

	if (_entries.empty()) {
		logMissingString(Common::String(src));
		return src;
	}

	const Common::String key = normalizeKey(Common::String(src));

	// If the exact string is already a target translation value, it means
	// AGS is re-translating an already translated string (e.g., in UI loops).
	// We should just return it without parsing it as a missing original string.
	// NOTE: We don't have a reverse lookup right now, but for simplicity we
	// can just check values. Since checking all values every frame is O(N),
	// we will add a reverse cache during JSON load!
	if (_entries.contains(key)) {
		return _entries[key].c_str();
	}

	// If the exact string is already a target translation value, it means
	// AGS is re-translating an already translated string (e.g., in UI loops).
	// We should just return it without parsing it as a missing original string.
	if (_translatedValues.contains(src)) {
		return src;
	}

	logMissingString(Common::String(src));
	return src;
}

bool AgsTranslator::loadFromJsonIfPresent(const Common::String &filename) {
	if (_loaded)
		return !_entries.empty();

	_loaded = true;
	_entries.clear();

	Common::SeekableReadStream *rs = SearchMan.createReadStreamForMember(Common::Path(filename));
	if (!rs) {
		// Optional file.
		return false;
	}

	Common::MemoryWriteStreamDynamic buffer(DisposeAfterUse::YES);
	buffer.writeStream(rs, rs->size());
	Common::JSON::zeroTerminateContents(buffer);
	delete rs;

	Common::JSONValue *json = Common::JSON::parse((const char *)buffer.getData());
	if (!json) {
		warning("AGS Translator: failed to parse %s (invalid JSON)", filename.c_str());
		return false;
	}

	if (!json->isObject()) {
		warning("AGS Translator: failed to parse %s (expected JSON object at root)", filename.c_str());
		delete json;
		return false;
	}

	const Common::JSONObject &obj = json->asObject();
	for (Common::JSONObject::const_iterator it = obj.begin(); it != obj.end(); ++it) {
		if (it->_value->isString()) {
			Common::String key = normalizeKey(it->_key);
			if (!key.empty()) {
				Common::String val = it->_value->asString();
				_entries[key] = val;
				_translatedValues[val] = true;
			}
		}
	}

	delete json;
	return !_entries.empty();
}

bool AgsTranslator::translate(const Common::String &src, Common::String &out) const {
	if (_entries.empty()) {
		return false;
	}

	const Common::String key = normalizeKey(src);
	if (_entries.contains(key)) {
		out = _entries[key];
		return true;
	}

	return false;
}

Common::String AgsTranslator::normalizeKey(const Common::String &s) {
	Common::String r = s;
	r.trim();

	// Collapse whitespace (space/tab) to single spaces.
	Common::String out;
	bool prevSpace = false;

	for (uint i = 0; i < r.size(); i++) {
		const char ch = r[i];
		const bool isSpace = (ch == ' ' || ch == '\t');
		if (isSpace) {
			if (!prevSpace)
				out += ' ';
			prevSpace = true;
		} else {
			out += ch;
			prevSpace = false;
		}
	}

	return out;
}

} // namespace AGS3
