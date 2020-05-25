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
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/conf/config_file_manager.h"
#include "common/config-manager.h"

namespace Ultima {
namespace Ultima8 {

SettingManager *SettingManager::_settingManager = nullptr;

SettingManager::SettingManager() {
	debugN(MM_INFO, "Creating SettingManager...\n");

	_settingManager = this;

	_domains.resize(DOM_GAME + 1);

	_confFileMan = ConfigFileManager::get_instance();

	_confFileMan->readConfigString("", "defaultsettings", false);
}

SettingManager::~SettingManager() {
	debugN(MM_INFO, "Destroying SettingManager...\n");

	_settingManager = nullptr;
}

bool SettingManager::readConfigFile(Std::string fname, bool readonly) {
	return _confFileMan->readConfigFile(fname, "settings", readonly);
}

void SettingManager::write() {
	_confFileMan->write("settings");
}

bool SettingManager::exists(istring key, Domain dom) {
	Domain temp;

	return findKeyDomain(key, dom, temp);
}

bool SettingManager::get(istring key, Std::string &ret, Domain dom) {
	Domain keydom;
	bool found = findKeyDomain(key, dom, keydom);
	if (!found)
		return false;

	_confFileMan->get(getConfigKey(key, keydom), ret);

	return true;
}

bool SettingManager::get(istring key, int &ret, Domain dom) {
	Domain keydom;
	bool found = findKeyDomain(key, dom, keydom);
	if (!found)
		return false;

	_confFileMan->get(getConfigKey(key, keydom), ret);

	return true;
}

bool SettingManager::get(istring key, bool &ret, Domain dom) {
	Domain keydom;
	bool found = findKeyDomain(key, dom, keydom);
	if (!found)
		return false;

	_confFileMan->get(getConfigKey(key, keydom), ret);

	return true;
}


void SettingManager::set(istring key, Std::string value, Domain dom) {
	_confFileMan->set(getConfigKey(key, dom), value);

	callCallbacks(key);
}

void SettingManager::set(istring key, const char *value, Domain dom) {
	_confFileMan->set(getConfigKey(key, dom), value);

	callCallbacks(key);
}

void SettingManager::set(istring key, int value, Domain dom) {
	_confFileMan->set(getConfigKey(key, dom), value);

	callCallbacks(key);
}

void SettingManager::set(istring key, bool value, Domain dom) {
	_confFileMan->set(getConfigKey(key, dom), value);

	callCallbacks(key);
}

void SettingManager::unset(istring key, Domain dom) {
	_confFileMan->unset(getConfigKey(key, dom));

	callCallbacks(key);
}




void SettingManager::setDefault(istring key, Std::string value) {
	set(key, value, DOM_DEFAULTS);
}

void SettingManager::setDefault(istring key, const char *value) {
	set(key, value, DOM_DEFAULTS);
}

void SettingManager::setDefault(istring key, int value) {
	set(key, value, DOM_DEFAULTS);
}

void SettingManager::setDefault(istring key, bool value) {
	set(key, value, DOM_DEFAULTS);
}



void SettingManager::setCurrentDomain(Domain dom) {
	_currentDomain = dom;
}

void SettingManager::setDomainName(Domain dom, istring section) {
	unsigned int d = static_cast<unsigned int>(dom);

	if (_domains.size() <= d)
		_domains.resize(d + 1);
	_domains[d] = section;
}

void SettingManager::registerCallback(istring key, ConfigCallback callback) {
	_callbacks[key].push_back(callback);
}

void SettingManager::unregisterCallback(istring key, ConfigCallback callback) {
	Callbacks::iterator i = _callbacks.find(key);
	if (i == _callbacks.end())
		return;

	Std::vector<ConfigCallback> &cb = (*i)._value;
	Std::vector<ConfigCallback>::iterator iter;
	for (iter = cb.begin(); iter != cb.end(); ++iter) {
		if (*iter == callback) {
			cb.erase(iter);
			return;
		}
	}
}

Std::vector<istring> SettingManager::listGames() {
	Std::vector<istring> games;
	games.push_back("pentagram");
	games.push_back("ultima8");
	games.push_back("remorse");
	games.push_back("regret");

	return games;
}

Std::vector<istring> SettingManager::listDataKeys(istring section) {
	istring csection = "settings/" + _domains[DOM_GAME] + ":" + section;

	return _confFileMan->listKeys(csection, false);
}

KeyMap SettingManager::listDataValues(istring section) {
	istring csection = "settings/" + _domains[DOM_GAME] + ":" + section;

	return _confFileMan->listKeyValues(csection, false);
}

bool SettingManager::findKeyDomain(istring key, Domain dom, Domain &keydom) {
	// if domain is DOM_CURRENT we search through all _domains below the
	//    current domain.
	// otherwise, we search only the domain passed

	if (dom == DOM_CURRENT) {
		int d = static_cast<int>(_currentDomain);
		for (; d >= 0; --d) {
			if (_confFileMan->exists(getConfigKey(key, static_cast<Domain>(d)))) {
				keydom = static_cast<Domain>(d);
				return true;
			}
		}
		return false;
	} else {
		keydom = dom;
		return _confFileMan->exists(getConfigKey(key, dom));
	}
}

istring SettingManager::getConfigKey(istring key, Domain dom) {
	istring ckey;

	if (dom == DOM_CURRENT)
		dom = _currentDomain;

	if (dom == DOM_GLOBAL && ConfMan.hasKey(key))
		// Key exists in scummvm.ini, so can be used as is
		return key;

	if (dom == DOM_DEFAULTS) {
		ckey = "defaultsettings/";
	} else {
		ckey = "settings/" + _domains[dom];
	}

	istring::size_type pos = key.find('/');

	if (pos != istring::npos) {
		ckey += ":" + key;
	} else {
		ckey += "/" + key;
	}

	return ckey;
}

void SettingManager::callCallbacks(istring key) {
	Callbacks::iterator i;
	i = _callbacks.find(key);

	if (i == _callbacks.end()) return;

	Std::vector<ConfigCallback> &cb = (*i)._value;
	Std::vector<ConfigCallback>::iterator iter;
	for (iter = cb.begin(); iter != cb.end(); ++iter) {
		(*iter)(key);
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
