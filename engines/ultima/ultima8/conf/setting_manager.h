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

#ifndef ULTIMA8_CONF_SETTINGMANAGER_H
#define ULTIMA8_CONF_SETTINGMANAGER_H

#include "ultima/ultima8/conf/config_file_manager.h"
#include "ultima/ultima8/misc/istring.h"
#include "ultima/ultima8/std/string.h"
#include "ultima/ultima8/std/containers.h"
#include "ultima/ultima8/std/containers.h"

namespace Ultima8 {

class ConfigFileManager;

class SettingManager {
public:
	SettingManager();
	~SettingManager();

	static SettingManager *get_instance() {
		return settingmanager;
	}

	enum Domain {
		DOM_DEFAULTS = 0,
		DOM_GLOBAL   = 1,
		DOM_GAME     = 2,
		DOM_CURRENT  = 100
	};

	typedef void (*ConfigCallback)(Pentagram::istring key);
	typedef std::map<Common::String, std::vector<ConfigCallback>, Common::IgnoreCase_Hash > Callbacks;

	//! read a config file. Multiple files may be read. Order is important.
	//! \param fname The file to read
	//! \param readonly If true, don't write to this file's tree (or the file)
	//! \return true if succesful
	bool readConfigFile(std::string fname, bool readonly = false);

	//! write all (writable) config files
	void write();

	//! does the key exist?
	bool exists(Pentagram::istring key, Domain dom = DOM_CURRENT);

	//! get value
	bool get(Pentagram::istring key, std::string &ret, Domain dom = DOM_CURRENT);
	//! get value
	bool get(Pentagram::istring key, int &ret, Domain dom = DOM_CURRENT);
	//! get value
	bool get(Pentagram::istring key, bool &ret, Domain dom = DOM_CURRENT);

	//! set value
	void set(Pentagram::istring key, std::string value, Domain dom = DOM_CURRENT);
	//! set value
	void set(Pentagram::istring key, const char *value, Domain dom = DOM_CURRENT);
	//! set value
	void set(Pentagram::istring key, int value, Domain dom = DOM_CURRENT);
	//! set value
	void set(Pentagram::istring key, bool value, Domain dom = DOM_CURRENT);

	//! remove key
	void unset(Pentagram::istring key, Domain dom = DOM_CURRENT);

	//! set default value
	void setDefault(Pentagram::istring key, std::string value);
	//! set default value
	void setDefault(Pentagram::istring key, const char *value);
	//! set default value
	void setDefault(Pentagram::istring key, int value);
	//! set default value
	void setDefault(Pentagram::istring key, bool value);

	//! set the current domain
	void setCurrentDomain(Domain dom);
	//! set the configuration section for a domain
	void setDomainName(Domain dom, Pentagram::istring section);

	//! register a function to be called when the key changes
	void registerCallback(Pentagram::istring key, ConfigCallback callback);
	//! unregister a callback
	void unregisterCallback(Pentagram::istring key, ConfigCallback callback);

	//! list all games
	std::vector<Pentagram::istring> listGames();

	//! list all keys in a game data section
	//! \param section The section to return setkeys of
	//! \return the keys. They have no guaranteed order.
	std::vector<Pentagram::istring> listDataKeys(Pentagram::istring section);

	//! list all key-value pairs in the given section.
	//! \param section The section to list
	//! \return the key-value pairs. They have no guaranteed order.
	KeyMap listDataValues(Pentagram::istring section);

private:

	bool findKeyDomain(Pentagram::istring key, Domain dom, Domain &keydom);
	Pentagram::istring getConfigKey(Pentagram::istring key, Domain dom);
	void callCallbacks(Pentagram::istring key);

	Callbacks callbacks;
	std::vector<Pentagram::istring> domains;

	Domain currentDomain;

	ConfigFileManager *conffileman;

	static SettingManager *settingmanager;
};

} // End of namespace Ultima8

#endif
