/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include <fstream>
#include <iostream>
#include <string>

#include "engines/icb/configfile.h"

// TODO: Complete refactor

#include "common/textconsole.h"
#include "common/config-manager.h"

namespace ICB {

ConfigFile::ConfigFile() {}

void ConfigFile::writeSetting(const Common::String &section, const Common::String &key, const Common::String &value) { _dataSet[section][key] = value; }

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

void ConfigFile::readFile(const Common::String &filename) {
	Common::String path = ConfMan.get("path") + "/" + filename;
	std::string currentSection = "";
	std::ifstream file(path.c_str());
	if (!file.is_open() || file.fail()) {
		assert(0);
	}

	std::string line;
	while (getline(file, line)) {
		size_t start = line.find_first_of("[");
		size_t end = line.find_first_of("]");
		// Section start
		if (start == 0 && start != std::string::npos && end != std::string::npos) {
			currentSection = line.substr(1, end - 1);
			std::cout << "Section: " << currentSection << std::endl;
			continue;
		}
		size_t split = line.find_first_of("=");
		if (split == std::string::npos) {
			continue;
		} else {
			std::string key = line.substr(0, split);
			std::string value = line.substr(split + 1);
			std::cout << "Key: " << key << " Value: " << value << std::endl;
			writeSetting(currentSection.c_str(), key.c_str(), value.c_str());
		}
	}
	file.close();
}

int ConfigFile::readIntSetting(const Common::String &section, const Common::String &key, int defaultValue) const {
	char buff[32];
	sprintf(buff, "%d", defaultValue);
	return atoi(readSetting(section, key, buff).c_str());
}

void ConfigFile::writeFile(const Common::String &filename) const {
	warning("TODO: Fix ConfMan-writing");
	return;
	std::ofstream file(filename.c_str());
	if (!file.is_open() || file.fail()) {
		assert(0);
	}

	Common::HashMap<Common::String, Common::HashMap<Common::String, Common::String> >::const_iterator sectionIt;
	for (sectionIt = _dataSet.begin(); sectionIt != _dataSet.end(); ++sectionIt) {
		file << "[" << sectionIt->_key.c_str() << "]\n";
		Common::HashMap<Common::String, Common::String>::const_iterator keyIt;
		for (keyIt = sectionIt->_value.begin(); keyIt != sectionIt->_value.end(); ++keyIt) {
			file << keyIt->_key.c_str() << "=" << keyIt->_value.c_str() << "\n";
		}
	}
	file.close();
}

} // End of namespace ICB
