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

#if defined(__DC__)

#include "common/stdafx.h"

#include "backends/fs/abstract-fs.h"
#include "backends/fs/fs.h"

#include <ronin/cdfs.h>
#include <stdio.h>
#include <unistd.h>

/*
 * Implementation of the ScummVM file system API based on ronin.
 */

class RoninCDFilesystemNode : public AbstractFilesystemNode {
protected:
	String _displayName;
	bool _isDirectory;
	bool _isValid;
	String _path;

public:
	RoninCDFilesystemNode();
	RoninCDFilesystemNode(const String &path, bool verify);

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
	// Since there is no way to _set_ the current directory,
	// it will always be /...

	return getRoot();
}

AbstractFilesystemNode *AbstractFilesystemNode::getRoot() {
	return new RoninCDFilesystemNode();
}

AbstractFilesystemNode *AbstractFilesystemNode::getNodeForPath(const String &path) {
	return new RoninCDFilesystemNode(path, true);
}

RoninCDFilesystemNode::RoninCDFilesystemNode() {
	// The root dir.
	_path = "/";
	_displayName = _path;
	_isValid = true;
	_isDirectory = true;
}

RoninCDFilesystemNode::RoninCDFilesystemNode(const String &p, bool verify) {
	assert(p.size() > 0);

	_path = p;
	_displayName = lastPathComponent(_path);
	_isValid = true;
	_isDirectory = true;

	if (verify) {
		int fd;

		if((fd = open(_path.c_str(), O_RDONLY)) >= 0) {
			close(fd);
			_isDirectory = false;
		}
		else if((fd = open(_path.c_str(), O_DIR|O_RDONLY)) >= 0) {
			close(fd);
		}
		else {
			_isValid = false;
		}
	}
}

bool RoninCDFilesystemNode::listDir(AbstractFSList &myList, ListMode mode) const {
	assert(_isDirectory);
	DIR *dirp = opendir(_path.c_str());

	struct dirent *dp;

	if (dirp == NULL)
		return false;

	// ... loop over dir entries using readdir
	while ((dp = readdir(dirp)) != NULL) {
		String newPath(_path);
		if (newPath.lastChar() != '/')
			newPath += '/';
		newPath += dp->d_name;

		RoninCDFilesystemNode entry(newPath, false);

		entry._isDirectory = dp->d_size < 0;

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
		myList.push_back(new RoninCDFilesystemNode(entry));
	}
	closedir(dirp);
	return true;
}

AbstractFilesystemNode *RoninCDFilesystemNode::parent() const {
	if (_path == "/")
		return 0;

	const char *start = _path.c_str();
	const char *end = lastPathComponent(_path);

	RoninCDFilesystemNode *p = new RoninCDFilesystemNode(String(start, end - start), false);

	return p;
}

AbstractFilesystemNode *RoninCDFilesystemNode::child(const String &name) const {
	// FIXME: Pretty lame implementation! We do no error checking to speak
	// of, do not check if this is a special node, etc.
	assert(_isDirectory);
	String newPath(_path);
	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += name;
	RoninCDFilesystemNode *p = new RoninCDFilesystemNode(newPath, true);

	return p;
}

#endif // defined(__DC__)
