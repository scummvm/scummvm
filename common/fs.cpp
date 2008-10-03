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
 * $URL$
 * $Id$
 */

#include "common/util.h"
#include "common/system.h"
#include "backends/fs/abstract-fs.h"
#include "backends/fs/fs-factory.h"

namespace Common {

FSNode::FSNode() {
}

FSNode::FSNode(AbstractFSNode *realNode) 
	: _realNode(realNode) {
}

FSNode::FSNode(const Common::String &p) {
	FilesystemFactory *factory = g_system->getFilesystemFactory();
	AbstractFSNode *tmp = 0;
	
	if (p.empty() || p == ".")
		tmp = factory->makeCurrentDirectoryFileNode();
	else
		tmp = factory->makeFileNodePath(p);
	_realNode = Common::SharedPtr<AbstractFSNode>(tmp);
}

bool FSNode::operator<(const FSNode& node) const {
	if (isDirectory() != node.isDirectory())
		return isDirectory();

	return getDisplayName().compareToIgnoreCase(node.getDisplayName()) < 0;
}

bool FSNode::exists() const {
	if (_realNode == 0)
		return false;

	return _realNode->exists();
}

FSNode FSNode::getChild(const Common::String &n) const {
	// If this node is invalid or not a directory, return an invalid node
	if (_realNode == 0 || !_realNode->isDirectory())
		return FSNode();

	AbstractFSNode *node = _realNode->getChild(n);
	return FSNode(node);
}

bool FSNode::getChildren(FSList &fslist, ListMode mode, bool hidden) const {
	if (!_realNode || !_realNode->isDirectory())
		return false;

	AbstractFSList tmp;

	if (!_realNode->getChildren(tmp, mode, hidden))
		return false;

	fslist.clear();
	for (AbstractFSList::iterator i = tmp.begin(); i != tmp.end(); ++i) {
		fslist.push_back(FSNode(*i));
	}

	return true;
}

Common::String FSNode::getDisplayName() const {
	assert(_realNode);
	return _realNode->getDisplayName();
}

Common::String FSNode::getName() const {
	assert(_realNode);
	return _realNode->getName();
}

FSNode FSNode::getParent() const {
	if (_realNode == 0)
		return *this;

	AbstractFSNode *node = _realNode->getParent();
	if (node == 0) {
		return *this;
	} else {
		return FSNode(node);
	}
}

Common::String FSNode::getPath() const {
	assert(_realNode);
	return _realNode->getPath();
}

bool FSNode::isDirectory() const {
	if (_realNode == 0)
		return false;

	return _realNode->isDirectory();
}

bool FSNode::isReadable() const {
	if (_realNode == 0)
		return false;

	return _realNode->isReadable();
}

bool FSNode::isWritable() const {
	if (_realNode == 0)
		return false;

	return _realNode->isWritable();
}

bool FSNode::lookupFile(FSList &results, const Common::String &p, bool hidden, bool exhaustive, int depth) const {
	if (!isDirectory())
		return false;

	FSList children;
	FSList subdirs;
	Common::String pattern = p;

	pattern.toUppercase();

	// First match all files on this level
	getChildren(children, FSNode::kListAll, hidden);
	for (FSList::iterator entry = children.begin(); entry != children.end(); ++entry) {
		if (entry->isDirectory()) {
			if (depth != 0)
				subdirs.push_back(*entry);
		} else {
			Common::String filename = entry->getName();
			filename.toUppercase();
			if (filename.matchString(pattern)) {
				results.push_back(*entry);

				if (!exhaustive)
					return true;	// Abort on first match if no exhaustive search was requested
			}
		}
	}

	// Now scan all subdirs
	for (FSList::iterator child = subdirs.begin(); child != subdirs.end(); ++child) {
		child->lookupFile(results, pattern, hidden, exhaustive, depth - 1);
		if (!exhaustive && !results.empty())
			return true;	// Abort on first match if no exhaustive search was requested
	}

	return !results.empty();
}

Common::SeekableReadStream *FSNode::openForReading() const {
	if (_realNode == 0)
		return 0;

	if (!_realNode->exists()) {
		warning("FSNode::openForReading: FSNode does not exist");
		return false;
	} else if (_realNode->isDirectory()) {
		warning("FSNode::openForReading: FSNode is a directory");
		return false;
	}

	return _realNode->openForReading();
}

Common::WriteStream *FSNode::openForWriting() const {
	if (_realNode == 0)
		return 0;

	if (_realNode->isDirectory()) {
		warning("FSNode::openForWriting: FSNode is a directory");
		return 0;
	}

	return _realNode->openForWriting();
}

}	// End of namespace Common
