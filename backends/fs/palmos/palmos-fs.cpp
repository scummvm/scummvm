/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#if defined(__PALMOS_TRAPS__)

#include "../fs.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Implementation of the ScummVM file system API based on PalmOS VFS API.
 */

class PalmOSFilesystemNode : public FilesystemNode {
protected:
	String _displayName;
	bool _isDirectory;
	bool _isValid;
	bool _isPseudoRoot;
	String _path;
	
public:
	PalmOSFilesystemNode();
	PalmOSFilesystemNode(const Char *path);
	PalmOSFilesystemNode(const PalmOSFilesystemNode *node);

	virtual String displayName() const { return _displayName; }
	virtual bool isValid() const { return _isValid; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual String path() const { return _path; }

	virtual FSList *listDir(ListMode) const;
	virtual FilesystemNode *parent() const;
	virtual FilesystemNode *clone() const { return new PalmOSFilesystemNode(this); }

private:
	static void addFile (FSList* list, ListMode mode, const Char *base, FileInfoType* find_data);
};

void PalmOSFilesystemNode::addFile (FSList* list, ListMode mode, const char *base, FileInfoType* find_data) {
	PalmOSFilesystemNode entry;
	bool isDirectory;

	isDirectory = (find_data->attributes & vfsFileAttrDirectory);

	if ((!isDirectory && mode == kListDirectoriesOnly) ||
		(isDirectory && mode == kListFilesOnly))
		return;

	entry._isDirectory = isDirectory;
	entry._displayName = find_data->nameP;
	entry._path = base;
	entry._path += find_data->nameP;
	if (entry._isDirectory)
		entry._path += "/";

	entry._isValid = true;	
	entry._isPseudoRoot = false;
	list->push_back(entry);
}

FilesystemNode *FilesystemNode::getRoot() {
	return new PalmOSFilesystemNode();
}

PalmOSFilesystemNode::PalmOSFilesystemNode() {
	_isDirectory = true;
	_displayName = "Root";
	_isValid = true;
	_path = "/";
	_isPseudoRoot = true;
}

PalmOSFilesystemNode::PalmOSFilesystemNode(const PalmOSFilesystemNode *node) {
	_displayName = node->_displayName;
	_isDirectory = node->_isDirectory;
	_isValid = node->_isValid;
	_isPseudoRoot = node->_isPseudoRoot;
	_path = node->_path;
}

FSList *PalmOSFilesystemNode::listDir(ListMode mode) const {

	FSList *myList = new FSList();
	Err e;
	Char nameP[256];
	FileInfoType desc;
	FileRef handle;
	UInt32 dirIterator = expIteratorStart;

	desc.nameP = nameP;
	desc.nameBufLen = 256;
	e = VFSFileOpen(gVars->volRefNum, _path.c_str(), vfsModeRead, &handle);

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

FilesystemNode *PalmOSFilesystemNode::parent() const {

	PalmOSFilesystemNode *p = new PalmOSFilesystemNode();

	if (!_isPseudoRoot) {
        const char *start = _path.c_str();
        const char *end = lastPathComponent(_path);

		p->_path = String(start, end - start);
		p->_isValid = true;
		p->_isDirectory = true;
		p->_displayName = lastPathComponent(p->_path);
		p->_isPseudoRoot =(p->_path == "/");
	}
	return p;
}

#endif // __PALMOS_TRAPS__
