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

#include "ultima/shared/conf/xml_tree.h"
#include "ultima/shared/conf/xml_node.h"
#include "ultima/shared/std/misc.h"
#include "common/algorithm.h"
#include "common/file.h"

namespace Ultima {
namespace Shared {

XMLTree::XMLTree()
		: _tree(new XMLNode("config")), _root("config"), _isFile(false),
		_readOnly(false) {
}

XMLTree::XMLTree(const Common::String &fname, const Common::String &root)
		: _tree(new XMLNode(root)), _root(root), _isFile(true),
	  _readOnly(false) {
	readConfigFile(fname);
}

XMLTree::XMLTree(Common::SeekableReadStream *stream, const Common::String &root)
		: _tree(new XMLNode(root)), _root(root), _isFile(true),
		_readOnly(false) {
	readConfigStream(stream);
}

XMLTree::~XMLTree() {
	delete _tree;
}

void XMLTree::clear(const Common::String &root) {
	delete _tree;
	_tree = new XMLNode(root);
	_root = root;
	_isFile = false;
	_readOnly = false;
}

bool XMLTree::readConfigFile(const Common::String &fname) {
	Common::File f;

	if (!f.open(fname)) {
		warning("Error opening config file");
		return false;
	}

	bool result = readConfigStream(&f);

	f.close();

	_filename = fname;
	return result;
}

bool XMLTree::readConfigStream(Common::SeekableReadStream *stream) {
	Common::String sbuf, line;
	while (!stream->err() && !stream->eos()) {
		line = stream->readLine();
		sbuf += line;
	}

	if (!readConfigString(sbuf))
		return false;

	_isFile = true;		// readConfigString sets _isFile = false
	return true;
}

bool XMLTree::readConfigString(const Common::String &s) {
	_isFile = false;
	_filename.clear();

	Common::String sbuf(s);
	size_t nn = 0;
	bool parsedXmlElement = false, parsedDocType = false;

	for (;;) {
		while (nn < s.size() && Common::isSpace(s[nn]))
			++nn;

		if (nn >= s.size()) {
			warning("Unexpected end of XML");
			return false;
		}
		if (s[nn] != '<') {
			warning("expected '<' while reading config file, found %c\n", s[nn]);
			return false;
		}
		++nn;

		if (nn < s.size() && s[nn] == '?') {
			assert(!parsedXmlElement);
			parsedXmlElement = true;
			nn = s.findFirstOf('>', nn);
		} else if (nn < s.size() && s.substr(nn, 8).equalsIgnoreCase("!doctype")) {
			assert(!parsedDocType);
			parsedDocType = true;
			parseDocTypeElement(s, nn);
		} else {
			_tree->xmlParse(sbuf, nn);
			continue;
		}

		// If this point was reached, we just skipped ?xml or doctype element
		++nn;
	}

	return true;
}

void XMLTree::parseDocTypeElement(const Common::String &s, size_t &nn) {
	nn = s.findFirstOf(">[", nn);
	if (nn == Common::String::npos)
		// No ending tag
		return;

	if (s[nn] == '[') {
		// Square bracketed area
		nn = s.findFirstOf(']', nn) + 1;
	}

	if (nn >= s.size() || s[nn] != '>')
		nn = Common::String::npos;
}

Common::String XMLTree::dump() {
	return _tree->dump();
}

void XMLTree::write() {
	if (!_isFile || _readOnly)
		return;

	Common::DumpFile df;

	if (df.open(_filename)) {
		Common::String content = dump();
		df.write(content.c_str(), content.size());
		df.close();
	}
}

const XMLNode *XMLTree::getNode(const Common::String &key) const {
	return _tree->subtree(key);
}

bool XMLTree::hasNode(const Common::String &key) const {
	const XMLNode *sub = _tree->subtree(key);
	if (sub)
		return true;
	else
		return false;
}

bool XMLTree::checkRoot(const Common::String &key) const {
	Common::String k = key.substr(0, key.find('/'));
	return (k == _root);
}

void XMLTree::value(const Common::String &key, Common::String &ret,
                    const char *defaultvalue) const {
	const XMLNode *sub = _tree->subtree(key);
	if (sub)
		ret = sub->value();
	else
		ret = defaultvalue;
}

void XMLTree::value(const Common::String &key, int &ret,
                    int defaultvalue) const {
	const XMLNode *sub = _tree->subtree(key);
	if (sub)
		ret = strtol(sub->value().c_str(), 0, 0);
	else
		ret = defaultvalue;
}

void XMLTree::value(const Common::String &key, bool &ret,
                    bool defaultvalue) const {
	const XMLNode *sub = _tree->subtree(key);
	if (sub)
		ret = sub->value().equalsIgnoreCase("YES");
	else
		ret = defaultvalue;
}

void XMLTree::set(const Common::String &key, const Common::String &value) {
	_tree->xmlAssign(key, value);
}

void XMLTree::set(const Common::String &key, const char *value) {
	_tree->xmlAssign(key, value);
}

void XMLTree::set(const Common::String &key, int value) {
	char buf[32];
	snprintf(buf, 32, "%d", value);
	set(key, buf);
}

void XMLTree::set(const Common::String &key, bool value) {
	if (value)
		set(key, "yes");
	else
		set(key, "no");
}

Common::Array<Common::String> XMLTree::listKeys(const Common::String &key, bool longformat) {
	Common::Array<Common::String> keys;
	const XMLNode *sub = _tree->subtree(key);
	if (sub)
		sub->listKeys(key, keys, longformat);

	return keys;
}

void XMLTree::getSubkeys(KeyTypeList &ktl, const Common::String &basekey) {
	_tree->searchPairs(ktl, basekey, Common::String(), 0);
}

} // End of namespace Shared
} // End of namespace Ultima
