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

XMLTree::XMLTree() : _tree(nullptr), _isFile(false),
		_readOnly(false) {
}

XMLTree::XMLTree(const Common::String &fname) : _tree(nullptr), _isFile(true),
	  _readOnly(false) {
	readConfigFile(fname);
}

XMLTree::XMLTree(Common::SeekableReadStream *stream)
		: _tree(nullptr), _isFile(true), _readOnly(false) {
	readConfigStream(stream);
}

XMLTree::~XMLTree() {
	delete _tree;
}

void XMLTree::clear() {
	delete _tree;
	_tree = nullptr;
	_isFile = false;
	_readOnly = false;
}

bool XMLTree::readConfigFile(const Common::String &fname) {
	Common::File f;
	_filename = fname;

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
	// Read in the stream contents
	char *buf = new char[stream->size() + 1];
	stream->read(buf, stream->size());
	buf[stream->size()] = '\0';
	Common::String text(buf, buf + stream->size());

	if (!readConfigString(buf))
		return false;

	delete[] buf;
	_isFile = true;		// readConfigString sets _isFile = false
	return true;
}

bool XMLTree::readConfigString(const Common::String &s) {
	_tree = _tree->xmlParseDoc(this, s);

	_isFile = false;
	_filename.clear();

	return _tree != nullptr;
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
	return _tree && k == _tree->id();
}

void XMLTree::value(const Common::String &key, Common::String &ret,
                    const char *defaultvalue) const {
	const XMLNode *sub = _tree->subtree(key);
	if (sub)
		ret = sub->text();
	else
		ret = defaultvalue;
}

void XMLTree::value(const Common::String &key, int &ret,
                    int defaultvalue) const {
	const XMLNode *sub = _tree->subtree(key);
	if (sub)
		ret = strtol(sub->text().c_str(), 0, 0);
	else
		ret = defaultvalue;
}

void XMLTree::value(const Common::String &key, bool &ret,
                    bool defaultvalue) const {
	const XMLNode *sub = _tree->subtree(key);
	if (sub)
		ret = sub->text().equalsIgnoreCase("YES");
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
