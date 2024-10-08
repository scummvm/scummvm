/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#if defined(POSIX)

#include "backends/fs/chroot/chroot-fs.h"

ChRootFilesystemNode::ChRootFilesystemNode(const Common::String &root, POSIXFilesystemNode *node, const Common::String &drive) {
	_root = Common::normalizePath(root, '/');
	_drive = drive;
	_realNode = node;
}

ChRootFilesystemNode::ChRootFilesystemNode(const Common::String &root, const Common::String &path, const Common::String &drive) {
	_root = Common::normalizePath(root, '/');
	_drive = drive;
	_realNode = new POSIXFilesystemNode(addPathComponent(root, path));
}

ChRootFilesystemNode::~ChRootFilesystemNode() {
	delete _realNode;
}

bool ChRootFilesystemNode::exists() const {
	return _realNode->exists();
}

Common::U32String ChRootFilesystemNode::getDisplayName() const {
	return _realNode->getDisplayName();
}

Common::String ChRootFilesystemNode::getName() const {
	return _realNode->getName();
}

Common::String ChRootFilesystemNode::getPath() const {
	Common::String path = _realNode->getPath();
	if (path.size() > _root.size())
		return _drive + Common::String(path.c_str() + _root.size());
	return _drive + "/";
}

bool ChRootFilesystemNode::isDirectory() const {
	return _realNode->isDirectory();
}

bool ChRootFilesystemNode::isReadable() const {
	return _realNode->isReadable();
}

bool ChRootFilesystemNode::isWritable() const {
	// Assume virtual drives are not writable
	if (!_drive.empty())
		return false;
	return _realNode->isWritable();
}

AbstractFSNode *ChRootFilesystemNode::getChild(const Common::String &n) const {
	return new ChRootFilesystemNode(_root, (POSIXFilesystemNode *)_realNode->getChild(n), _drive);
}

bool ChRootFilesystemNode::getChildren(AbstractFSList &list, ListMode mode, bool hidden) const {
	AbstractFSList tmp;
	if (!_realNode->getChildren(tmp, mode, hidden)) {
		return false;
	}

	for (AbstractFSList::iterator i=tmp.begin(); i!=tmp.end(); ++i) {
		list.push_back(new ChRootFilesystemNode(_root, (POSIXFilesystemNode *) *i, _drive));
	}

	return true;
}

AbstractFSNode *ChRootFilesystemNode::getParent() const {
	if (getPath() == _drive + "/")
		return nullptr;
	return new ChRootFilesystemNode(_root, (POSIXFilesystemNode *)_realNode->getParent(), _drive);
}

Common::SeekableReadStream *ChRootFilesystemNode::createReadStream() {
	return _realNode->createReadStream();
}

Common::SeekableWriteStream *ChRootFilesystemNode::createWriteStream(bool atomic) {
	return _realNode->createWriteStream(atomic);
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
