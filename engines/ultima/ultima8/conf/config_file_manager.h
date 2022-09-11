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

#ifndef ULTIMA8_CONF_CONFIGFILEMANAGER_H
#define ULTIMA8_CONF_CONFIGFILEMANAGER_H

#include "common/ini-file.h"
#include "ultima/shared/std/string.h"
#include "ultima/ultima8/misc/istring.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima8 {

typedef Common::HashMap<istring, Std::string, Common::IgnoreCase_Hash> KeyMap;

class ConfigFileManager {
public:
	ConfigFileManager();
	~ConfigFileManager();

	struct ConfigFile {
		istring _category;
		Common::INIFile _iniFile;
	};

	static ConfigFileManager *get_instance() {
		return _configFileManager;
	}

	//! read a config file. Multiple files may be read. Order is important.
	//! \param fname The file to read
	//! \param root The name of the root node in the file
	//! \param readonly If true, don't write to this file's tree (or the file)
	//! \return true if succesful
	bool readConfigFile(Std::string fname, const istring &category);

	//! clear everything
	void clear();

	//! clear everything in a root
	void clearRoot(const istring &category);

	//! get value
	bool get(const istring &category, const istring &section, const istring &key, Std::string &ret);
	//! get value
	bool get(const istring &category, const istring &section, const istring &key, int &ret);
	//! get value
	bool get(const istring &category, const istring &section, const istring &key, bool &ret);

	//! list all sections
	//! \param category The config category to list all sections in
	//! \return the sections. They have no guaranteed order.
	Std::vector<istring> listSections(const istring &category);
	//! list all key-value pairs in the given section.
	//! \param category The config category for the section to list
	//! \param section The section to list
	//! \return the key-value pairs. They have no guaranteed order.
	KeyMap listKeyValues(const istring &category, const istring &section);

private:
	Std::vector<ConfigFile *> _configFiles;

	static ConfigFileManager *_configFileManager;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
