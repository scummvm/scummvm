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

#ifndef SHARED_CONF_XML_NODE_H
#define SHARED_CONF_XML_NODE_H

#include "ultima/shared/std/containers.h"
#include "common/str.h"
#include "common/array.h"

namespace Ultima {
namespace Shared {

class XMLNode {
private:
	Common::String _id;
	Common::String _content;
	Common::Array<XMLNode *> _nodeList;
	bool _noClose;
	Common::String _emptyString;

public:
	XMLNode() : _noClose(false) {}
	XMLNode(const Common::String &i) : _id(i), _noClose(false) {}
	XMLNode(const XMLNode &n) : _id(n._id), _content(n._content),
		_nodeList(n._nodeList), _noClose(false) {}
	~XMLNode();

	XMLNode &operator=(const XMLNode &n) {
		_id = n._id;
		_content = n._content;
		_nodeList = n._nodeList;
		_noClose = n._noClose;
		return *this;
	}

	const Common::String &reference(const Common::String &, bool &);
	const XMLNode *subtree(const Common::String &) const;

	const Common::String &value(void) const {
		return _content;
	}

	typedef Std::pair<Common::String, Common::String> KeyType;
	typedef Common::Array<KeyType> KeyTypeList;

	/**
	 * Returns a list of key->value pairs that are found under the provided 'basekey'.
	 * Ignores comments (<!-- ... --> and doesn't return them.
	 * Returns true if search is 'finished'
	 */
	bool searchPairs(KeyTypeList &ktl, const Common::String &basekey,
		const Common::String currkey, const unsigned int pos);
	void selectPairs(KeyTypeList &ktl, const Common::String currkey);

	Common::String dump(int depth = 0);

	void xmlAssign(const Common::String &key, const Common::String &value);
	void xmlParse(const Common::String &s, size_t &pos);

	void listKeys(const Common::String &, Common::Array<Common::String> &,
		bool longformat = true) const;
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
