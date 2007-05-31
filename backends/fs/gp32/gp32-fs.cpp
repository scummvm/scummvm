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

#define MAX_PATH_SIZE 256

/**
 * Implementation of the ScummVM file system API.
 * 
 * Parts of this class are documented in the base interface class, AbstractFilesystemNode.
 */
class GP32FilesystemNode : public AbstractFilesystemNode {
protected:
	String _displayName;
	String _path;
	bool _isDirectory;
	bool _isRoot;

public:
	/**
	 * Creates a GP32FilesystemNode with the root node as path.
	 */
	GP32FilesystemNode();
	
	/**
	 * Creates a GP32FilesystemNode for a given path.
	 * 
	 * @param path String with the path the new node should point to.
	 */
	GP32FilesystemNode(const String &path);

	virtual String getDisplayName() const { return _displayName; }
	virtual String getName() const { return _displayName; }
	virtual String getPath() const { return _path; }
	virtual bool isDirectory() const { return _isDirectory; }
	// FIXME: isValid should return false if this Node can't be used!
	// so client code can rely on the return value.
	virtual bool isValid() const { return true; }

	virtual AbstractFilesystemNode *getChild(const String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode) const;
	virtual AbstractFilesystemNode *getParent() const;
};

const char gpRootPath[] = "gp:\\";
//char gpCurrentPath[MAX_PATH_SIZE] = "gp:\\";		// must end with '\'

/**
 * Returns the last component of a given path.
 * 
 * Examples:
 * 			gp:\foo\bar.txt would return "\bar.txt"
 * 			gp:\foo\bar\    would return "\bar\"
 *  
 * @param str Path to obtain the last component from.
 * @return Pointer to the first char of the last component inside str.
 */
static const char *lastPathComponent(const Common::String &str) {
	const char *start = str.c_str();
	const char *cur = start + str.size() - 2;

	while (cur >= start && *cur != '\\') {
		--cur;
	}

	return cur + 1;
}

/**
 * FIXME: document this function.
 * 
 * @param path
 * @param convPath
 */
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
		_isRoot = false;
		_displayName = String(dsplName);
	}
	_isDirectory = true;
}

AbstractFilesystemNode *GP32FilesystemNode::getChild(const String &n) const {
	// FIXME: Pretty lame implementation! We do no error checking to speak
	// of, do not check if this is a special node, etc.
	assert(_isDirectory);
	
	String newPath(_path);
	if (_path.lastChar() != '\\')
		newPath += '\\';
	newPath += n;

	return new GP32FilesystemNode(newPath);
}

bool GP32FilesystemNode::getChildren(AbstractFSList &myList, ListMode mode) const {
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
		entry._isRoot = false;

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

AbstractFilesystemNode *GP32FilesystemNode::getParent() const {
	if(_isRoot)
		return 0;

	const char *start = _path.c_str();
	const char *end = lastPathComponent(_path);

	GP32FilesystemNode *p = new GP32FilesystemNode(String(start, end - start));

	NP("%s", p->_path.c_str());

	return p;
}
