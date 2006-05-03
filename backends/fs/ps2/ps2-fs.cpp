/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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
#include <kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include <cdvd_rpc.h>

#define MAX_LIST_ENTRIES 64

class Ps2FilesystemNode : public AbstractFilesystemNode {
protected:
	String _displayName;
	bool _isDirectory;
	bool _isRoot;
	String _path;

public:
	Ps2FilesystemNode(void);
	Ps2FilesystemNode(const String &path);

	virtual String displayName() const { return _displayName; }
	virtual bool isValid() const { return true; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual String path() const { return _path; }

	virtual FSList listDir(ListMode) const;
	virtual AbstractFilesystemNode *parent() const;
	virtual AbstractFilesystemNode *child(const String &name) const;

	virtual AbstractFilesystemNode *clone() const { return new Ps2FilesystemNode(this); }
};

AbstractFilesystemNode *AbstractFilesystemNode::getRoot(void) {
	return new Ps2FilesystemNode();
}

AbstractFilesystemNode *AbstractFilesystemNode::getNodeForPath(const String &path) {
	return new Ps2FilesystemNode(path);
}

Ps2FilesystemNode::Ps2FilesystemNode(void) {
	_isDirectory = true;
	_isRoot = true;
	_displayName = "CD Root";
	_path = "cdfs:";
}

Ps2FilesystemNode::Ps2FilesystemNode(const String &path) {
	if (strcmp(path.c_str(), "cdfs:") == 0)
		_isRoot = true;
	_path = path;
	const char *dsplName = NULL, *pos = path.c_str();
	while (*pos)
		if (*pos++ == '/')
			dsplName = pos;
	_displayName = String(dsplName);
	_isDirectory = true;
}

FSList Ps2FilesystemNode::listDir(ListMode mode) const {
	assert(_isDirectory);

	FSList myList;

	struct TocEntry tocEntries[MAX_LIST_ENTRIES];
	int files;
	char listDir[512];
	sprintf(listDir, "%s/", _path.c_str() + 5);

	switch(mode) {
		case FilesystemNode::kListFilesOnly:
			files = CDVD_GetDir(listDir, NULL, CDVD_GET_FILES_ONLY, tocEntries, MAX_LIST_ENTRIES, NULL);
			break;
		case FilesystemNode::kListDirectoriesOnly:
			files = CDVD_GetDir(listDir, NULL, CDVD_GET_DIRS_ONLY, tocEntries, MAX_LIST_ENTRIES, NULL);
			break;
		default:
			files = CDVD_GetDir(listDir, NULL, CDVD_GET_FILES_AND_DIRS, tocEntries, MAX_LIST_ENTRIES, NULL);
			break;
	}

	Ps2FilesystemNode dirEntry;
	for (int fCnt = 0; fCnt < files; fCnt++) {
		if (tocEntries[fCnt].filename[0] != '.') { // skip .. directory
			dirEntry._isDirectory = (bool)(tocEntries[fCnt].fileProperties & 2);
			dirEntry._isRoot = false;

			dirEntry._path = _path;
			dirEntry._path += "/";
			dirEntry._path += tocEntries[fCnt].filename;

			dirEntry._displayName = tocEntries[fCnt].filename;
			myList.push_back(wrap(new Ps2FilesystemNode(dirEntry)));
		}
	}
	return myList;
}

AbstractFilesystemNode *Ps2FilesystemNode::parent() const {
	if (_isRoot)
		return new Ps2FilesystemNode(this);

	Ps2FilesystemNode *p = new Ps2FilesystemNode();

	const char *slash = NULL;
	const char *cnt = _path.c_str();

	while (*cnt) {
		if (*cnt == '/')
			slash = cnt;
		cnt++;
	}

	p->_path = String(_path.c_str(), slash - _path.c_str());
	p->_isDirectory = true;
	p->_displayName = slash + 1;
	return p;
}


AbstractFilesystemNode *Ps2FilesystemNode::child(const String &name) const {
	TODO
}
