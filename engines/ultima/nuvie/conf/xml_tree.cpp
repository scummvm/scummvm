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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/conf/misc.h"
#include "ultima/nuvie/conf/xml_tree.h"
#include "ultima/nuvie/conf/xml_node.h"
#include "ultima/shared/std/string.h"
#include "common/algorithm.h"
#include "common/file.h"

namespace Ultima {
namespace Nuvie {

Std::string to_uppercase(Std::string s) {
	Std::string str = s;
	Std::string::iterator X;
	for (X = str.begin(); X != str.end(); ++X)
		*X = Std::toUpper(*X);

	return str;
}

XMLTree::XMLTree()
	: tree(new XMLNode("config")), root("config"), is_file(false),
	  readonly(false) {

}

XMLTree::XMLTree(Std::string fname, Std::string root_)
	: tree(new XMLNode(root_)), root(root_), is_file(true),
	  readonly(false) {
	readConfigFile(fname);
}

XMLTree::~XMLTree() {
	delete tree;
}

void XMLTree::clear(Std::string root_) {
	delete tree;
	tree = new XMLNode(root_);
	root = root_;
	is_file = false;
	readonly = false;
}

bool XMLTree::readConfigFile(Std::string fname) {
	Common::File f;

//	if (!FileSystem::get_instance()->rawopen(f, fname, true))
//		return false;
// mode = Std::ios::in;

	if (f.open(fname)) {
		DEBUG(0, LEVEL_CRITICAL, "Error opening config file");
		return false;
	}

	Std::string sbuf, line;
	while (!f.err() && !f.eos()) {
		line = readLine(&f);
		sbuf += line;
	}

	f.close();

	if (!readConfigString(sbuf))
		return false;

	is_file = true; // readConfigString sets is_file = false
	filename = fname;
	return true;
}

bool XMLTree::readConfigString(Std::string s) {
	is_file = false;

	Std::string sbuf(s);
	size_t nn = 0;
	while (Common::isSpace(s[nn]))
		++nn;

	if (s[nn] != '<') {
		DEBUG(0, LEVEL_ERROR, "expected '<' while reading config file, found %c\n", s[nn]);
		return false;
	}
	++nn;

	tree->xmlparse(sbuf, nn);

	return true;
}

Std::string XMLTree::dump() {
	return tree->dump();
}

void XMLTree::write() {
	if (!is_file || readonly)
		return;

	Common::DumpFile df;

	if (df.open(filename)) {
		Std::string content = dump();
		df.write(content.c_str(), content.size());
		df.close();
	}
}

bool XMLTree::hasNode(Std::string key) const {
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		return true;
	else
		return false;
}

bool XMLTree::checkRoot(Std::string key) const {
	Std::string k = key.substr(0, key.find('/'));
	return (k == root);
}

void XMLTree::value(Std::string key, Std::string &ret,
                    const char *defaultvalue) const {
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		ret = sub->value();
	else
		ret = defaultvalue;
}

void XMLTree::value(Std::string key, int &ret,
                    int defaultvalue) const {
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		ret = strtol(sub->value().c_str(), 0, 0);
	else
		ret = defaultvalue;
}

void XMLTree::value(Std::string key, bool &ret,
                    bool defaultvalue) const {
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		ret = (to_uppercase(sub->value()) == "YES");
	else
		ret = defaultvalue;
}

void XMLTree::set(Std::string key, Std::string value) {
	tree->xmlassign(key, value);
}

void XMLTree::set(Std::string key, const char *value) {
	tree->xmlassign(key, value);
}

void XMLTree::set(Std::string key, int value) {
	char buf[32];
	snprintf(buf, 32, "%d", value);
	set(key, buf);
}

void XMLTree::set(Std::string key, bool value) {
	if (value)
		set(key, "yes");
	else
		set(key, "no");
}

Std::vector<Std::string> XMLTree::listKeys(Std::string key, bool longformat) {
	Std::vector<Std::string> keys;
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		sub->listkeys(key, keys, longformat);

	return keys;
}

void XMLTree::getSubkeys(KeyTypeList &ktl, Std::string basekey) {
	tree->searchpairs(ktl, basekey, Std::string(), 0);
}

} // End of namespace Nuvie
} // End of namespace Ultima
