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

#ifndef SHARED_CONF_XML_TREE_H
#define SHARED_CONF_XML_TREE_H

#include "common/str.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Shared {

class XMLNode;

class XMLTree {
private:
	XMLNode *tree;
	Common::String filename;
	Common::String root;
	bool is_file;
	bool readonly;
public:
	XMLTree();
	XMLTree(const Common::String &fname, const Common::String &root);
	~XMLTree();

	bool readConfigFile(const Common::String &fname);
	bool readConfigString(const Common::String &s);

	void clear(const Common::String &root);

	Common::String dump();
	void write();

	void setReadonly() {
		readonly = true;
	}
	bool isReadonly() const {
		return readonly;
	}

	bool hasNode(Common::String key) const;
	bool checkRoot(Common::String key) const;

	// get value
	void value(Common::String key, Common::String &ret,
	           const char *defaultvalue = "") const;
	void value(Common::String key, int &ret,
	           int defaultvalue = 0) const;
	void value(Common::String key, bool &ret,
	           bool defaultvalue = false) const;

	// set value
	void set(Common::String key, Common::String value);
	void set(Common::String key, const char *value);
	void set(Common::String key, int value);
	void set(Common::String key, bool value);

	Common::Array<Common::String> listKeys(Common::String key, bool longformat = false);

	typedef Std::pair<Common::String, Common::String> KeyType;
	typedef Common::Array<KeyType> KeyTypeList;

	void getSubkeys(KeyTypeList &ktl, Common::String basekey);
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
