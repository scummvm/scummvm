/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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

#if defined(UNIX) || defined (__GP32__) //ph0x

#include "../fs.h"

#ifdef MACOSX
#include <sys/types.h>
#endif
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>

#ifdef __GP32__ //ph0x FIXME: implement and move to portdefs.h
#define opendir(x) (0)
#define readdir(x) (0)
#define closedir(x) (0)
#endif

/*
 * Implementation of the ScummVM file system API based on POSIX.
 */

class POSIXFilesystemNode : public FilesystemNode {
protected:
	String _displayName;
	bool _isDirectory;
	bool _isValid;
	String _path;
	
public:
	POSIXFilesystemNode();
	POSIXFilesystemNode(const String &path);
	POSIXFilesystemNode(const POSIXFilesystemNode *node);

	virtual String displayName() const { return _displayName; }
	virtual bool isValid() const { return _isValid; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual String path() const { return _path; }

	virtual FSList *listDir(ListMode mode = kListDirectoriesOnly) const;
	virtual FilesystemNode *parent() const;
	virtual FilesystemNode *clone() const { return new POSIXFilesystemNode(this); }
};


static const char *lastPathComponent(const Common::String &str) {
	const char *start = str.c_str();
	const char *cur = start + str.size() - 2;
	
	while (cur > start && *cur != '/') {
		--cur;
	}
	
	return cur+1;
}

FilesystemNode *FilesystemNode::getRoot() {
	return new POSIXFilesystemNode();
}

POSIXFilesystemNode::POSIXFilesystemNode() {
#if 1
	char buf[MAXPATHLEN];
	getcwd(buf, MAXPATHLEN);
	
	_path = buf;
	_displayName = lastPathComponent(_path);
	_path += '/';
#else
	_path = "/";
	_displayName = _path;
#endif
	_isValid = true;
	_isDirectory = true;
}

/*
POSIXFilesystemNode::POSIXFilesystemNode(const String &p) {
	// TODO - extract last component from path
	_displayName = p;
	// TODO - check whether it is a directory, and whether the file actually exists
	_isValid = true;
	_isDirectory = true;
	_path = p;
}
*/

POSIXFilesystemNode::POSIXFilesystemNode(const POSIXFilesystemNode *node) {
	_displayName = node->_displayName;
	_isValid = node->_isValid;
	_isDirectory = node->_isDirectory;
	_path = node->_path;
}

FSList *POSIXFilesystemNode::listDir(ListMode mode) const {
	assert(_isDirectory);
	DIR *dirp = opendir(_path.c_str());
	struct stat st;

	struct dirent *dp;
	FSList *myList = new FSList();

	if (dirp == NULL) return myList;
	
	// ... loop over dir entries using readdir
	while ((dp = readdir(dirp)) != NULL) {
		// Skip 'invisible' files
		if (dp->d_name[0] == '.')
			continue;
		
		POSIXFilesystemNode entry;
		entry._displayName = dp->d_name;
		entry._path = _path;
		entry._path += dp->d_name;

		if (stat(entry._path.c_str(), &st))
			continue;
		entry._isDirectory = S_ISDIR(st.st_mode);

		// Honor the chosen mode
		if ((mode == kListFilesOnly && entry._isDirectory) ||
			(mode == kListDirectoriesOnly && !entry._isDirectory))
			continue;

		if (entry._isDirectory)
			entry._path += "/";
		myList->push_back(entry);
	}
	closedir(dirp);
	return myList;
}

FilesystemNode *POSIXFilesystemNode::parent() const {
	POSIXFilesystemNode *p = new POSIXFilesystemNode();

	// Root node is its own parent. Still we can't just return this
	// as the GUI code will call delete on the old node.
	if (_path != "/") {
		const char *start = _path.c_str();
		const char *end = lastPathComponent(_path);

		p->_path = String(start, end - start);
		p->_displayName = lastPathComponent(p->_path);
	} else {
		p->_path = _path;
		p->_displayName = _displayName;
	}
	p->_isValid = true;
	p->_isDirectory = true;
	return p;
}

#endif // defined(UNIX)
