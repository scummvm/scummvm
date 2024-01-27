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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#ifndef GOB_INICONFIG_H
#define GOB_INICONFIG_H

#include "common/formats/ini-file.h"
#include "common/hashmap.h"
#include "common/str.h"

#include "gob.h"

namespace Gob {

class INIConfig {
public:
	INIConfig(GobEngine *vm);
	~INIConfig();

	bool getValue(Common::String &result, const Common::String &file,
			const Common::String &section, const Common::String &key,
			const Common::String &def = "");

	bool setValue(const Common::String &file, const Common::String &section,
			const Common::String &key, const Common::String &value);

private:
	struct Config {
		Common::INIFile *config;
		bool created;
	};

	typedef Common::HashMap<Common::String, Config, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> ConfigMap;

	GobEngine *_vm;
	ConfigMap _configs;

	bool getConfig(const Common::String &file, Config &config);

	bool readConfigFromDisk(const Common::String &file, Config &config);
	bool openConfig(const Common::String &file, Config &config);
	bool createConfig(const Common::String &file, Config &config);
};

} // End of namespace Gob

#endif // GOB_INICONFIG_H
