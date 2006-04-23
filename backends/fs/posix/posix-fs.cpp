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

#if defined(UNIX) || defined(__DC__)

#include "common/stdafx.h"

#include "backends/fs/fs.h"

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
	POSIXFilesystemNode(const String &path, bool useStat = false);

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

	return cur + 1;
}

AbstractFilesystemNode *FilesystemNode::getRoot() {
	return new POSIXFilesystemNode();
}

AbstractFilesystemNode *FilesystemNode::getNodeForPath(const String &path) {
	return new POSIXFilesystemNode(path, true);
}

POSIXFilesystemNode::POSIXFilesystemNode() {
#ifndef __DC__
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
#else
	_path = "/";
	_displayName = _path;
#endif
	_isValid = true;
	_isDirectory = true;
}

POSIXFilesystemNode::POSIXFilesystemNode(const String &p, bool useStat) {
	assert(p.size() > 0);

	_path = p;
	_displayName = lastPathComponent(_path);
	_isValid = true;
	_isDirectory = true;

#ifndef __DC__
	if (useStat) {
		struct stat st;
		_isValid = (0 == stat(_path.c_str(), &st));
		_isDirectory = S_ISDIR(st.st_mode);
	}
#endif
}

FSList POSIXFilesystemNode::listDir(ListMode mode) const {
	assert(_isDirectory);
	DIR *dirp = opendir(_path.c_str());

	struct dirent *dp;
	FSList myList;

	if (dirp == NULL)
		return myList;

	// ... loop over dir entries using readdir
	while ((dp = readdir(dirp)) != NULL) {
		// Skip 'invisible' files
		if (dp->d_name[0] == '.')
			continue;

		String newPath(_path);
		if (newPath.lastChar() != '/')
			newPath += '/';
		newPath += dp->d_name;

		POSIXFilesystemNode entry(newPath);

#ifdef __DC__
		entry._isDirectory = dp->d_size < 0;
#elif defined(SYSTEM_NOT_SUPPORTING_D_TYPE)
		// TODO: d_type is not part of POSIX, so it might not be supported
		// on some of our targets. For those systems where it isn't supported,
		// add this #elif case, which tries to use stat() instead.
		struct stat st;
		entry._isValid = (0 == stat(entry._path.c_str(), &st));
		entry._isDirectory = S_ISDIR(st.st_mode);
#else
		if (dp->d_type == DT_UNKNOWN) {
			// Fall back to stat()
			struct stat st;
			entry._isValid = (0 == stat(entry._path.c_str(), &st));
			entry._isDirectory = S_ISDIR(st.st_mode);
		} else {
			entry._isValid = (dp->d_type == DT_DIR) || (dp->d_type == DT_REG) || (dp->d_type == DT_LNK);
			if (dp->d_type == DT_LNK) {
				struct stat st;
				stat(entry._path.c_str(), &st);
				entry._isDirectory = S_ISDIR(st.st_mode);
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
		if ((mode == kListFilesOnly && entry._isDirectory) ||
			(mode == kListDirectoriesOnly && !entry._isDirectory))
			continue;

		if (entry._isDirectory)
			entry._path += "/";
		myList.push_back(wrap(new POSIXFilesystemNode(entry)));
	}
	closedir(dirp);
	return myList;
}

AbstractFilesystemNode *POSIXFilesystemNode::parent() const {
	if (_path == "/")
		return 0;

	const char *start = _path.c_str();
	const char *end = lastPathComponent(_path);

	POSIXFilesystemNode *p = new POSIXFilesystemNode(String(start, end - start));

	return p;
}

#endif // defined(UNIX)
