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

	debug(0, "AGS Translation Missing: '%s'", src.c_str());
}

const char *AgsTranslator::getTranslation(const char *src) {
	loadFromPoIfPresent("translation_override.po");

	if (!src || !*src)
		return src;

	if (_entries.empty()) {
		logMissingString(Common::String(src));
		return src;
	}

	const Common::String key = normalizeKey(Common::String(src));

	// Check if we have a translation for the normalized original string.
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

void AgsTranslator::storePoEntry(const Common::String &msgId, const Common::String &msgStr) {
	if (!msgId.empty() && !msgStr.empty()) {
		Common::String key = normalizeKey(msgId);
		if (!key.empty()) {
			_entries[key] = msgStr;
			_translatedValues[msgStr] = true;
		}
	}
}

Common::String AgsTranslator::unescapePoString(const Common::String &s) {
	Common::String result;
	int start = 0, end = s.size();
	if (s.size() >= 2 && s.firstChar() == '"' && s.lastChar() == '"') {
		start = 1;
		end = s.size() - 1;
	}

	for (int i = start; i < end; ++i) {
		if (s[i] == '\\' && i + 1 < end) {
			char nextChar = s[i + 1];
			if (nextChar == 'n')
				result += '\n';
			else if (nextChar == 'r')
				result += '\r';
			else if (nextChar == 't')
				result += '\t';
			else if (nextChar == '\\')
				result += '\\';
			else if (nextChar == '"')
				result += '"';
			else {
				result += '\\';
				result += nextChar;
			}
			++i; // skip escaped char
		} else {
			result += s[i];
		}
	}
	return result;
}

bool AgsTranslator::loadFromPoIfPresent(const Common::String &filename) {
	if (_loaded)
		return !_entries.empty();

	_loaded = true;
	_entries.clear();

	Common::SeekableReadStream *rs = SearchMan.createReadStreamForMember(Common::Path(filename));
	if (!rs) {
		// Optional file.
		return false;
	}

	Common::String currentMsgId, currentMsgStr;
	bool inMsgId = false;
	bool inMsgStr = false;

	while (!rs->eos() && !rs->err()) {
		Common::String line = rs->readLine();
		line.trim();

		if (line.empty() || line.hasPrefix("#"))
			continue;

		if (line.hasPrefix("msgid ")) {
			// Save previous entry if any
			if (!currentMsgId.empty() && !currentMsgStr.empty()) {
				storePoEntry(currentMsgId, currentMsgStr);
			}
			inMsgId = true;
			inMsgStr = false;
			currentMsgId = unescapePoString(line.c_str() + 6);
			currentMsgStr.clear();
		} else if (line.hasPrefix("msgstr ")) {
			inMsgId = false;
			inMsgStr = true;
			currentMsgStr = unescapePoString(line.c_str() + 7);
		} else if (line.hasPrefix("\"")) {
			// Multiline string
			Common::String appendStr = unescapePoString(line);
			if (inMsgId) {
				currentMsgId += appendStr;
			} else if (inMsgStr) {
				currentMsgStr += appendStr;
			}
		}
	}

	// Save the last entry
	if (!currentMsgId.empty() && !currentMsgStr.empty()) {
		storePoEntry(currentMsgId, currentMsgStr);
	}

	delete rs;
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
