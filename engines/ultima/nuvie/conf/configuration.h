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

#ifndef NUVIE_CONF_CONFIGURATION_H
#define NUVIE_CONF_CONFIGURATION_H

#include "ultima/shared/std/string.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/misc.h"
#include "ultima/detection.h"

namespace Ultima {
namespace Shared {
class XMLTree;
} // End of namespace Shared

namespace Nuvie {

class ConfigNode;

#define NUVIE_CONF_READONLY true
#define NUVIE_CONF_READWRITE false

/**
 * Configuration class.
 *
 * Configuration values are stored in one of two ways -either as a standalone
 * nuvie.cfg file, or otherwise from the ScummVM domain for the added game.
 *
 * WHen the nuvie.cfg file is present, it's contents are stored as an XML tree
 * (or a forest, technically). All values are stored as strings, but access
 * functions for ints and bools are provided
 * You should only store values in leaf nodes. (This isn't enforced everywhere,
 * but contents of non-leaf nodes can disappear without warning)
 *
 * You can load multiple config files, which can be read-only.
 * Each config file contains a single tree.
 * Values in files loaded last override values in files loaded earlier.
 * Values are written to the last-loaded writable config file with the right root.
 * Because of this it's important to make sure the last-loaded config file with
 * a given root is writable. (The idea is that you can load a system-wide config
 * file first, and a user's config file after that.)
 */
class Configuration {
private:
	Std::vector<Shared::XMLTree*> _trees;
    Common::HashMap<Common::String, Common::String, Common::IgnoreCase_Hash,
        Common::IgnoreCase_EqualTo> _localKeys;
	Common::HashMap<Common::String, Common::String, Common::IgnoreCase_Hash,
		Common::IgnoreCase_EqualTo> _settings;
	Std::string _configFilename;
    bool _configChanged;

    // Sets default configurations common to both enhanced and unhenaced
    void setCommonDefaults(GameId gameType);

	// sets up unenhanced version defaults
	void setUnenhancedDefaults(GameId gameType);

	// sets up enhanced version defaults
	void setEnhancedDefaults(GameId gameType);
public:
	Configuration();
	~Configuration();

	// read config file. Multiple files may be read. Order is important.
	bool readConfigFile(Std::string fname, Std::string root, bool readonly = true);

    // Returns true if default settings for game have previously been set
    bool isDefaultsSet() const;

	// Loads up the configuration settings
	void load(GameId gameId, bool isEnhanced);

	// write all (writable) config files
	void write();

	// clear everything
	void clear();

	Std::string filename() const {
		return _configFilename;
	}

	// get value
	void value(const Std::string &key, Std::string &ret, const char *defaultvalue = "");
	void value(const Std::string &key, int &ret, int defaultvalue = 0);
	void value(const Std::string &key, bool &ret, bool defaultvalue = false);

	void pathFromValue(const Std::string &key, Std::string file, Std::string &full_path);

	// set value
	bool set(const Std::string &key, const Std::string &value);
	bool set(const Std::string &key, const char *value);
	bool set(const Std::string &key, int value);
	bool set(const Std::string &key, bool value);

	// get node ref. (delete it afterwards)
	ConfigNode *getNode(const Std::string &key);

	// list all subkeys of a key. (no guaranteed order in result)
	Std::set<Std::string> listKeys(const Std::string &key, bool longformat = false);

	typedef Std::pair<Common::String, Common::String> KeyType;
	typedef Common::Array<KeyType> KeyTypeList;

	void getSubkeys(KeyTypeList &ktl, Std::string basekey);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
