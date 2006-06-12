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
 * $URL$
 * $Id$
 *
 */

#include "stdafx.h"

#include "backends/fs/abstract-fs.h"
#include "backends/fs/fs.h"

class GP32FilesystemNode : public AbstractFilesystemNode {
protected:
	String _displayName;
	bool _isDirectory;
	bool _isRoot;
	String _path;

public:
	GP32FilesystemNode();
	GP32FilesystemNode(const String &path);

	virtual String displayName() const { return _displayName; }
	virtual bool isValid() const { return true; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual String path() const { return _path; }

	virtual bool listDir(AbstractFSList &list, ListMode mode) const;
	virtual AbstractFilesystemNode *parent() const;
	virtual AbstractFilesystemNode *child(const String &name) const;
};

#define MAX_PATH_SIZE 256

const char gpRootPath[] = "gp:\\";
//char gpCurrentPath[MAX_PATH_SIZE] = "gp:\\";		// must end with '\'

int gpMakePath(const char *path, char *convPath) {
	// copy root or current directory
	const char *p;
	if ((*path == '/') || (*path == '\\')) {
		path++;
		p = gpRootPath;
		while (*p)
			*convPath++ = *p++;
	}// else
	//	p = gpCurrentPath;

	//while (*p)
	//	*convPath++ = *p++;

	// add filenames/directories. remove "." & "..", replace "/" with "\"
	do {
		switch (*path) {
		case 0:
		case '/':
		case '\\':
			if (*(convPath - 1) == '\\') {
				// already ends with '\'
			} else if ((*(convPath - 2) == '\\') && (*(convPath - 1) == '.')) {
				convPath--;	// remove '.' and end with '\'
			} else if ((*(convPath - 3) == '\\') && (*(convPath - 2) == '.') && (*(convPath - 1) == '.')) {
				convPath -= 3;	// remove "\.."
				if (*(convPath - 1) == ':')
					*convPath++ = '\\';	// "gp:" -> "gp:\"
				else
					while (*(convPath - 1) != '\\')
						convPath--;	// remove one directory and end with '\'
			} else {
				*convPath++ = '\\';	// just add '\'
			}
			break;

		default:
			*convPath++ = *path;
			break;
		}
	} while (*path++);

	*convPath = '\\';

	//  *--convPath = 0;	// remove last '\' and null-terminate
	*convPath = 0;	// remove last '\' and null-terminate

	return 0;
}

AbstractFilesystemNode *AbstractFilesystemNode::getCurrentDirectory() {
	return AbstractFilesystemNode::getRoot();
}

AbstractFilesystemNode *AbstractFilesystemNode::getRoot() {
	return new GP32FilesystemNode();
}

AbstractFilesystemNode *AbstractFilesystemNode::getNodeForPath(const String &path) {
	return new GP32FilesystemNode(path);
}

GP32FilesystemNode::GP32FilesystemNode() {
	_isDirectory = true;
	_isRoot = true;
	_displayName = "GP32 Root";
	_path = "gp:\\";
}

GP32FilesystemNode::GP32FilesystemNode(const String &path) {
	const char *dsplName = NULL, *pos = NULL;
	char convPath[256];

	gpMakePath(path.c_str(), convPath);

	_path = convPath;

	pos = convPath;
	while (*pos)
		if (*pos++ == '\\')
			dsplName = pos;

	BP("FS: path name: %s", path.c_str());

	if (strcmp(path.c_str(), "gp:\\") == 0) {
		_isRoot = true;
		_displayName = "GP32 Root";
	} else {
		_displayName = String(dsplName);
	}
	_isDirectory = true;
}

bool GP32FilesystemNode::listDir(AbstractFSList &myList, ListMode mode) const {
	assert(_isDirectory);

	GPDIRENTRY dirEntry;
	GPFILEATTR attr;

	GP32FilesystemNode entry;

	uint32 read;

	if (mode == FilesystemNode::kListAll)
		LP("listDir(kListAll)");
	else
		LP("listDir(kListDirectoriesOnly)");

	int startIdx = 0; // current file
	String listDir(_path);
	//listDir += "/";
	while (GpDirEnumList(listDir.c_str(), startIdx++, 1, &dirEntry, &read)  == SM_OK) {
		if (dirEntry.name[0] == '.')
			continue;
		entry._displayName = dirEntry.name;
		entry._path = _path;
		entry._path += dirEntry.name;

		GpFileAttr(entry._path.c_str(), &attr);
		entry._isDirectory = attr.attr & (1 << 4);

		// Honor the chosen mode
		if ((mode == FilesystemNode::kListFilesOnly && entry._isDirectory) ||
			(mode == FilesystemNode::kListDirectoriesOnly && !entry._isDirectory))
			continue;

		if (entry._isDirectory)
			entry._path += "\\";
		myList.push_back(new GP32FilesystemNode(entry));
	}

	BP("Dir... %s", listDir.c_str());

	return true;
}

const char *lastPathComponent(const Common::String &str) {
	const char *start = str.c_str();
	const char *cur = start + str.size() - 2;

	while (cur > start && *cur != '\\') {
		--cur;
	}

	return cur + 1;
}

AbstractFilesystemNode *GP32FilesystemNode::parent() const {
	if(_isRoot)
		return 0;

	GP32FilesystemNode *p = new GP32FilesystemNode();
	if (_path.size() > 4) {
		const char *start = _path.c_str();
		const char *end = lastPathComponent(_path);

		p->_path = String(start, end - start);
		p->_isDirectory = true;
		p->_displayName = lastPathComponent(p->_path);
		p->_isRoot = false;
		
		GPDEBUG("%s", p->_path.c_str());
	}

	return p;
}

AbstractFilesystemNode *GP32FilesystemNode::child(const String &name) const {
	// FIXME: Pretty lame implementation! We do no error checking to speak
	// of, do not check if this is a special node, etc.
	assert(_isDirectory);
	String newPath(_path);
	if (_path.lastChar() != '\\')
		newPath += '\\';
	newPath += name;
	GP32FilesystemNode *p = new GP32FilesystemNode(newPath);

	return p;
}
