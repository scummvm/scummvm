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

#ifndef NUVIE_CONF_CONFIG_NODE_H
#define NUVIE_CONF_CONFIG_NODE_H

#include "ultima/shared/std/string.h"
#include "ultima/nuvie/conf/configuration.h"

namespace Ultima {
namespace Nuvie {

class ConfigNode {
	friend class Configuration;
	ConfigNode(Configuration &config_, Std::string key_)
		: config(config_), key(key_) {
	}

public:
	~ConfigNode() { }

	// fix "assignment operator could not be generated" warning
	const ConfigNode &operator = (const ConfigNode &other) {
		config = other.config;
		key = other.key;
		return *this;
	}

	Std::string get_string(const char *defaultvalue = "") {
		Std::string s;
		config.value(key, s, defaultvalue);
		return s;
	}
	int get_int(int defaultvalue = 0) {
		int i;
		config.value(key, i, defaultvalue);
		return i;
	}
	bool get_bool(bool defaultvalue = false) {
		bool b;
		config.value(key, b, defaultvalue);
		return b;
	}

	void set(Std::string value) {
		config.set(key, value);
	}
	void set(const char *value) {
		config.set(key, value);
	}
	void set(int value) {
		config.set(key, value);
	}
	void set(bool value) {
		config.set(key, value);
	}

private:
	Configuration &config;
	Std::string key;

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
