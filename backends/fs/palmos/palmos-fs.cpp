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
#include "backends/fs/abstract-fs.h"
#include "backends/fs/fs.h"

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
	PalmOSFilesystemNode(const String &p);

	virtual String displayName() const { return _displayName; }
	virtual bool isValid() const { return _isValid; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual String path() const { return _path; }

	virtual bool listDir(AbstractFSList &list, ListMode mode) const;
	virtual AbstractFilesystemNode *parent() const;
	virtual AbstractFilesystemNode *child(const String &name) const;

private:
	static void addFile (AbstractFSList &list, ListMode mode, const Char *base, FileInfoType* find_data);
};

static const char *lastPathComponent(const Common::String &str) {
	const char *start = str.c_str();
	const char *cur = start + str.size() - 2;

	while (cur > start && *cur != '/')
		--cur;

	return cur + 1;
}

void PalmOSFilesystemNode::addFile(AbstractFSList &list, ListMode mode, const char *base, FileInfoType* find_data) {
	PalmOSFilesystemNode entry;
	bool isDir;

	isDir = (find_data->attributes & vfsFileAttrDirectory);

	if ((!isDir && mode == FilesystemNode::kListDirectoriesOnly) ||
		(isDir && mode == FilesystemNode::kListFilesOnly))
		return;

	entry._isDirectory = isDir;
	entry._displayName = find_data->nameP;
	entry._path = base;
	entry._path += find_data->nameP;

	if (entry._isDirectory)
		entry._path += "/";

	entry._isValid = true;	
	entry._isPseudoRoot = false;

	list.push_back(new PalmOSFilesystemNode(entry));
}

AbstractFilesystemNode *AbstractFilesystemNode::getCurrentDirectory() {
	return AbstractFilesystemNode::getRoot();
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
	_isPseudoRoot = false;
}

PalmOSFilesystemNode::PalmOSFilesystemNode(const String &p) {
	_path = p;
	_displayName = lastPathComponent(p);

	UInt32 attr;
	FileRef handle;
	Err e = VFSFileOpen(gVars->VFS.volRefNum, _path.c_str(), vfsModeRead, &handle);
	if (!e) {
		e = VFSFileGetAttributes(handle, &attr);
		VFSFileClose(handle);
	}

	if (e) {
		_isValid = false;
		_isDirectory = false;

	} else {
		_isValid = true;
		_isDirectory = (attr & vfsFileAttrDirectory);
	}
	_isPseudoRoot = false;
}

bool PalmOSFilesystemNode::listDir(AbstractFSList &myList, ListMode mode) const {
	Err e;
	Char nameP[256];
	FileInfoType desc;
	FileRef handle;
	UInt32 dirIterator = expIteratorStart;

	desc.nameP = nameP;
	desc.nameBufLen = 256;
	e = VFSFileOpen(gVars->VFS.volRefNum, _path.c_str(), vfsModeRead, &handle);

	if (e)
		return false;
	
	while(dirIterator != expIteratorStop) {
		e = VFSDirEntryEnumerate(handle, &dirIterator, &desc);
		if (!e) {
			addFile(myList, mode, _path.c_str(), &desc);
		}
	}

	VFSFileClose(handle);

	return true;
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
	assert(_isDirectory);
	String newPath(_path);

	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += name;

	FileRef handle;
	UInt32 attr;
	Err e = VFSFileOpen(gVars->VFS.volRefNum, newPath.c_str(), vfsModeRead, &handle);
	if (e)
		return 0;
	
	e = VFSFileGetAttributes(handle, &attr);
	VFSFileClose(handle);

	if (e || !(attr & vfsFileAttrDirectory))
		return 0;

	PalmOSFilesystemNode *p = new PalmOSFilesystemNode(newPath);
	return p;
}

#endif // PALMOS_MODE
