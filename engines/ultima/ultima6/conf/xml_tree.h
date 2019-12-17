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

#ifndef ULTIMA6_CONF_XML_TREE_H
#define ULTIMA6_CONF_XML_TREE_H

#include <string>
#include <vector>

namespace Ultima {
namespace Ultima6 {

std::string to_uppercase(std::string s);

class XMLNode;

class XMLTree {
public:
	XMLTree();
	XMLTree(std::string fname, std::string root);
	~XMLTree();

	bool readConfigFile(std::string fname);
	bool readConfigString(std::string s);

	void clear(std::string root);

	std::string dump();
	void write();

	void setReadonly() {
		readonly = true;
	}
	bool isReadonly() const {
		return readonly;
	}

	bool hasNode(std::string key) const;
	bool checkRoot(std::string key) const;

	// get value
	void value(std::string key, std::string &ret,
	           const char *defaultvalue = "") const;
	void value(std::string key, int &ret,
	           int defaultvalue = 0) const;
	void value(std::string key, bool &ret,
	           bool defaultvalue = false) const;

	// set value
	void set(std::string key, std::string value);
	void set(std::string key, const char *value);
	void set(std::string key, int value);
	void set(std::string key, bool value);

	std::vector<std::string> listKeys(std::string key, bool longformat = false);

	typedef std::pair<std::string, std::string> KeyType;
	typedef std::vector<KeyType> KeyTypeList;

	void getSubkeys(KeyTypeList &ktl, std::string basekey);

private:
	XMLNode *tree;
	std::string filename;
	std::string root;
	bool is_file;
	bool readonly;
};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
