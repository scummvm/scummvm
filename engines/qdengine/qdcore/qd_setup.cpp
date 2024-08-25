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

bool enumerateIniSections(const char *fname, Common::INIFile::SectionList &sectionList) {

	Common::INIFile ini;
	Common::Path iniFilePath(fname);
	ini.loadFromFile(iniFilePath);
	sectionList = ini.getSections();
	int size = sectionList.size();

	if (!size) {
		return false;
	}

	return true;
}

const char *getIniKey(const char *fname, const char *section, const char *key) {
	Common::INIFile ini;
	Common::String buf;

	ini.loadFromFile(fname);
	bool hasValue = ini.getKey(key, section, buf);

	if (!hasValue) {
		return "";
	}

	return buf.c_str();
}

} // namespace QDEngine
