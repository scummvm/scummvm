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

#include "common/stdafx.h"

#include "backends/fs/abstract-fs.h"
#include "common/util.h"


FilesystemNode::FilesystemNode(AbstractFilesystemNode *realNode) {
	_realNode = realNode;
	_refCount = new int(1);
}

FilesystemNode::FilesystemNode() {
	_realNode = 0;
	_refCount = 0;
}

FilesystemNode::FilesystemNode(const FilesystemNode &node) {
	_realNode = node._realNode;
	_refCount = node._refCount;
	if (_refCount)
		++(*_refCount);
}

FilesystemNode::FilesystemNode(const Common::String &p) {
	if (p.empty() || p == ".")
		_realNode = AbstractFilesystemNode::getCurrentDirectory();
	else
		_realNode = AbstractFilesystemNode::getNodeForPath(p);
	_refCount = new int(1);
}

FilesystemNode::~FilesystemNode() {
	decRefCount();
}

void FilesystemNode::decRefCount() {
	if (_refCount) {
		assert(*_refCount > 0);
		--(*_refCount);
		if (*_refCount == 0) {
			delete _refCount;
			delete _realNode;
		}
	}
}

FilesystemNode &FilesystemNode::operator  =(const FilesystemNode &node) {
	if (node._refCount)
		++(*node._refCount);

	decRefCount();

	_realNode = node._realNode;
	_refCount = node._refCount;

	return *this;
}

bool FilesystemNode::isValid() const {
	if (_realNode == 0)
		return false;
	return _realNode->isValid();
}

FilesystemNode FilesystemNode::getParent() const {
	if (_realNode == 0)
		return *this;

	AbstractFilesystemNode *node = _realNode->parent();
	if (node == 0) {
		return *this;
	} else {
		return FilesystemNode(node);
	}
}

FilesystemNode FilesystemNode::getChild(const Common::String &n) const {
	if (_realNode == 0)
		return *this;

	assert(_realNode->isDirectory());
	AbstractFilesystemNode *node = _realNode->child(n);
	return FilesystemNode(node);
}

bool FilesystemNode::listDir(FSList &fslist, ListMode mode) const {
	if (!_realNode || !_realNode->isDirectory())
		return false;

	AbstractFSList tmp;
	
	if (!_realNode->listDir(tmp, mode))
		return false;
	
	fslist.clear();
	for (AbstractFSList::iterator i = tmp.begin(); i != tmp.end(); ++i) {
		fslist.push_back(FilesystemNode(*i));
	}
	
	return true;
}

bool FilesystemNode::isDirectory() const {
	if (_realNode == 0)
		return false;
	return _realNode->isDirectory();
}

Common::String FilesystemNode::displayName() const {
	assert(_realNode);
	return _realNode->displayName();
}

Common::String FilesystemNode::name() const {
	assert(_realNode);
	return _realNode->name();
}

Common::String FilesystemNode::path() const {
	assert(_realNode);
	return _realNode->path();
}


bool FilesystemNode::operator< (const FilesystemNode& node) const
{
	if (isDirectory() && !node.isDirectory())
		return true;
	if (!isDirectory() && node.isDirectory())
		return false;
	return scumm_stricmp(displayName().c_str(), node.displayName().c_str()) < 0;
}
