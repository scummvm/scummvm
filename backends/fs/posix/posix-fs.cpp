/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
 * $Header$
 */

#if defined(UNIX) || defined(__DC__)

#include "common/stdafx.h"

#include "../fs.h"

#ifdef MACOSX
#include <sys/types.h>
#endif
#include <sys/param.h>
#include <sys/stat.h>
#ifndef __DC__
#include <dirent.h>
#endif
#include <stdio.h>
#include <unistd.h>

/*
 * Implementation of the ScummVM file system API based on POSIX.
 */

class POSIXFilesystemNode : public AbstractFilesystemNode {
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

	virtual FSList listDir(ListMode mode = kListDirectoriesOnly) const;
	virtual AbstractFilesystemNode *parent() const;
};


static const char *lastPathComponent(const Common::String &str) {
	const char *start = str.c_str();
	const char *cur = start + str.size() - 2;

	while (cur > start && *cur != '/') {
		--cur;
	}

	return cur+1;
}

AbstractFilesystemNode *FilesystemNode::getRoot() {
	return new POSIXFilesystemNode();
}

AbstractFilesystemNode *FilesystemNode::getNodeForPath(const String &path) {
	return new POSIXFilesystemNode(path);
}

POSIXFilesystemNode::POSIXFilesystemNode() {
#ifndef __DC__
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

POSIXFilesystemNode::POSIXFilesystemNode(const String &p) {
	int len = 0, offset = p.size();
	struct stat st;

	assert(offset > 0);

	_path = p;

	// Extract last component from path
	const char *str = p.c_str();
	while (offset > 0 && str[offset-1] == '/')
		offset--;
	while (offset > 0 && str[offset-1] != '/') {
		len++;
		offset--;
	}
	_displayName = String(str + offset, len);

	// Check whether it is a directory, and whether the file actually exists
#ifdef __DC__
	_isValid = true;
	_isDirectory = true;
#else
	_isValid = (0 == stat(_path.c_str(), &st));
	_isDirectory = S_ISDIR(st.st_mode);
#endif
}

POSIXFilesystemNode::POSIXFilesystemNode(const POSIXFilesystemNode *node) {
	_displayName = node->_displayName;
	_isValid = node->_isValid;
	_isDirectory = node->_isDirectory;
	_path = node->_path;
}

FSList POSIXFilesystemNode::listDir(ListMode mode) const {
	assert(_isDirectory);
	DIR *dirp = opendir(_path.c_str());
	struct stat st;

	struct dirent *dp;
	FSList myList;

	if (dirp == NULL)
		return myList;

	// ... loop over dir entries using readdir
	while ((dp = readdir(dirp)) != NULL) {
		// Skip 'invisible' files
		if (dp->d_name[0] == '.')
			continue;

		POSIXFilesystemNode entry;
		entry._displayName = dp->d_name;
		entry._path = _path;
		if (entry._path.lastChar() != '/')
			entry._path += '/';
		entry._path += dp->d_name;

#ifdef __DC__
		entry._isDirectory = dp->d_size < 0;
#else
		if (stat(entry._path.c_str(), &st))
			continue;
		entry._isDirectory = S_ISDIR(st.st_mode);
#endif

		// Honor the chosen mode
		if ((mode == kListFilesOnly && entry._isDirectory) ||
			(mode == kListDirectoriesOnly && !entry._isDirectory))
			continue;

		if (entry._isDirectory)
			entry._path += "/";
		myList.push_back(wrap(new POSIXFilesystemNode(&entry)));
	}
	closedir(dirp);
	return myList;
}

AbstractFilesystemNode *POSIXFilesystemNode::parent() const {
	if (_path == "/")
		return 0;

	POSIXFilesystemNode *p = new POSIXFilesystemNode();
	const char *start = _path.c_str();
	const char *end = lastPathComponent(_path);

	p->_path = String(start, end - start);
	p->_displayName = lastPathComponent(p->_path);

	p->_isValid = true;
	p->_isDirectory = true;

	return p;
}

#endif // defined(UNIX)
