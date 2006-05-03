/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM Team
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

#if defined(PALMOS_MODE)

#include "common/stdafx.h"
#include "fs/fs.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Implementation of the ScummVM file system API based on PalmOS VFS API.
 */

class PalmOSFilesystemNode : public AbstractFilesystemNode {
protected:
	String _displayName;
	bool _isDirectory;
	bool _isValid;
	bool _isPseudoRoot;
	String _path;
	
public:
	PalmOSFilesystemNode();
	PalmOSFilesystemNode(const PalmOSFilesystemNode &node);
	PalmOSFilesystemNode(const String &path);

	virtual String displayName() const { return _displayName; }
	virtual bool isValid() const { return _isValid; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual String path() const { return _path; }

	virtual AbstractFSList listDir(ListMode) const;
	virtual AbstractFilesystemNode *parent() const;
	virtual AbstractFilesystemNode *child(const String &name) const;

private:
	static void addFile (AbstractFSList &list, ListMode mode, const Char *base, FileInfoType* find_data);
};

void PalmOSFilesystemNode::addFile(AbstractFSList &list, ListMode mode, const char *base, FileInfoType* find_data) {
	PalmOSFilesystemNode entry;
	bool isDirectory;

	isDirectory = (find_data->attributes & vfsFileAttrDirectory);

	if ((!isDirectory && mode == FilesystemNode::kListDirectoriesOnly) ||
		(isDirectory && mode == FilesystemNode::kListFilesOnly))
		return;

	entry._isDirectory = isDirectory;
	entry._displayName = find_data->nameP;
	entry._path = base;
	entry._path += find_data->nameP;
	if (entry._isDirectory)
		entry._path += "/";

	entry._isValid = true;	
	entry._isPseudoRoot = false;
	list.push_back(new PalmOSFilesystemNode(entry));
}

AbstractFilesystemNode *AbstractFilesystemNode::getRoot() {
	return new PalmOSFilesystemNode();
}

AbstractFilesystemNode *AbstractFilesystemNode::getNodeForPath(const String &path) {
	return new PalmOSFilesystemNode(path);
}


PalmOSFilesystemNode::PalmOSFilesystemNode() {
	_isDirectory = true;
	_displayName = "Root";
	_isValid = true;
	_path = "/";
	_isPseudoRoot = true;
}

PalmOSFilesystemNode::PalmOSFilesystemNode(const String &path) {
	if (path.size() == 0)
		_isPseudoRoot = true;
	_path = path;
	const char *dsplName = NULL, *pos = path.c_str();
	while (*pos)
		if (*pos++ == '/')
			dsplName = pos;
	_displayName = String(dsplName);
	_isValid = true;
	_isDirectory = true;
}

PalmOSFilesystemNode::PalmOSFilesystemNode(const PalmOSFilesystemNode &node) {
	_displayName = node._displayName;
	_isDirectory = node._isDirectory;
	_isValid = node._isValid;
	_isPseudoRoot = node._isPseudoRoot;
	_path = node._path;
}

AbstractFSList PalmOSFilesystemNode::listDir(ListMode mode) const {
	AbstractFSList myList;
	Err e;
	Char nameP[256];
	FileInfoType desc;
	FileRef handle;
	UInt32 dirIterator = expIteratorStart;

	desc.nameP = nameP;
	desc.nameBufLen = 256;
	e = VFSFileOpen(gVars->VFS.volRefNum, _path.c_str(), vfsModeRead, &handle);

	if (e)
		return myList;
	
	while(dirIterator != expIteratorStop) {
		e = VFSDirEntryEnumerate(handle, &dirIterator, &desc);
		if (!e) {
			addFile(myList, mode, _path.c_str(), &desc);
		}
	}

	VFSFileClose(handle);

	return myList;
}

const char *lastPathComponent(const Common::String &str) {
	const char *start = str.c_str();
	const char *cur = start + str.size() - 2;

	while (cur > start && *cur != '/') {
		--cur;
	}

	return cur+1;
}

AbstractFilesystemNode *PalmOSFilesystemNode::parent() const {
	PalmOSFilesystemNode *p = 0;
	
	if (!_isPseudoRoot) {
		const char *start = _path.c_str();
		const char *end = lastPathComponent(_path);
	
		p = new PalmOSFilesystemNode();
		p->_path = String(start, end - start);
		p->_isValid = true;
		p->_isDirectory = true;
		p->_displayName = lastPathComponent(p->_path);
		p->_isPseudoRoot =(p->_path == "/");
	}
	return p;
}


AbstractFilesystemNode *PalmOSFilesystemNode::child(const String &name) const {
	TODO
}

#endif // PALMOS_MODE
