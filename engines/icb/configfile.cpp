/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/configfile.h"

#include "common/textconsole.h"
#include "common/ini-file.h"

namespace ICB {

ConfigFile::ConfigFile() {}

void ConfigFile::readFile(const Common::String &filename) {
	Common::INIFile file;
	if (!file.loadFromFile(filename)) {
		error("Opening file '%s' failed'", filename.c_str());
		return;
	}

	Common::INIFile::SectionList sections = file.getSections();
	for (Common::INIFile::SectionList::const_iterator i = sections.begin(); i != sections.end(); i++) {
		Common::INIFile::SectionKeyList kList = i->getKeys();
		for (Common::INIFile::SectionKeyList::const_iterator j = kList.begin(); j != kList.end(); j++) {
			_dataSet[i->name][j->key] = j->value;
		}
	}
}

Common::String ConfigFile::readSetting(const Common::String &section, const Common::String &key, const Common::String &defaultValue) const {
	Common::HashMap<Common::String, Common::HashMap<Common::String, Common::String> >::const_iterator sectionIt;
	sectionIt = _dataSet.find(section);
	if (sectionIt != _dataSet.end()) {
		Common::HashMap<Common::String, Common::String>::const_iterator keyIt = sectionIt->_value.find(key);
		if (keyIt != sectionIt->_value.end()) {
			return keyIt->_value;
		}
	}
	return defaultValue;
}

int32 ConfigFile::readIntSetting(const Common::String &section, const Common::String &key, int32 defaultValue) const {
	return atoi(readSetting(section, key, Common::String().format("%d", defaultValue)).c_str());
}

} // End of namespace ICB
