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

#include "common/formats/ini-file.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_setup.h"

namespace QDEngine {

bool enumerateIniSections(Common::INIFile& ini, const Common::Path &fname, Common::INIFile::SectionList &sectionList) {

	Common::Path iniFilePath(fname);
	ini.allowNonEnglishCharacters();

	if (!ini.loadFromFile(iniFilePath)) {
		warning("Failed to load INI file: %s", iniFilePath.toString().c_str());
		return false;
	}

	sectionList = ini.getSections();
	int size = sectionList.size();

	if (!size) {
		return false;
	}

	return true;
}

const Common::String getIniKey(Common::INIFile& ini, const Common::Path &fname, const char *section, const char *key) {
	Common::String buf;

	bool hasValue = ini.getKey(key, section, buf);

	if (!hasValue) {
		return "";
	}

	return buf;
}

} // namespace QDEngine
