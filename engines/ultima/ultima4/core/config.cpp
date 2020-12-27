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

#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/settings.h"

namespace Ultima {
namespace Ultima4 {

Config *Config::_instance;

Config::Config() {
	_instance = this;

	if (!_doc.readConfigFile("data/conf/config.xml"))
		error("Failed to read core configuration");
}

Config::~Config() {
	_instance = nullptr;
}

ConfigElement Config::getElement(const Common::String &name) const {
	Common::String key = Common::String::format("config/%s", name.c_str());
	const Shared::XMLNode *node = _doc.getNode(key);
	assert(node);
	return ConfigElement(node);
}

Std::vector<Common::String> Config::getGames() {
	Std::vector<Common::String> result;
	result.push_back("Ultima IV");
	return result;
}

void Config::setGame(const Common::String &name) {
	// No implementation
}

/*-------------------------------------------------------------------*/

ConfigElement::ConfigElement(const Shared::XMLNode *xmlNode) :
	_node(xmlNode), _name(xmlNode->id().c_str()) {
}

ConfigElement::ConfigElement(const ConfigElement &e) : _node(e._node), _name(e._name) {
}

ConfigElement::ConfigElement() : _node(nullptr) {
}

ConfigElement::~ConfigElement() {
}

ConfigElement &ConfigElement::operator=(const ConfigElement &e) {
	if (&e != this) {
		_node = e._node;
		_name = e._name;
	}
	return *this;
}

bool ConfigElement::exists(const Common::String &name) const {
	return !(*_node)[name].empty();
}

Common::String ConfigElement::getString(const Common::String &name) const {
	return (*_node)[name];
}

int ConfigElement::getInt(const Common::String &name, int defaultValue) const {
	Common::String str = (*_node)[name];
	return str.empty() ? defaultValue : atol(str.c_str());
}

bool ConfigElement::getBool(const Common::String &name) const {
	Common::String str = (*_node)[name];
	if (str.empty())
		return false;

	return toupper(str[0]) == 'T' || str == "1";
}

int ConfigElement::getEnum(const Common::String &name, const char *enumValues[]) const {
	Common::String str = (*_node)[name];
	if (str.empty())
		return 0;

	for (int i = 0; enumValues[i]; ++i) {
		if (str.equalsIgnoreCase(enumValues[i]))
			return i;
	}

	error("invalid enum value for %s: %s", name.c_str(), str.c_str());
}

Std::vector<ConfigElement> ConfigElement::getChildren() const {
	const Common::Array<Shared::XMLNode *> &children = _node->children();
	Std::vector<ConfigElement> result;

	for (Common::Array<Shared::XMLNode *>::const_iterator it = children.begin();
	        it != children.end(); ++it)
		result.push_back(*it);

	return result;
}

} // End of namespace Ultima4
} // End of namespace Ultima
