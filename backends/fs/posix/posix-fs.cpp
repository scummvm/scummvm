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

#if defined(UNIX)

#include "../fs.h"

#ifdef MACOSX
#include <sys/types.h>
#endif
#include <dirent.h>

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

	virtual FSList *listDir() const;
	virtual FilesystemNode *parent() const;
	virtual FilesystemNode *clone() const { return new POSIXFilesystemNode(this); }
};


FilesystemNode *FilesystemNode::getRoot() {
	return new POSIXFilesystemNode();
}

POSIXFilesystemNode::POSIXFilesystemNode() {
	_displayName = "/";
	_isValid = true;
	_isDirectory = true;
	_path = "/";
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

FSList *POSIXFilesystemNode::listDir() const {
	assert(_isDirectory);
	DIR *dirp = opendir(_path.c_str());
	assert(dirp != 0);

	struct dirent *dp;
	FSList *myList = new FSList();
	
	// ... loop over dir entries using readdir
	while ((dp = readdir(dirp)) != NULL) {
		POSIXFilesystemNode entry;
		entry._displayName = dp->d_name;
		entry._isDirectory = (dp->d_type == DT_DIR);	// TODO - add support for symlinks to dirs?

		// FIXME - skip any non-directories for now
		if (!entry._isDirectory) continue;

		entry._path = _path;
		entry._path += dp->d_name;
		if (entry._isDirectory)
			entry._path += "/";
		myList->push_back(entry);
	}
	closedir(dirp);
	return myList;
}

FilesystemNode *POSIXFilesystemNode::parent() const {
	// TODO !!!
	return 0;
}

#endif // defined(UNIX)
