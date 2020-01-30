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

#ifndef NUVIE_CONF_XML_TREE_H
#define NUVIE_CONF_XML_TREE_H

#include "ultima/shared/std/string.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/misc.h"

namespace Ultima {
namespace Nuvie {

Std::string to_uppercase(Std::string s);

class XMLNode;

class XMLTree {
public:
	XMLTree();
	XMLTree(Std::string fname, Std::string root);
	~XMLTree();

	bool readConfigFile(Std::string fname);
	bool readConfigString(Std::string s);

	void clear(Std::string root);

	Std::string dump();
	void write();

	void setReadonly() {
		readonly = true;
	}
	bool isReadonly() const {
		return readonly;
	}

	bool hasNode(Std::string key) const;
	bool checkRoot(Std::string key) const;

	// get value
	void value(Std::string key, Std::string &ret,
	           const char *defaultvalue = "") const;
	void value(Std::string key, int &ret,
	           int defaultvalue = 0) const;
	void value(Std::string key, bool &ret,
	           bool defaultvalue = false) const;

	// set value
	void set(Std::string key, Std::string value);
	void set(Std::string key, const char *value);
	void set(Std::string key, int value);
	void set(Std::string key, bool value);

	Std::vector<Std::string> listKeys(Std::string key, bool longformat = false);

	typedef Std::pair<Std::string, Std::string> KeyType;
	typedef Std::vector<KeyType> KeyTypeList;

	void getSubkeys(KeyTypeList &ktl, Std::string basekey);

private:
	XMLNode *tree;
	Std::string filename;
	Std::string root;
	bool is_file;
	bool readonly;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
