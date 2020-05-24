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
#include "common/hash-str.h"

namespace Ultima {
namespace Shared {

class XMLTree;

class XMLNode {
private:
	XMLTree *_tree;
	XMLNode *_parent;
	Common::String _id;
	Common::String _text;
	Common::Array<XMLNode *> _nodeList;
	Common::StringMap _attributes;
	bool _noClose;
	Common::String _emptyString;
private:
	static void parseDocTypeElement(const Common::String &s, size_t &nn);

	void parseNodeText(const Common::String &nodeText);

	/**
	 * Trim down any whitespaces in the passed text
	 */
	static void trim(Common::String &s);

	/**
	 * Parses a specified file and returns the root node for it
	 */
	static XMLNode *xmlParseFile(XMLTree *tree, const Common::String &fname);

	static Common::String closeTag(const Common::String &s);
	static Common::String encodeEntity(const Common::String &s);
public:
	XMLNode(XMLTree *tree, XMLNode *parent = nullptr) : _tree(tree), _parent(parent), _noClose(false) {}
	XMLNode(const XMLNode &n) : _tree(n._tree), _parent(n._parent), _id(n._id),
		_text(n._text), _nodeList(n._nodeList), _noClose(false) {}
	~XMLNode();

	XMLNode &operator=(const XMLNode &n) {
		_id = n._id;
		_text = n._text;
		_nodeList = n._nodeList;
		_noClose = n._noClose;
		return *this;
	}

	const Common::String &reference(const Common::String &, bool &);
	const XMLNode *subtree(const Common::String &) const;

	const Common::String &id() const {
		return _id;
	}
	const Common::String &text(void) const {
		return _text;
	}
	bool nodeIsText() const {
		return !_text.empty();
	}
	XMLNode *getParent() const {
		return _parent;
	}
	bool hasChildren() const {
		return !_nodeList.empty();
	}
	XMLNode *firstChild() const {
		return _nodeList.empty() ? nullptr : _nodeList[0];
	}
	const Common::StringMap &attributes() const {
		return _attributes;
	}
	bool hasProperty(const Common::String &attrName) const {
		return _attributes.contains(attrName);
	}
	Common::String getProperty(const Common::String &attrName) const {
		return _attributes.contains(attrName) ? _attributes[attrName] : "";
	}
	int getPropertyInt(const Common::String &attrName) const {
		return _attributes.contains(attrName) ? atol(_attributes[attrName].c_str()) : 0;
	}
	bool getPropertyBool(const Common::String &attrName) const {
		if (_attributes.contains(attrName)) {
			Common::String str = _attributes[attrName];
			return toupper(str[0]) == 'T' || str == "1";
		} else {
			return false;
		}
	}
	Common::String operator[](const Common::String &attrName) const {
		return getProperty(attrName);
	}

	const Common::Array<XMLNode *> &children() const {
		return _nodeList;
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
	static XMLNode *xmlParseDoc(XMLTree *tree, const Common::String &s);
	static XMLNode *xmlParse(XMLTree *tree, const Common::String &s, size_t &pos);

	void listKeys(const Common::String &, Common::Array<Common::String> &,
		bool longformat = true) const;

	/**
	 * Deletes the entire tree this node belongs to, including itself
	 */
	void freeDoc();

	/**
	 * Gets the prior sibling to this one
	 */
	XMLNode *getPrior() const;

	/**
	 * Gets the following sibling to this one
	 */
	XMLNode *getNext() const;
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
