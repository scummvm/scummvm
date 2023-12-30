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

#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/conf/config_file_manager.h"
#include "ultima/ultima8/filesys/file_system.h"

namespace Ultima {
namespace Ultima8 {

using Std::string;

ConfigFileManager *ConfigFileManager::_configFileManager = nullptr;

ConfigFileManager::ConfigFileManager() {
	debugN(MM_INFO, "Creating ConfigFileManager...\n");

	_configFileManager = this;
}

ConfigFileManager::~ConfigFileManager() {
	debugN(MM_INFO, "Destroying ConfigFileManager...\n");

	clear();
	_configFileManager = nullptr;
}

bool ConfigFileManager::readConfigFile(string fname, const Std::string &category) {
	Common::SeekableReadStream *f = FileSystem::get_instance()->ReadFile(fname);
	if (!f) return false;

	ConfigFile *configFile = new ConfigFile();
	configFile->_category = category;

	// We need various characters as the inis are used for translations.
	configFile->_iniFile.allowNonEnglishCharacters();

	if (!configFile->_iniFile.loadFromStream(*f)) {
		delete configFile;
		return false;
	}

	_configFiles.push_back(configFile);
	return true;
}

void ConfigFileManager::clear() {
	Std::vector<ConfigFile*>::iterator i;
	for (i = _configFiles.begin(); i != _configFiles.end(); ++i) {
		delete(*i);
	}
	_configFiles.clear();
}

void ConfigFileManager::clearRoot(const Std::string &category) {
	Std::vector<ConfigFile *>::iterator i = _configFiles.begin();

	while (i != _configFiles.end()) {
		if (category.equalsIgnoreCase((*i)->_category)) {
			delete(*i);
			i = _configFiles.erase(i);
		} else {
			++i;
		}
	}
}

bool ConfigFileManager::get(const Std::string &category, const Std::string &section, const Std::string &key, string &ret) const {
	Std::vector<ConfigFile*>::const_reverse_iterator i;
	for (i = _configFiles.rbegin(); i != _configFiles.rend(); ++i) {
		if (category.equalsIgnoreCase((*i)->_category)) {
			if ((*i)->_iniFile.getKey(key, section, ret)) {
				return true;
			}
		}
	}

	return false;
}


bool ConfigFileManager::get(const Std::string &category, const Std::string &section, const Std::string &key, int &ret) const {
	string stringval;
	if (!get(category, section, key, stringval))
		return false;

	ret = strtol(stringval.c_str(), 0, 0);
	return true;
}

bool ConfigFileManager::get(const Std::string &category, const Std::string &section, const Std::string &key, bool &ret) const {
	string stringval;
	if (!get(category, section, key, stringval))
		return false;

	ret = (stringval == "yes" || stringval == "true");
	return true;
}

Std::vector<Std::string> ConfigFileManager::listSections(const Std::string &category) const {
	Std::vector<Std::string> sections;
	Std::vector<ConfigFile*>::const_iterator i;

	for ( i = _configFiles.begin(); i != _configFiles.end(); ++i) {
		if (category.equalsIgnoreCase((*i)->_category)) {
			Common::INIFile::SectionList sectionList = (*i)->_iniFile.getSections();
			Common::INIFile::SectionList::const_iterator j;
			for (j = sectionList.begin(); j != sectionList.end(); ++j) {
				sections.push_back(j->name);
			}
		}
	}

	return sections;
}

KeyMap ConfigFileManager::listKeyValues(const Std::string &category, const Std::string &section) const {
	KeyMap values;
	Std::vector<ConfigFile*>::const_iterator i;

	for (i = _configFiles.begin(); i != _configFiles.end(); ++i) {
		const ConfigFile *c = *i;
		if (category.equalsIgnoreCase((*i)->_category) && c->_iniFile.hasSection(section)) {
			Common::INIFile::SectionKeyList keys = c->_iniFile.getKeys(section);
			Common::INIFile::SectionKeyList::const_iterator j;
			for (j = keys.begin(); j != keys.end(); ++j) {
				values[j->key] = j->value;
			}
		}
	}

	return values;
}

} // End of namespace Ultima8
} // End of namespace Ultima
