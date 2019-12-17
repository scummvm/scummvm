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

#include "ultima/ultima6/core/nuvie_defs.h"
#include "ultima/ultima6/conf/misc.h"
#include "ultima/ultima6/conf/configuration.h"
#include "ultima/ultima6/conf/xml_tree.h"
#include "ultima/ultima6/conf/config_node.h"

namespace Ultima {
namespace Ultima6 {

Configuration::Configuration() {
	config_filename = "";
}

Configuration::~Configuration() {
	for (std::vector<XMLTree *>::iterator i = trees.begin();
	        i != trees.end(); ++i) {
		delete(*i);
	}
}

bool Configuration::readConfigFile(std::string fname, std::string root,
                                   bool readonly) {
	config_filename = fname;
	XMLTree *tree = new XMLTree();
	tree->clear(root);
	if (!tree->readConfigFile(fname)) {
		delete tree;
		return false;
	}

	trees.push_back(tree);
	return true;
}

void Configuration::write() {
	for (std::vector<XMLTree *>::iterator i = trees.begin();
	        i != trees.end(); ++i) {
		if (!(*i)->isReadonly())
			(*i)->write();
	}
}

void Configuration::clear() {
	for (std::vector<XMLTree *>::iterator i = trees.begin();
	        i != trees.end(); ++i) {
		delete(*i);
	}
	trees.clear();
}

void Configuration::value(std::string key, std::string &ret,
                          const char *defaultvalue) {
	for (std::vector<XMLTree *>::reverse_iterator i = trees.rbegin();
	        i != trees.rend(); ++i) {
		if ((*i)->hasNode(key)) {
			(*i)->value(key, ret, defaultvalue);
			return;
		}
	}

	ret = defaultvalue;
}

void Configuration::value(std::string key, int &ret, int defaultvalue) {
	for (std::vector<XMLTree *>::reverse_iterator i = trees.rbegin();
	        i != trees.rend(); ++i) {
		if ((*i)->hasNode(key)) {
			(*i)->value(key, ret, defaultvalue);
			return;
		}
	}

	ret = defaultvalue;
}

void Configuration::value(std::string key, bool &ret, bool defaultvalue) {
	for (std::vector<XMLTree *>::reverse_iterator i = trees.rbegin();
	        i != trees.rend(); ++i) {
		if ((*i)->hasNode(key)) {
			(*i)->value(key, ret, defaultvalue);
			return;
		}
	}

	ret = defaultvalue;
}

void Configuration::pathFromValue(std::string key, std::string file, std::string &full_path) {
	value(key, full_path);

	if (full_path.length() > 0 && full_path[full_path.length() - 1] != U6PATH_DELIMITER)
		full_path += U6PATH_DELIMITER + file;
	else
		full_path += file;

	return;
}

bool Configuration::set(std::string key, std::string value) {
	// Currently a value is written to the last writable tree with
	// the correct root.

	for (std::vector<XMLTree *>::reverse_iterator i = trees.rbegin();
	        i != trees.rend(); ++i) {
		if (!((*i)->isReadonly()) &&
		        (*i)->checkRoot(key)) {
			(*i)->set(key, value);
			return true;
		}
	}

	DEBUG(0, LEVEL_CRITICAL, "No writable config file found: unable to set value");
	// we could maybe add a non-file XMLTree to the list and save to that?
	// OTOH, that might make things very untransparent
	return false;
}

bool Configuration::set(std::string key, const char *value) {
	return set(key, std::string(value));
}


bool Configuration::set(std::string key, int value) {
	// Currently a value is written to the last writable tree with
	// the correct root.

	for (std::vector<XMLTree *>::reverse_iterator i = trees.rbegin();
	        i != trees.rend(); ++i) {
		if (!((*i)->isReadonly()) &&
		        (*i)->checkRoot(key)) {
			(*i)->set(key, value);
			return true;
		}
	}

	DEBUG(0, LEVEL_CRITICAL, "No writable config file found: unable to set value");
	return false;
}

bool Configuration::set(std::string key, bool value) {
	// Currently a value is written to the last writable tree with
	// the correct root.

	for (std::vector<XMLTree *>::reverse_iterator i = trees.rbegin();
	        i != trees.rend(); ++i) {
		if (!((*i)->isReadonly()) &&
		        (*i)->checkRoot(key)) {
			(*i)->set(key, value);
			return true;
		}
	}

	DEBUG(0, LEVEL_CRITICAL, "No writable config file found: unable to set value");
	return false;
}

ConfigNode *Configuration::getNode(std::string key) {
	return new ConfigNode(*this, key);
}

std::set<std::string> Configuration::listKeys(std::string key, bool longformat) {
	std::set<std::string> keys;
	for (std::vector<XMLTree *>::iterator i = trees.begin();
	        i != trees.end(); ++i) {
		std::vector<std::string> k = (*i)->listKeys(key, longformat);
		for (std::vector<std::string>::iterator iter = k.begin();
		        iter != k.end(); ++iter) {
			keys.insert(*iter);
		}
	}
	return keys;
}

void Configuration::getSubkeys(KeyTypeList &ktl, std::string basekey) {
	for (std::vector<XMLTree *>::iterator tree = trees.begin();
	        tree != trees.end(); ++tree) {
		KeyTypeList l;
		(*tree)->getSubkeys(l, basekey);

		for (KeyTypeList::iterator i = l.begin();
		        i != l.end(); ++i) {
			bool found = false;
			for (KeyTypeList::iterator j = ktl.begin();
			        j != ktl.end() && !found; ++j) {
				if (j->first == i->first) {
					// already have this subkey, so just replace the value
					j->second = i->second;
					found = true;
				}
			}
			if (!found) {
				// new subkey
				ktl.push_back(*i);
			}
		}
	}
}

} // End of namespace Ultima6
} // End of namespace Ultima
