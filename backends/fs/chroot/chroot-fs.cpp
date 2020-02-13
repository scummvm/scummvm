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

#if defined(POSIX)

#include "backends/fs/chroot/chroot-fs.h"

ChRootFilesystemNode::ChRootFilesystemNode(const Common::String &root, POSIXFilesystemNode *node) {
	_root = Common::normalizePath(root, '/');
	_realNode = node;
}

ChRootFilesystemNode::ChRootFilesystemNode(const Common::String &root, const Common::String &path) {
	_root = Common::normalizePath(root, '/');
	_realNode = new POSIXFilesystemNode(addPathComponent(root, path));
}

ChRootFilesystemNode::~ChRootFilesystemNode() {
	delete _realNode;
}

bool ChRootFilesystemNode::exists() const {
	return _realNode->exists();
}

Common::String ChRootFilesystemNode::getDisplayName() const {
	return getName();
}

Common::String ChRootFilesystemNode::getName() const {
	return _realNode->AbstractFSNode::getDisplayName();
}

Common::String ChRootFilesystemNode::getPath() const {
	Common::String path = _realNode->getPath();
	if (path.size() > _root.size()) {
		return Common::String(path.c_str() + _root.size());
	}
	return Common::String("/");
}

bool ChRootFilesystemNode::isDirectory() const {
	return _realNode->isDirectory();
}

bool ChRootFilesystemNode::isReadable() const {
	return _realNode->isReadable();
}

bool ChRootFilesystemNode::isWritable() const {
	return _realNode->isWritable();
}

AbstractFSNode *ChRootFilesystemNode::getChild(const Common::String &n) const {
	return new ChRootFilesystemNode(_root, (POSIXFilesystemNode *)_realNode->getChild(n));
}

bool ChRootFilesystemNode::getChildren(AbstractFSList &list, ListMode mode, bool hidden) const {
	AbstractFSList tmp;
	if (!_realNode->getChildren(tmp, mode, hidden)) {
		return false;
	}

	for (AbstractFSList::iterator i=tmp.begin(); i!=tmp.end(); ++i) {
		list.push_back(new ChRootFilesystemNode(_root, (POSIXFilesystemNode *) *i));
	}

	return true;
}

AbstractFSNode *ChRootFilesystemNode::getParent() const {
	if (getPath() == "/") return 0;
	return new ChRootFilesystemNode(_root, (POSIXFilesystemNode *)_realNode->getParent());
}

Common::SeekableReadStream *ChRootFilesystemNode::createReadStream() {
	return _realNode->createReadStream();
}

Common::WriteStream *ChRootFilesystemNode::createWriteStream() {
	return _realNode->createWriteStream();
}

bool ChRootFilesystemNode::createDirectory() {
	return _realNode->createDirectory();
}

Common::String ChRootFilesystemNode::addPathComponent(const Common::String &path, const Common::String &component) {
	const char sep = '/';
	if (path.lastChar() == sep && component.firstChar() == sep) {
		return Common::String::format("%s%s", path.c_str(), component.c_str() + 1);
	}

	if (path.lastChar() == sep || component.firstChar() == sep) {
		return Common::String::format("%s%s", path.c_str(), component.c_str());
	}

	return Common::String::format("%s%c%s", path.c_str(), sep, component.c_str());
}

#endif
