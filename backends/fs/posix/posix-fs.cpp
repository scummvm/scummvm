/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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

#if defined(UNIX)

#include "common/stdafx.h"

#include "backends/fs/abstract-fs.h"
#include "backends/fs/fs.h"

#ifdef MACOSX
#include <sys/types.h>
#endif
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
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
	POSIXFilesystemNode(const String &path, bool verify);

	virtual String displayName() const { return _displayName; }
	virtual bool isValid() const { return _isValid; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual String path() const { return _path; }

	virtual bool listDir(AbstractFSList &list, ListMode mode) const;
	virtual AbstractFilesystemNode *parent() const;
	virtual AbstractFilesystemNode *child(const String &name) const;
};


static const char *lastPathComponent(const Common::String &str) {
	const char *start = str.c_str();
	const char *cur = start + str.size() - 2;

	while (cur > start && *cur != '/') {
		--cur;
	}

	return cur + 1;
}

AbstractFilesystemNode *AbstractFilesystemNode::getCurrentDirectory() {
	char buf[MAXPATHLEN];
	getcwd(buf, MAXPATHLEN);
	return new POSIXFilesystemNode(buf, true);
}

AbstractFilesystemNode *AbstractFilesystemNode::getRoot() {
	return new POSIXFilesystemNode();
}

AbstractFilesystemNode *AbstractFilesystemNode::getNodeForPath(const String &path) {
	return new POSIXFilesystemNode(path, true);
}

POSIXFilesystemNode::POSIXFilesystemNode() {
/*  The Browser code now saves the last browsed directory into the config file.
    Hence the need to start at the "current" directory is far less, and we can
    remove this hack for now. Still, there may be some need to obtain a ref
    to the "current" directory. See the TODO list for some thoughts on this.
    
    I am leaving this code in here for the time being, to be used as reference.

	// FIXME: It is evil & slow to always call getcwd here.
	// The intention behind this hack was/is to be more user friendly
	// in our save/load dialogs: Instead of starting at the FS root,
	// we start at the current directory. However, that's just a hack.
	// Proper solution would be to extend FilesystemNode by the concept
	// of 'current' or 'default' directory, and then modify the
	// save/load dialogs to explicitly use that as starting point.
	char buf[MAXPATHLEN];
	getcwd(buf, MAXPATHLEN);

	_path = buf;
	_displayName = lastPathComponent(_path);
	_path += '/';
*/
	// The root dir.
	_path = "/";
	_displayName = _path;
	_isValid = true;
	_isDirectory = true;
}

POSIXFilesystemNode::POSIXFilesystemNode(const String &p, bool verify) {
	assert(p.size() > 0);

	_path = p;
	_displayName = lastPathComponent(_path);
	_isValid = true;
	_isDirectory = true;

	if (verify) {
		struct stat st;
		_isValid = (0 == stat(_path.c_str(), &st));
		_isDirectory = _isValid ? S_ISDIR(st.st_mode) : false;
	}
}

bool POSIXFilesystemNode::listDir(AbstractFSList &myList, ListMode mode) const {
	assert(_isDirectory);
	DIR *dirp = opendir(_path.c_str());

	struct dirent *dp;

	if (dirp == NULL)
		return false;

	// ... loop over dir entries using readdir
	while ((dp = readdir(dirp)) != NULL) {
		// Skip 'invisible' files
		if (dp->d_name[0] == '.')
			continue;

		String newPath(_path);
		if (newPath.lastChar() != '/')
			newPath += '/';
		newPath += dp->d_name;

		POSIXFilesystemNode entry(newPath, false);

#if defined(SYSTEM_NOT_SUPPORTING_D_TYPE)
		// TODO: d_type is not part of POSIX, so it might not be supported
		// on some of our targets. For those systems where it isn't supported,
		// add this #elif case, which tries to use stat() instead.
		struct stat st;
		entry._isValid = (0 == stat(entry._path.c_str(), &st));
		entry._isDirectory = entry._isValid ? S_ISDIR(st.st_mode) : false;
#else
		if (dp->d_type == DT_UNKNOWN) {
			// Fall back to stat()
			struct stat st;
			entry._isValid = (0 == stat(entry._path.c_str(), &st));
			entry._isDirectory = entry._isValid ? S_ISDIR(st.st_mode) : false;
		} else {
			entry._isValid = (dp->d_type == DT_DIR) || (dp->d_type == DT_REG) || (dp->d_type == DT_LNK);
			if (dp->d_type == DT_LNK) {
				struct stat st;
				if (stat(entry._path.c_str(), &st) == 0)
					entry._isDirectory = S_ISDIR(st.st_mode);
				else
					entry._isDirectory = false;
			} else {
				entry._isDirectory = (dp->d_type == DT_DIR);
			}
		}
#endif

		// Skip files that are invalid for some reason (e.g. because we couldn't
		// properly stat them).
		if (!entry._isValid)
			continue;

		// Honor the chosen mode
		if ((mode == FilesystemNode::kListFilesOnly && entry._isDirectory) ||
			(mode == FilesystemNode::kListDirectoriesOnly && !entry._isDirectory))
			continue;

		if (entry._isDirectory)
			entry._path += "/";
		myList.push_back(new POSIXFilesystemNode(entry));
	}
	closedir(dirp);
	return true;
}

AbstractFilesystemNode *POSIXFilesystemNode::parent() const {
	if (_path == "/")
		return 0;

	const char *start = _path.c_str();
	const char *end = lastPathComponent(_path);

	POSIXFilesystemNode *p = new POSIXFilesystemNode(String(start, end - start), false);

	return p;
}

AbstractFilesystemNode *POSIXFilesystemNode::child(const String &name) const {
	// FIXME: Pretty lame implementation! We do no error checking to speak
	// of, do not check if this is a special node, etc.
	assert(_isDirectory);
	String newPath(_path);
	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += name;
	POSIXFilesystemNode *p = new POSIXFilesystemNode(newPath, true);

	return p;
}

#endif // defined(UNIX)
