/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002 Ph0x - GP32 Backend
 * Copyright (C) 2003/2004 DJWillis - GP32 Backend
 * Copyright (C) 2005 Won Star - GP32 Backend
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"

#include "backends/fs/fs.h"

class GP32FilesystemNode : public AbstractFilesystemNode {
protected:
	String _displayName;
	bool _isDirectory;
	bool _isRoot;
	String _path;

public:
	GP32FilesystemNode(void);
	GP32FilesystemNode(const GP32FilesystemNode *node);
	GP32FilesystemNode(const String &path);

	virtual String displayName() const { return _displayName; }
	virtual bool isValid() const { return true; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual String path() const { return _path; }

	virtual FSList listDir(ListMode) const;
	virtual AbstractFilesystemNode *parent() const;
	virtual AbstractFilesystemNode *clone() const { return new GP32FilesystemNode(this); }
};

AbstractFilesystemNode *FilesystemNode::getRoot(void) {
	return new GP32FilesystemNode();
}

AbstractFilesystemNode *FilesystemNode::getNodeForPath(const String &path) {
	return new GP32FilesystemNode(path);
}

GP32FilesystemNode::GP32FilesystemNode(void) {
	_isDirectory = true;
	_isRoot = true;
	_displayName = "GP32 Root";
	_path = "gp:\\";
}

GP32FilesystemNode::GP32FilesystemNode(const String &path) {
	_path = path;
	const char *dsplName = NULL, *pos = path.c_str();
	while (*pos)
		if (*pos++ == '\\')
			dsplName = pos;
			
	BP("path name: %s", path.c_str());

	if (strcmp(path.c_str(), "gp:\\") == 0) {
		_isRoot = true;
		_displayName = "GP32 Root";
	} else {
		_displayName = String(dsplName);
	}
	_isDirectory = true;
}

GP32FilesystemNode::GP32FilesystemNode(const GP32FilesystemNode *node) {
	_displayName = node->_displayName;
	_isDirectory = node->_isDirectory;
	_path = node->_path;
	_isRoot = node->_isRoot;
}

FSList GP32FilesystemNode::listDir(ListMode mode) const {
	assert(_isDirectory);

	GPDIRENTRY dirEntry;
	uint32 read;
	FSList myList;

	if (mode == AbstractFilesystemNode::kListAll)
		LP("listDir(kListAll)");
	else
		LP("listDir(kListDirectoriesOnly)");

	int startIdx = 0; // current file
	String listDir(_path);
	//listDir += "/";
	while (GpDirEnumList(listDir.c_str(), startIdx++, 1, &dirEntry, &read)  == SM_OK) {
		if (dirEntry.name[0] == '.')
			continue;
		GP32FilesystemNode entry;
		entry._displayName = dirEntry.name;
		entry._path = _path;
		entry._path += dirEntry.name;

		GPFILEATTR attr;
		String fileName(entry._path);
		GpFileAttr(fileName.c_str(), &attr);
		entry._isDirectory = attr.attr & (1<<4);

		// Honor the chosen mode
		if ((mode == kListFilesOnly && entry._isDirectory) ||
			(mode == kListDirectoriesOnly && !entry._isDirectory))
			continue;

		if (entry._isDirectory)
			entry._path += "\\";
		myList.push_back(wrap(new GP32FilesystemNode(&entry)));
	}

	BP("Dir... %s", listDir.c_str());

	return myList;
}
/*
AbstractFilesystemNode *GP32FilesystemNode::parent() const {
	if (_isRoot)
		return new GP32FilesystemNode(this);

	GP32FilesystemNode *p = new GP32FilesystemNode();

	const char *slash = NULL;
	const char *cnt = _path.c_str();

	while (*cnt) {
		if (*cnt == '\\')
			slash = cnt;
		cnt++;
	}

	p->_path = String(_path.c_str(), slash - _path.c_str());
	p->_isDirectory = true;
	p->_displayName = slash + 1;
	return p;
}
*/

const char *lastPathComponent(const Common::String &str) {
	const char *start = str.c_str();
	const char *cur = start + str.size() - 2;

	while (cur > start && *cur != '\\') { //ph0x
		--cur;
	}

	return cur+1;
}

AbstractFilesystemNode *GP32FilesystemNode::parent() const {

	GP32FilesystemNode *p = new GP32FilesystemNode();

	// Root node is its own parent. Still we can't just return this
	// as the GUI code will call delete on the old node.
	if (_path != "gp:\\") {
		const char *start = _path.c_str();
		const char *end = lastPathComponent(_path);

		p->_path = Common::String(start, end - start);
		p->_displayName = lastPathComponent(p->_path);
		p->_isRoot = true;
	} else {
		p->_isRoot = false;
		p->_path = _path;
		p->_displayName = _displayName;
	}
	p->_isDirectory = true;
	return p;
}
