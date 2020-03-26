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
#include "common/stream.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Shared {

class XMLNode;

class XMLTree {
	friend class XMLNode;
private:
	XMLNode *_tree;
	Common::String _filename;
	bool _isFile;
	bool _readOnly;
public:
	XMLTree();
	XMLTree(const Common::String &fname);
	XMLTree(Common::SeekableReadStream *stream);
	~XMLTree();

	bool readConfigFile(const Common::String &fname);
	bool readConfigStream(Common::SeekableReadStream *stream);
	bool readConfigString(const Common::String &s);

	void clear();

	Common::String dump();
	void write();

	void setReadonly() {
		_readOnly = true;
	}
	bool isReadonly() const {
		return _readOnly;
	}
	XMLNode *getTree() const {
		return _tree;
	}

	const XMLNode *getNode(const Common::String &key) const;
	bool hasNode(const Common::String &key) const;
	bool checkRoot(const Common::String &key) const;

	// get value
	void value(const Common::String &key, Common::String &ret,
		const char *defaultvalue = "") const;
	void value(const Common::String &key, int &ret, int defaultvalue = 0) const;
	void value(const Common::String &key, bool &ret, bool defaultvalue = false) const;

	// set value
	void set(const Common::String &key, const Common::String &value);
	void set(const Common::String &key, const char *value);
	void set(const Common::String &key, int value);
	void set(const Common::String &key, bool value);

	Common::Array<Common::String> listKeys(const Common::String &key, bool longformat = false);

	typedef Std::pair<Common::String, Common::String> KeyType;
	typedef Common::Array<KeyType> KeyTypeList;

	void getSubkeys(KeyTypeList &ktl, const Common::String &basekey);
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
