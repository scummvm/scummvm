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

#include "backends/fs/fs.h"
#include "common/util.h"


static AbstractFilesystemNode *_rootNode = 0;
static int *_rootRefCount = 0;

FilesystemNode AbstractFilesystemNode::wrap(AbstractFilesystemNode *node) {
	FilesystemNode wrapper(node);
	return wrapper;
}

FilesystemNode::FilesystemNode(AbstractFilesystemNode *realNode) {
	_realNode = realNode;
	_refCount = new int(1);
}

FilesystemNode::FilesystemNode() {
	if (_rootNode == 0) {
		_rootNode = getRoot();
		_rootRefCount = new int(1);
	}
	_realNode = _rootNode;
	_refCount = _rootRefCount;
	++(*_refCount);
}

FilesystemNode::FilesystemNode(const FilesystemNode &node)
	: AbstractFilesystemNode() {
	_realNode = node._realNode;
	_refCount = node._refCount;
	++(*_refCount);
}

FilesystemNode::FilesystemNode(const Common::String &p) {
	_realNode = getNodeForPath(p);
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
		return AbstractFilesystemNode::wrap(node);
	}
}

FilesystemNode FilesystemNode::getChild(const String &name) const {
	if (_realNode == 0)
		return *this;

	assert(_realNode->isDirectory());
	AbstractFilesystemNode *node = _realNode->child(name);
	return AbstractFilesystemNode::wrap(node);
}

FSList FilesystemNode::listDir(ListMode mode) const {
	assert(_realNode);
	assert(_realNode->isDirectory());
	return _realNode->listDir(mode);
}

Common::String FilesystemNode::displayName() const {
	assert(_realNode);
	return _realNode->displayName();
}

bool FilesystemNode::isValid() const {
	if (_realNode == 0)
		return false;
	return _realNode->isValid();
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
