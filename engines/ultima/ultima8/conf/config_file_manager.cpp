/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/conf/config_file_manager.h"
#include "ultima/ultima8/conf/ini_file.h"
#include "common/config-manager.h"

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

	ConfMan.flushToDisk();
	clear();
	_configFileManager = nullptr;
}

bool ConfigFileManager::readConfigFile(string fname, istring root,
                                       bool readonly) {
	INIFile *inifile = new INIFile();
	inifile->clear(root);
	if (!inifile->readConfigFile(fname)) {
		delete inifile;
		return false;
	}
	if (readonly)
		inifile->setReadonly();

	_iniFiles.push_back(inifile);
	return true;
}

bool ConfigFileManager::readConfigString(string config, istring root,
        bool readonly) {
	INIFile *inifile = new INIFile();
	inifile->clear(root);
	if (!inifile->readConfigString(config)) {
		delete inifile;
		return false;
	}
	if (readonly)
		inifile->setReadonly();

	_iniFiles.push_back(inifile);
	return true;
}

void ConfigFileManager::write(istring root) {
	for (Std::vector<INIFile *>::iterator i = _iniFiles.begin();
	        i != _iniFiles.end(); ++i) {
		if (!(*i)->isReadonly() && (root == "" || (*i)->checkRoot(root)))
			(*i)->write();
	}
}

void ConfigFileManager::clear() {
	for (Std::vector<INIFile *>::iterator i = _iniFiles.begin();
	        i != _iniFiles.end(); ++i) {
		delete(*i);
	}
	_iniFiles.clear();
}

void ConfigFileManager::clearRoot(istring root) {
	Std::vector<INIFile *>::iterator i = _iniFiles.begin();

	while (i != _iniFiles.end()) {
		if ((*i)->checkRoot(root)) {
			delete(*i);
			i = _iniFiles.erase(i);
		} else {
			++i;
		}
	}
}

bool ConfigFileManager::exists(istring key) {
	return ConfMan.hasKey(key) || (findKeyINI(key) != nullptr);
}

bool ConfigFileManager::get(istring key, string &ret) {
	if (ConfMan.hasKey(key)) {
		ret = ConfMan.get(key);
		return true;
	}

	INIFile *ini = findKeyINI(key);
	if (!ini) return false;

	ini->value(key, ret);
	return true;
}


bool ConfigFileManager::get(istring key, int &ret) {
	if (ConfMan.hasKey(key)) {
		ret = ConfMan.getInt(key);
		return true;
	}

	INIFile *ini = findKeyINI(key);
	if (!ini) return false;

	ini->value(key, ret);
	return true;
}

bool ConfigFileManager::get(istring key, bool &ret) {
	if (ConfMan.hasKey(key)) {
		ret = ConfMan.getBool(key);
		return true;
	}

	INIFile *ini = findKeyINI(key);
	if (!ini) return false;

	ini->value(key, ret);
	return true;
}

void ConfigFileManager::set(istring key, string val) {
	if (key.hasPrefix("settings/")) {
		Common::String subKey(key.c_str() + key.findLastOf('/') + 1);
		ConfMan.set(subKey, val);
	} else {
		INIFile *ini = findWriteINI(key);
		if (!ini) return;

		ini->set(key, val);
	}
}

void ConfigFileManager::set(istring key, const char *val) {
	if (key.hasPrefix("settings/")) {
		Common::String subKey(key.c_str() + key.findLastOf('/') + 1);
		ConfMan.set(subKey, val);
	} else {
		INIFile *ini = findWriteINI(key);
		if (!ini) return;

		ini->set(key, val);
	}
}

void ConfigFileManager::set(istring key, int val) {
	if (key.hasPrefix("settings/")) {
		Common::String subKey(key.c_str() + key.findLastOf('/') + 1);
		ConfMan.setInt(subKey, val);
	} else {
		INIFile *ini = findWriteINI(key);
		if (!ini) return;

		ini->set(key, val);
	}
}

void ConfigFileManager::set(istring key, bool val) {
	if (key.hasPrefix("settings/")) {
		Common::String subKey(key.c_str() + key.findLastOf('/') + 1);
		ConfMan.setBool(subKey, val);
	} else {
		INIFile *ini = findWriteINI(key);
		if (!ini) return;

		ini->set(key, val);
	}
}

void ConfigFileManager::unset(istring key) {
	if (key.hasPrefix("settings/")) {
		Common::String subKey(key.c_str() + key.findLastOf('/') + 1);
		ConfMan.set(subKey, "");
	} else {
		INIFile *ini = findWriteINI(key);
		if (!ini) return;

		ini->unset(key);
	}
}



Std::vector<istring> ConfigFileManager::listKeys(istring section,
        bool longformat) {
	Std::vector<istring> keys;

	Std::set<istring> keyset;
	Std::set<istring>::iterator iter;

	for (Std::vector<INIFile *>::iterator i = _iniFiles.begin();
	        i != _iniFiles.end(); ++i) {
		if ((*i)->checkRoot(section)) {
			(*i)->listKeys(keyset, section, longformat);
		}
	}

	for (iter = keyset.begin(); iter != keyset.end(); ++iter) {
		keys.push_back(*iter);
	}

	return keys;
}

Std::vector<istring> ConfigFileManager::listSections(istring root,
        bool longformat) {
	Std::vector<istring> sections;

	Std::set<istring> sectionset;
	Std::set<istring>::iterator iter;

	for (Std::vector<INIFile *>::iterator i = _iniFiles.begin();
	        i != _iniFiles.end(); ++i) {
		if ((*i)->checkRoot(root)) {
			(*i)->listSections(sectionset, longformat);
		}
	}

	for (iter = sectionset.begin(); iter != sectionset.end(); ++iter) {
		sections.push_back(*iter);
	}

	return sections;
}

KeyMap ConfigFileManager::listKeyValues(istring section,
        bool longformat) {
	KeyMap values;

	for (Std::vector<INIFile *>::iterator i = _iniFiles.begin();
	        i != _iniFiles.end(); ++i) {
		if ((*i)->checkRoot(section)) {
			(*i)->listKeyValues(values, section, longformat);
		}
	}

	return values;
}


INIFile *ConfigFileManager::findKeyINI(istring key) {
	for (Std::vector<INIFile *>::reverse_iterator i = _iniFiles.rbegin();
	        i != _iniFiles.rend(); ++i) {
		if ((*i)->hasKey(key))
			return (*i);
	}

	return nullptr;
}

INIFile *ConfigFileManager::findWriteINI(istring key) {
	for (Std::vector<INIFile *>::reverse_iterator i = _iniFiles.rbegin();
	        i != _iniFiles.rend(); ++i) {
		if (!(*i)->isReadonly() && (*i)->checkRoot(key))
			return (*i);
	}

	return nullptr;
}

} // End of namespace Ultima8
} // End of namespace Ultima
