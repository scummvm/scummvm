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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/ini.h"

#include "common/formats/ini-file.h"

namespace Ripper {

namespace {

static Common::String stripIniComment(const Common::String &source) {
	const size_t comment = source.find(';');
	Common::String value = comment == Common::String::npos ?
		source : source.substr(0, comment);
	value.trim();
	return value;
}

static bool parseUnsignedDecimal(const Common::String &text, uint64 maximum,
		uint64 &value) {
	if (text.empty())
		return false;

	uint64 parsed = 0;
	for (uint i = 0; i < text.size(); ++i) {
		if (text[i] < '0' || text[i] > '9')
			return false;
		const uint digit = text[i] - '0';
		if (parsed > (maximum - digit) / 10)
			return false;
		parsed = parsed * 10 + digit;
	}
	value = parsed;
	return true;
}

} // End of anonymous namespace

Common::String getIniString(const Common::INIFile &ini, const char *section,
		const char *key, const Common::String &fallback) {
	Common::String value;
	if (!ini.getKey(key, section, value))
		return fallback;
	value = stripIniComment(value);
	return value.empty() ? fallback : value;
}

int getIniInt(const Common::INIFile &ini, const char *section,
		const char *key, int fallback) {
	const Common::String text = getIniString(ini, section, key);
	uint64 value = 0;
	return parseUnsignedDecimal(text, 0x7fffffffU, value) ? (int)value : fallback;
}

bool readIniUint(const Common::INIFile &ini, const Common::String &section,
		const Common::String &key, uint &value) {
	const Common::String text = getIniString(ini, section.c_str(), key.c_str());
	uint64 parsed = 0;
	if (!parseUnsignedDecimal(text, 0xffffffffULL, parsed))
		return false;
	value = (uint)parsed;
	return true;
}

} // End of namespace Ripper
