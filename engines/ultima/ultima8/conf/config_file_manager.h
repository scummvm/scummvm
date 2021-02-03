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

#ifndef ULTIMA8_CONF_CONFIGFILEMANAGER_H
#define ULTIMA8_CONF_CONFIGFILEMANAGER_H

#include "ultima/shared/std/string.h"
#include "ultima/ultima8/misc/istring.h"
#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/conf/ini_file.h"

namespace Ultima {
namespace Ultima8 {

class ConfigFileManager {
public:
	ConfigFileManager();
	~ConfigFileManager();

	static ConfigFileManager *get_instance() {
		return _configFileManager;
	}

	//! read a config file. Multiple files may be read. Order is important.
	//! \param fname The file to read
	//! \param root The name of the root node in the file
	//! \param readonly If true, don't write to this file's tree (or the file)
	//! \return true if succesful
	bool readConfigFile(Std::string fname, istring root);

	//! clear everything
	void clear();

	//! clear everything in a root
	void clearRoot(istring root);

	//! does the key exist?
	bool exists(istring key);

	//! get value
	bool get(istring key, Std::string &ret);
	//! get value
	bool get(istring key, int &ret);
	//! get value
	bool get(istring, bool &ret);

	//! list all sections
	//! \param root The config root to list all sections in
	//! \param longformat If true, return the full key name (including section)
	//! \return the sections. They have no guaranteed order.
	Std::vector<istring> listSections(istring root,
	        bool longformat = false);

	//! list all key-value pairs in the given section.
	//! \param section The section to list
	//! \param longformat If true, return the full key name (including section)
	//! \return the key-value pairs. They have no guaranteed order.
	KeyMap listKeyValues(istring section, bool longformat = false);

private:

	INIFile *findKeyINI(istring key);

	Std::vector<INIFile *> _iniFiles;

	static ConfigFileManager *_configFileManager;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
