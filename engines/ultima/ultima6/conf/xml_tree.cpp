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
#include "ultima/ultima6/conf/xml_tree.h"
#include "ultima/ultima6/conf/xml_node.h"
#include "ultima/shared/std/string.h"
#include "common/algorithm.h"
#include "common/file.h"

namespace Ultima {
namespace Ultima6 {

std::string to_uppercase(std::string s) {
	std::string str = s;
	std::string::iterator X;
	for (X = str.begin(); X != str.end(); ++X) {
#if (defined(BEOS) || defined(OPENBSD) || defined(CYGWIN) || defined(__MORPHOS__) || defined(_MSC_VER))
		if ((*X >= 'a') && (*X <= 'z')) *X -= 32;
#else
		*X = std::toupper(*X);
#endif
	}
	return str;
}

XMLTree::XMLTree()
	: tree(new XMLNode("config")), root("config"), is_file(false),
	  readonly(false) {

}

XMLTree::XMLTree(std::string fname, std::string root_)
	: tree(new XMLNode(root_)), root(root_), is_file(true),
	  readonly(false) {
	readConfigFile(fname);
}

XMLTree::~XMLTree() {
	delete tree;
}

void XMLTree::clear(std::string root_) {
	delete tree;
	tree = new XMLNode(root_);
	root = root_;
	is_file = false;
	readonly = false;
}

bool XMLTree::readConfigFile(std::string fname) {
	Common::File f;

//	if (!FileSystem::get_instance()->rawopen(f, fname, true))
//		return false;
// mode = std::ios::in;

	if (f.open(fname)) {
		DEBUG(0, LEVEL_CRITICAL, "Error opening config file");
		return false;
	}

	std::string sbuf, line;
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

bool XMLTree::readConfigString(std::string s) {
	is_file = false;

	std::string sbuf(s);
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

std::string XMLTree::dump() {
	return tree->dump();
}

void XMLTree::write() {
	if (!is_file || readonly)
		return;

	Common::DumpFile df;

	if (df.open(filename)) {
		std::string content = dump();
		df.write(content.c_str(), content.size());
		df.close();
	}
}

bool XMLTree::hasNode(std::string key) const {
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		return true;
	else
		return false;
}

bool XMLTree::checkRoot(std::string key) const {
	std::string k = key.substr(0, key.find('/'));
	return (k == root);
}

void XMLTree::value(std::string key, std::string &ret,
                    const char *defaultvalue) const {
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		ret = sub->value();
	else
		ret = defaultvalue;
}

void XMLTree::value(std::string key, int &ret,
                    int defaultvalue) const {
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		ret = strtol(sub->value().c_str(), 0, 0);
	else
		ret = defaultvalue;
}

void XMLTree::value(std::string key, bool &ret,
                    bool defaultvalue) const {
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		ret = (to_uppercase(sub->value()) == "YES");
	else
		ret = defaultvalue;
}

void XMLTree::set(std::string key, std::string value) {
	tree->xmlassign(key, value);
}

void XMLTree::set(std::string key, const char *value) {
	tree->xmlassign(key, value);
}

void XMLTree::set(std::string key, int value) {
	char buf[32];
	snprintf(buf, 32, "%d", value);
	set(key, buf);
}

void XMLTree::set(std::string key, bool value) {
	if (value)
		set(key, "yes");
	else
		set(key, "no");
}

std::vector<std::string> XMLTree::listKeys(std::string key, bool longformat) {
	std::vector<std::string> keys;
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		sub->listkeys(key, keys, longformat);

	return keys;
}

void XMLTree::getSubkeys(KeyTypeList &ktl, std::string basekey) {
	tree->searchpairs(ktl, basekey, std::string(), 0);
}

} // End of namespace Ultima6
} // End of namespace Ultima
