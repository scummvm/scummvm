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
	: tree(new XMLNode("config")), root("config"), is_file(false),
	  readonly(false) {

}

XMLTree::XMLTree(const Common::String &fname, const Common::String &root_)
	: tree(new XMLNode(root_)), root(root_), is_file(true),
	  readonly(false) {
	readConfigFile(fname);
}

XMLTree::~XMLTree() {
	delete tree;
}

void XMLTree::clear(const Common::String &root_) {
	delete tree;
	tree = new XMLNode(root_);
	root = root_;
	is_file = false;
	readonly = false;
}

bool XMLTree::readConfigFile(const Common::String &fname) {
	Common::File f;

//	if (!FileSystem::get_instance()->rawopen(f, fname, true))
//		return false;
// mode = Std::ios::in;

	if (f.open(fname)) {
		warning("Error opening config file");
		return false;
	}

	Common::String sbuf, line;
	while (!f.err() && !f.eos()) {
		line = f.readLine();
		sbuf += line;
	}

	f.close();

	if (!readConfigString(sbuf))
		return false;

	is_file = true; // readConfigString sets is_file = false
	filename = fname;
	return true;
}

bool XMLTree::readConfigString(const Common::String &s) {
	is_file = false;

	Common::String sbuf(s);
	size_t nn = 0;
	while (Common::isSpace(s[nn]))
		++nn;

	if (s[nn] != '<') {
		warning("expected '<' while reading config file, found %c\n", s[nn]);
		return false;
	}
	++nn;

	tree->xmlParse(sbuf, nn);

	return true;
}

Common::String XMLTree::dump() {
	return tree->dump();
}

void XMLTree::write() {
	if (!is_file || readonly)
		return;

	Common::DumpFile df;

	if (df.open(filename)) {
		Common::String content = dump();
		df.write(content.c_str(), content.size());
		df.close();
	}
}

bool XMLTree::hasNode(Common::String key) const {
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		return true;
	else
		return false;
}

bool XMLTree::checkRoot(Common::String key) const {
	Common::String k = key.substr(0, key.find('/'));
	return (k == root);
}

void XMLTree::value(Common::String key, Common::String &ret,
                    const char *defaultvalue) const {
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		ret = sub->value();
	else
		ret = defaultvalue;
}

void XMLTree::value(Common::String key, int &ret,
                    int defaultvalue) const {
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		ret = strtol(sub->value().c_str(), 0, 0);
	else
		ret = defaultvalue;
}

void XMLTree::value(Common::String key, bool &ret,
                    bool defaultvalue) const {
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		ret = sub->value().equalsIgnoreCase("YES");
	else
		ret = defaultvalue;
}

void XMLTree::set(Common::String key, Common::String value) {
	tree->xmlAssign(key, value);
}

void XMLTree::set(Common::String key, const char *value) {
	tree->xmlAssign(key, value);
}

void XMLTree::set(Common::String key, int value) {
	char buf[32];
	snprintf(buf, 32, "%d", value);
	set(key, buf);
}

void XMLTree::set(Common::String key, bool value) {
	if (value)
		set(key, "yes");
	else
		set(key, "no");
}

Common::Array<Common::String> XMLTree::listKeys(Common::String key, bool longformat) {
	Common::Array<Common::String> keys;
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		sub->listKeys(key, keys, longformat);

	return keys;
}

void XMLTree::getSubkeys(KeyTypeList &ktl, Common::String basekey) {
	tree->searchPairs(ktl, basekey, Common::String(), 0);
}

} // End of namespace Shared
} // End of namespace Ultima
