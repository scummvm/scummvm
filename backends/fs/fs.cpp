/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
#include "backends/fs/fs.h"
#include "common/util.h"


static AbstractFilesystemNode *_rootNode = 0;
static int *_rootRefCount = 0;

FilesystemNode::FilesystemNode(AbstractFilesystemNode *realNode) {
	_realNode = realNode;
	_refCount = new int(1);
}

FilesystemNode::FilesystemNode() {
	if (_rootNode == 0) {
		_rootNode = AbstractFilesystemNode::getRoot();
		_rootRefCount = new int(1);
	}
	_realNode = _rootNode;
	_refCount = _rootRefCount;
	++(*_refCount);
}

FilesystemNode::FilesystemNode(const FilesystemNode &node) {
	_realNode = node._realNode;
	_refCount = node._refCount;
	++(*_refCount);
}

FilesystemNode::FilesystemNode(const Common::String &p) {
	_realNode = AbstractFilesystemNode::getNodeForPath(p);
	_refCount = new int(1);
}

FilesystemNode::~FilesystemNode() {
	decRefCount();
}

void FilesystemNode::decRefCount() {
	--(*_refCount);
	if (*_refCount <= 0) {
		delete _refCount;
		delete _realNode;
	}
}

FilesystemNode &FilesystemNode::operator  =(const FilesystemNode &node) {
	++(*node._refCount);

	decRefCount();

	_realNode = node._realNode;
	_refCount = node._refCount;

	return *this;
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

FilesystemNode FilesystemNode::getChild(const String &name) const {
	if (_realNode == 0)
		return *this;

	assert(_realNode->isDirectory());
	AbstractFilesystemNode *node = _realNode->child(name);
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

Common::String FilesystemNode::displayName() const {
	assert(_realNode);
	return _realNode->displayName();
}

bool FilesystemNode::isDirectory() const {
	if (_realNode == 0)
		return false;
	return _realNode->isDirectory();
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
